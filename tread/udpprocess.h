#ifndef UDPPROCESS_H
#define UDPPROCESS_H

#include <QObject>
#include <QUdpSocket>
#include <QThread>
#include "appdata.h"

#define UDP_RECEIVE_SIZE 64 * 1024 *1024    // UDP 接收区大小
#define UDP_BUFFER_SIZE 1024 * 4            // UDP 缓冲区大小

class UdpProcess : public QObject
{
    Q_OBJECT
public:
    UdpProcess(My_HostAddress self);

public slots:
    void stopSend();                                // 暂停通讯
    void startSend();                               // 开启通讯
    void sendCommand(QByteArray, My_HostAddress, My_HostAddress); // UDP 发送下位机指令

private:
    QUdpSocket *m_udpReceive;                       // 网口通信对象

    QByteArray udpReceiveBuffer[UDP_BUFFER_SIZE];   // UDP 接收数据缓存区
    QVector<quint8> m_data;                         // UDP 数据解析区
    QVector<double> m_accX;                         // 一包有效数据

    My_HostAddress m_self;
    My_HostAddress m_target;
    quint8 dataLength;                              // 一组有效数据长度
    quint16 packetNums;                             // 一包 Packet 中的有效数据组数
    quint16 packetLength;                           // 一包 Packet 的长度
    quint8 head1;                                   // 帧头
    quint8 head2;

private slots:
    void udpDataReceive();                          // UDP 网口接收信号的槽函数
    void udpDataProcess(QByteArray buf);            // UDP 缓冲区数据处理

signals:
    void udpData(QByteArray buf);                   // UDP 数据解码（校验、提取等）
    void udpSendMessageBox(QString type, QString context);// 报警框信息
    void sendAccX(QVector<double> accx);            // 发送解码后的有效数据
    void UDPConnectFailed();                        // UDP 连接失败的信号
};

#endif // UDPPROCESS_H
