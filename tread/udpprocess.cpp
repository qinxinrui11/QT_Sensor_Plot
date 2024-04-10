#include "udpprocess.h"
#include "appdata.h"

#if _MSC_VER >= 1600    // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif

/**
 * @brief 配置 UDP 数据接收
 *
 * 此函数用于 UDP 数据接收的初始化
 * 只配置了本机 IP 和端口号，而没有配置目标 IP 和端口号
 *
 * @param 无
 * @return 无
 */
UdpProcess::UdpProcess(My_HostAddress self)
{
    m_self = self;

    // 创建一个网口通信对象，实现数据接收解码和指令发送
    m_udpReceive = new QUdpSocket;
    m_udpReceive->setReadBufferSize(UDP_RECEIVE_SIZE);    // 设置接收缓冲区大小

    connect(m_udpReceive, &QUdpSocket::readyRead, this, &UdpProcess::udpDataReceive); // UDP 套接字数据接收至缓冲区

    // 设置数据解析参数
    head1 = 0xAA;       // 帧头
    head2 = 0x55;
    dataLength = 12;    // 一组有效数据包括三个轴的加速度值，每个值长度为4字节
    packetNums = 100;   // 一包 Packet 中包括 100 组有效数据
    packetLength = dataLength * packetNums + 4; // 一包 Packet 的总长度
    connect(this, &UdpProcess::udpData, this, &UdpProcess::udpDataProcess); // UDP 缓冲区数据处理
}

void UdpProcess::stopSend()
{
    m_udpReceive->close();  // 关闭网口通讯
    m_registryData.UDPConnect_flag = false;
}

void UdpProcess::startSend()
{
    //绑定端口号，接收数据，如果返回 0 说明绑定失败
    int receive = m_udpReceive->bind(QHostAddress(m_self.IP), m_self.port);
    if(receive != 0)
    {
        qDebug() << "UDP Connected Succeed ! ";
        m_registryData.UDPConnect_flag = true;
    }else
    {
        qDebug() << "UDP Connected Faild ! " << m_udpReceive->errorString();
        emit udpSendMessageBox("警告", "UDP绑定失败！");
        emit UDPConnectFailed();
    }
}

/**
 * @brief 向下位机发送命令
 *
 * 此函数接收 QbyteArray 型命令，以及通讯时的 ip 和端口，实现向下位机配置参数
 * 连续发送两次指令，避免未收到
 *
 * @param 无
 * @return 无
 */
void UdpProcess::sendCommand(QByteArray command, My_HostAddress self, My_HostAddress target)
{
    m_self = self;
    m_target = target;
    m_udpReceive->writeDatagram(command, QHostAddress(target.IP), target.port);
    QThread::msleep(50);
    m_udpReceive->writeDatagram(command, QHostAddress(target.IP), target.port);
    // 自动重连
    stopSend();
    startSend();
}

/**
 * @brief UDP 缓冲区数据接收
 *
 * 此函数将 UDP 套接字接收到的数据转存至缓冲区，并发送信号进行数据解码
 *
 * @param 无
 * @return 无
 */
void UdpProcess::udpDataReceive()
{
    m_registryData.udpPacket++;
    do{
        udpReceiveBuffer->resize(m_udpReceive->pendingDatagramSize());   // 将 UDPReceiveBuffer 的大小调成待读取数据的大小
        m_udpReceive->readDatagram(udpReceiveBuffer->data(), udpReceiveBuffer->size());    // 将 m_receive 中的数据读取到缓冲区 UDPReceiveBuffer 中
        emit udpData(udpReceiveBuffer[0]);
    }while(m_udpReceive->hasPendingDatagrams());   // 当 UDP 套接字中还有未被 readDatagram 读取到的数据
}

/**
 * @brief 定位帧头
 *
 * 内部函数，专用于定位帧头，支持两位帧头
 *
 * @param QVector<quint8> *data UPD 数据指针
 *          quint8 head1 第一个帧头
 *          quint8 head2 第二个帧头
 * @return 无
 */
void findHeadIndex(QVector<quint8> *data, quint8 head1, quint8 head2)
{
    // 查询第一个帧头的位置
    qint16 headIndex = data->indexOf(head1);
    if(headIndex == -1){    // 如果查询不到，直接返回
        qDebug() << "帧头错误，未定义到帧头1";
        data->clear();
        return;
    }

    // 继续查询第二个帧头的位置
    quint16 nextIndex = headIndex + 1;
    if(nextIndex < data->size() && data->at(nextIndex) == head2)    // 查询成功
        return;
    else{   // 查询失败，则移除第一个帧头，并递归查找下一个帧头
        qDebug() << "帧头错误，未定义到帧头2";
        data->remove(0, headIndex + 1);
        findHeadIndex(data, head1, head2);
    }
}

/**
 * @brief UDP 数据处理
 *
 * 此函数将 UDP 缓冲区发来的数据进行校验，有效数据提取
 *
 * @param QByteArray *buf 缓冲区指针
 * @return 无
 */
void UdpProcess::udpDataProcess(QByteArray buf)
{
    // 将数据从缓冲区转存到私有变量 data 中
    for(int i = 0; i < buf.size(); i++)
        m_data.push_back(buf[i]);

    // 定位到帧头 0xAA,0x55
    findHeadIndex(&m_data, head1, head2);

    // 开始解析数据
    while(m_data.size() >= packetLength)
    {
        // 进行求和校验
        quint8 sum = 0;
        for(int i = 0; i < packetLength - 1; i++)
            sum += m_data[i];
        if(sum != m_data[packetLength - 1]){
            qDebug() << "此帧未通过校验，重新寻找下一个帧头";
            m_data.pop_front();
            findHeadIndex(&m_data, head1, head2);
            continue;
        }

        // 跳过前三个字节：帧头和设备号，开始存数据帧中的有效数据部分
        m_accX.clear();
        qint32 acc_x, acc_y, acc_z;
        for(int num = 3; num < packetNums - 1; num += dataLength){
            acc_x = (m_data[num + 3] << 24) | (m_data[num + 2] << 16) | (m_data[num + 1] << 8) | (m_data[num + 0]);
            acc_y = (m_data[num + 7] << 24) | (m_data[num + 6] << 16) | (m_data[num + 5] << 8) | (m_data[num + 4]);
            acc_z = (m_data[num + 11] << 24) | (m_data[num + 10] << 16) | (m_data[num + 9] << 8) | (m_data[num + 8]);

            m_registryData.accX = acc_x / 100000.0;
            m_registryData.accY = acc_y / 100000.0;
            m_registryData.accZ = acc_z / 100000.0;

            m_accX.append(m_registryData.accX);
        }
        // 发送一包有效数据，去画图
        emit sendAccX(m_accX);

        // 从私有变量中弹出处理完的数据
        for(int i = 0; i < packetLength; i++)
            m_data.pop_front();

    }
}

