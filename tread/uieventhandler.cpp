#include "uieventhandler.h"
#include "appdata.h"
#include "udpprocess.h"

// 中文字符的 UTF-8 显示，解决乱码问题
#if _MSC_VER >= 1600    // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif

extern RegistryData m_registryData;

UIEventHandler::UIEventHandler(Ui::MainWindow *ui, quint8 refreshRate, MainWindow *mainWindow)
{
    m_ui = ui;
    m_mainWindow = mainWindow;

    ModifyParameter();
    /****************************************/
    // 初始化 UI 刷新的定时器，每隔一段时间触发绘图操作
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &UIEventHandler::updateUI);
    m_timer->start(1000 / refreshRate); // 设置定时器间隔(ms)
    // 初始化数据保存的定时器，在数据保存过程中每秒计时
    save_timer = new QTimer(this);
    connect(save_timer, &QTimer::timeout, this, [&](){save_seconds++;});
    save_seconds = 0;
}

/**
 * @brief 配置滚动条
 *
 * 此函数用于程序内部参数的在线改值功能的初始化
 * 将所有的 Label, Scroll, LineEdit 注册到 List 中，并关联更改后调用的槽函数
 *
 * @param 无
 * @return 无
 */
void UIEventHandler::ModifyParameter()
{
    // 所有 scroll 滚动条的信号连接
    connect(m_ui->num_scroll, &QScrollBar::valueChanged, this, &UIEventHandler::ScrollBarValueChanged);
    connect(m_ui->frequency_scroll, &QScrollBar::valueChanged, this, &UIEventHandler::ScrollBarValueChanged);
    connect(m_ui->count_scroll, &QScrollBar::valueChanged, this, &UIEventHandler::ScrollBarValueChanged);
    connect(m_ui->fftCount_scroll, &QScrollBar::valueChanged, this, &UIEventHandler::ScrollBarValueChanged);

    // 所有 lineEdit 的初始化
    m_ui->selfIP_lineEdit->setText(m_registryData.self.IP);
    m_ui->selfPort_lineEdit->setText(QString::number(m_registryData.self.port));
    m_ui->targetIP_lineEdit->setText(m_registryData.target.IP);
    m_ui->targetPort_lineEdit->setText(QString::number(m_registryData.target.port));
    connect(m_ui->selfIP_lineEdit, &QLineEdit::editingFinished, this, &UIEventHandler::LineEditValueChanged);
    connect(m_ui->selfPort_lineEdit, &QLineEdit::editingFinished, this, &UIEventHandler::LineEditValueChanged);
    connect(m_ui->targetIP_lineEdit, &QLineEdit::editingFinished, this, &UIEventHandler::LineEditValueChanged);
    connect(m_ui->targetPort_lineEdit, &QLineEdit::editingFinished, this, &UIEventHandler::LineEditValueChanged);

    // 所有 ComboBox 的初始化
    m_ui->workFreq_comboBox->setCurrentIndex(m_registryData.workFreqIndex); // 设置默认选择第一个选项
    m_ui->deadFreq_comboBox->setCurrentIndex(m_registryData.deadFreqIndex); // 设置默认选择第一个选项
    m_ui->range_comboBox->setCurrentIndex(m_registryData.accRangeIndex); // 设置默认选择第一个选项
    connect(m_ui->workFreq_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &UIEventHandler::ComboBoxValueChanged);
    connect(m_ui->deadFreq_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &UIEventHandler::ComboBoxValueChanged);
    connect(m_ui->range_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &UIEventHandler::ComboBoxValueChanged);

    // 所有 Slider 的初始化
    connect(m_ui->time_verticalSlider, &QSlider::valueChanged, this, &UIEventHandler::SliderValueChanged);
    connect(m_ui->freq_verticalSlider, &QSlider::valueChanged, this, &UIEventHandler::SliderValueChanged);

    // 所有 PushButton 的初始化
    connect(m_ui->modifyIP_pushButton, &QPushButton::clicked, this, &UIEventHandler::PushButtonValueChanged);
    connect(m_ui->startUDP_pushButton, &QPushButton::clicked, this, &UIEventHandler::PushButtonValueChanged);
    connect(m_ui->dataSave_pushButton, &QPushButton::clicked, this, &UIEventHandler::PushButtonValueChanged);
    connect(m_ui->plotClear_pushButton, &QPushButton::clicked, this, &UIEventHandler::PushButtonValueChanged);

    // lcdNumber 的初始化
    m_ui->dataSave_lcdNumber->display("00:00:00");
}

/**
 * @brief
 *
 * 该槽函数用于数据的在线改值响应
 * 读取对应更改的值，更改到全局变量 m_registryData 中
 *
 * @param int value
 * @return 无
 */
void UIEventHandler::ScrollBarValueChanged(int value)
{
    QScrollBar *scrollBar = qobject_cast<QScrollBar*>(sender());
    if(!scrollBar){
        return;
    }

    if(scrollBar == m_ui->num_scroll){
//        qDebug() << "ScrollBar: change num:" << value;
        m_registryData.T = value;
    }
    else if(scrollBar == m_ui->frequency_scroll){
//        qDebug() << "ScrollBar: change frequency:" << value;
        m_registryData.FREQUENCY_PER_SECOND = value;
        m_registryData.FFT_F = m_registryData.FREQUENCY_PER_SECOND * m_registryData.DATA_COUNT; // FFT 输入数据的频率
        m_mainWindow->m_dataGenerator->start();
    }
    else if(scrollBar == m_ui->count_scroll){
//        qDebug() << "ScrollBar: change count:" << value;
        m_registryData.DATA_COUNT = value;
        m_registryData.FFT_F = m_registryData.FREQUENCY_PER_SECOND * m_registryData.DATA_COUNT; // FFT 输入数据的频率
    }
    else if(scrollBar == m_ui->fftCount_scroll){
//            qDebug() << "ScrollBar: change fftCount:" << value;
            m_registryData.FFT_N = value;
    }
}

// 检验IP地址的正确性
bool IPIsRight(QStringList parts, QString ip)
{
    parts = ip.split('.');
    if(parts.size() != 4)
        return false;
    else{
        for(const QString& part : parts){
            bool ok;
            int num = part.toInt(&ok);
            if(!ok || num < 0 || num > 255){
                return false;
            }
        }
        return true;
    }
}

/**
 * @brief
 *
 * 该槽函数用于 lineEdit 的在线改值响应
 * 读取对应更改的值，并存入注册表中
 *
 * @param 无
 * @return 无
 */
void UIEventHandler::LineEditValueChanged()
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(sender());
    if(!lineEdit){
        return;
    }

    QString ip, port;
    bool success = true;
    int portInt;
    QStringList parts;

    if(lineEdit == m_ui->selfIP_lineEdit){
//        qDebug() << "LineEdit: change self ip.";
        ip = m_ui->selfIP_lineEdit->text();
        success = IPIsRight(parts, ip);
        if(success){
            m_registryData.self.IP = ip;
            m_ui->selfIP_lineEdit->setText(ip);
        }else{
            m_ui->selfIP_lineEdit->setText(m_registryData.self.IP);
            emit UISendMessageBox("警告", "本机IP配置有误！");
        }
    }
    else if(lineEdit == m_ui->selfPort_lineEdit){
//        qDebug() << "LineEdit: change self port.";
        port = m_ui->selfPort_lineEdit->text();

        portInt = port.toInt(&success);
        if(portInt < 1024 || portInt > 65535)
            success = false;

        if(success){
            m_registryData.self.port = portInt;
            m_ui->selfPort_lineEdit->setText(QString::number(portInt));
        }else{
            m_ui->selfPort_lineEdit->setText(QString::number(m_registryData.self.port));
            emit UISendMessageBox("警告", "本机端口号配置有误！");
        }
    }
    else if(lineEdit == m_ui->targetIP_lineEdit){
//        qDebug() << "LineEdit: change target ip.";
        ip = m_ui->targetIP_lineEdit->text();
        success = IPIsRight(parts, ip);
        if(success){
            m_registryData.target.IP = ip;
            m_ui->targetIP_lineEdit->setText(ip);
        }else{
            m_ui->targetIP_lineEdit->setText(m_registryData.target.IP);
            emit UISendMessageBox("警告", "目标IP地址配置有误！");
        }
    }
    else if(lineEdit == m_ui->targetPort_lineEdit){
//        qDebug() << "LineEdit: change target port.";
        port = m_ui->targetPort_lineEdit->text();

        portInt = port.toInt(&success);
        if(portInt < 1024 || portInt > 65535)
            success = false;

        if(success){
            m_registryData.target.port = portInt;
            m_ui->targetPort_lineEdit->setText(QString::number(portInt));
        }else{
            m_ui->targetPort_lineEdit->setText(QString::number(m_registryData.target.port));
            emit UISendMessageBox("警告", "目标端口号配置有误！");
        }
    }
}

/**
 * @brief
 *
 * 该槽函数用于 comboBox 的在线改值响应
 * 读取对应更改的值，并存入注册表中
 *
 * @param 无
 * @return 无
 */
void UIEventHandler::ComboBoxValueChanged()
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(sender());
    if(!comboBox){
        return;
    }

    quint8 comboBoxIndex = comboBox->currentIndex();

    if(comboBox == m_ui->workFreq_comboBox){
        qDebug() << "选择通信频率：" << workFreq_map[comboBoxIndex] << "Hz";
        m_registryData.workFreqIndex = comboBoxIndex;
        if(m_registryData.UDPConnect_flag){
            QByteArray sendArray(1,'A'+comboBoxIndex);
            sendArray = "$$"+sendArray;
            emit sendCommand(sendArray, m_registryData.self, m_registryData.target);
            qDebug() << "发送指令：" << sendArray;
            // 存入注册表
            m_mainWindow->m_setting->setValue(KEY_WORK_FREQUENCY, QString::number(comboBoxIndex));
        }
        else{
            emit UISendMessageBox("警告", "请先打开网络通信再配置参数！");
        }
    }

    else if(comboBox == m_ui->deadFreq_comboBox){
        qDebug() << "选择滤波截止频率：" << deadFreq_map[comboBoxIndex] << "Hz";
        m_registryData.deadFreqIndex = comboBoxIndex;
        if(m_registryData.UDPConnect_flag){
            QByteArray sendArray(1,'a'+comboBoxIndex);
            sendArray = "$$9"+sendArray;
            emit sendCommand(sendArray, m_registryData.self, m_registryData.target);
            qDebug() << "发送指令：" << sendArray;
            // 存入注册表
            m_mainWindow->m_setting->setValue(KEY_DEAD_FREQUENCY, QString::number(comboBoxIndex));
        }
        else{
            emit UISendMessageBox("警告", "请先打开网络通信再配置参数！");
        }
    }

    else if(comboBox == m_ui->range_comboBox){
        qDebug("选择量程范围：-%.1fg ~ %.1fg", accRange_map[comboBoxIndex], accRange_map[comboBoxIndex]);
        m_registryData.accRangeIndex = comboBoxIndex;
        m_registryData.timeRange.ymin = -accRange_map[m_registryData.accRangeIndex];
        m_registryData.timeRange.ymax = accRange_map[m_registryData.accRangeIndex];
        m_ui->time_plot->yAxis->setRange(m_registryData.timeRange.ymin, m_registryData.timeRange.ymax);
        m_ui->time_plot->replot();
        m_registryData.freqRange.ymin = 0;
        m_registryData.freqRange.ymax = accRange_map[m_registryData.accRangeIndex];
        m_ui->freq_plot->yAxis->setRange(m_registryData.freqRange.ymin, m_registryData.freqRange.ymax);
        m_ui->freq_plot->replot();
        // 存入注册表
        m_mainWindow->m_setting->setValue(KEY_ACC_RANGE, QString::number(comboBoxIndex));
    }
}

/**
 * @brief
 *
 * 该槽函数用于 Slider 的在线更改图像范围
 *
 * @param 无
 * @return 无
 */
void UIEventHandler::SliderValueChanged(int value)
{
    // 计算 QCustomPlot 范围，默认 Slider 都是0~1199
    double rate = 1 - value / 1000.0;

    // 查看触发的 Slider
    QSlider *slider = qobject_cast<QSlider*>(sender());
    if(!slider){
        return;
    }

    if(slider == m_ui->time_verticalSlider){
        // 基于当前位置改变 Y 轴范围
        QCPAxisRect *axisRect = m_ui->time_plot->axisRect();
        QCPAxis *yAxis = axisRect->axis(QCPAxis::atLeft);   // 获取左侧Y轴的坐标范围
        double yMiddle = 0.5 * (yAxis->range().lower + yAxis->range().upper);
        double min = rate * m_registryData.timeRange.ymin + yMiddle;
        double max = rate * m_registryData.timeRange.ymax + yMiddle;
        m_ui->time_plot->yAxis->setRange(min, max);
        m_ui->time_plot->replot();
    }
    else if(slider == m_ui->freq_verticalSlider){
//        // 基于当前位置改变 Y 轴范围
//        QCPAxisRect *axisRect = m_ui->freq_plot->axisRect();
//        QCPAxis *yAxis = axisRect->axis(QCPAxis::atLeft);   // 获取左侧Y轴的坐标范围
//        double yMiddle = 0.5 * (yAxis->range().lower + yAxis->range().upper);
        double min = m_registryData.freqRange.ymin;
        double max = rate * m_registryData.freqRange.ymax;
        m_ui->freq_plot->yAxis->setRange(min, max);
        m_ui->freq_plot->replot();
    }
}

/**
 * @brief
 *
 * 该槽函数用于 Slider 的在线更改图像范围
 *
 * @param 无
 * @return 无
 */
void UIEventHandler::PushButtonValueChanged()
{
    // 查看触发的 PushButton
    QPushButton *pushButton = qobject_cast<QPushButton*>(sender());
    if(!pushButton){
        return;
    }

    // 配置地址按钮
    if(pushButton == m_ui->modifyIP_pushButton){        // 配置下位机 UDP 通讯地址
        qDebug() << "按下modifyIP_pushButton，配置参数";
        if(m_registryData.UDPConnect_flag){
            My_HostAddress self, target;
            self.IP = m_ui->selfIP_lineEdit->text();
            self.port = m_ui->selfPort_lineEdit->text().toUShort();
            target.IP = m_ui->targetIP_lineEdit->text();
            target.port = m_ui->targetPort_lineEdit->text().toUShort();

            QStringList list1 = self.IP.split(".");
            QString str1  = list1.at(2);
            QString str2  = list1.at(3);
            QStringList list2 = target.IP.split(".");
            QString str3  = list2.at(2);
            QString str4  = list2.at(3);

            if(str1 == str3){
                QString STR = "$$$" + str1 + '.' + str4 + '.' + str2 + '.' + QString::number(target.port) + '.'+ QString::number(self.port) + '$';
                QByteArray sendArray = STR.toLatin1();
                qDebug() << "发送指令：" << sendArray;
                emit sendCommand(sendArray, self, target);
                // 更新存 IP 地址的全局变量
                m_registryData.self = self;
                m_registryData.target = target;
                // 存入注册表
                m_mainWindow->m_setting->setValue(KEY_SELF_IP, m_registryData.self.IP);
                m_mainWindow->m_setting->setValue(KEY_SELF_PORT, m_registryData.self.port);
                m_mainWindow->m_setting->setValue(KEY_TARGET_IP, m_registryData.target.IP);
                m_mainWindow->m_setting->setValue(KEY_TARGET_PORT, m_registryData.target.port);
            }
            else
                emit UISendMessageBox("警告", "IP未配置在同一网段下，请再次检查");
        }
        else
            emit UISendMessageBox("警告", "请先打开网络通信再配置参数！");
    }

    // 开始通讯按钮
    else if(pushButton == m_ui->startUDP_pushButton){
        QString value = pushButton->text();
        if(value == "开始通讯"){
            qDebug() << "按下startUDP_pushButton，开始通讯";
            pushButton->setText("暂停通讯");
            emit UDPSendStart(m_registryData.self);     // 发送 UDP 开始通讯的信号
        }else if(value == "暂停通讯"){
            qDebug() << "按下startUDP_pushButton，暂停通讯";
            pushButton->setText("开始通讯");
            emit UDPSendStop();                         // 发送 UDP 暂停通讯的信号
        }
    }

    // 数据保存按钮
    else if(pushButton == m_ui->dataSave_pushButton){
        if(m_registryData.UDPConnect_flag){
            QString value = pushButton->text();
            if(value == "数据保存"){
                qDebug() << "按下dataSave_pushButton，开始保存";
                QString fileName = QDateTime::currentDateTime().toString("yyyy.MM.dd.hh.mm.ss.zzz") + ".txt";
                m_file.setFileName(fileName);
                if(!m_file.open(QIODevice::WriteOnly | QIODevice::Text)){
                    emit UISendMessageBox("错误", "打开文件失败");
                }else{
                    m_registryData.saveN = 0;   // 存入数据清零
                    pushButton->setText("结束保存");
                    m_registryData.dataSave_flag = true;
                    // 写入表头
                    QTextStream m_textStream(&m_file);
                    QString writeForm = QString("时间戳(s) 程序运行时间(s) 加表x(g) 加表y(g) 加表z(g)\n");
                    m_textStream << writeForm;
                    // 开始数据保存计时
                    save_timer->start(1000);
                }
            }else if(value == "结束保存"){
                qDebug() << "按下dataSave_pushButton，结束保存";
                pushButton->setText("数据保存");
                m_registryData.dataSave_flag = false;
                save_timer->stop();
                emit UISendMessageBox("数据保存", QString("用时%1s，存入%2组数据，文件名" + m_file.fileName()).arg(save_seconds).arg(m_registryData.saveN));  // 加入文件路径显示
                save_seconds = 0;
                m_file.close();
            }
        }
        else
            emit UISendMessageBox("警告", "请先打开网络通信再保存数据！");
    }

    // 图像清空按钮
    else if(pushButton == m_ui->plotClear_pushButton){
        m_ui->time_plot->graph(0)->data().data()->clear();
    }

}

/**
 * @brief
 *
 * 该槽函数用于 lcdNumber 的时间更新
 *
 * @param QLCDNumber *m_lcdNumber 要显示的时间的 LCDNumber 对象名
 *          quint32 seconds 显示的时间(s)
 * @return 无
 */
void lcdShowTime(QLCDNumber *m_lcdNumber, quint32 seconds)
{
    int hour, minute, second;
    hour = seconds / 3600;
    minute = (seconds - hour * 3600) / 60;
    second = (seconds - hour * 3600) % 60;
    QString strNumber;
    strNumber.sprintf("%02d:%02d:%02d", hour, minute, second);
    m_lcdNumber->display(strNumber);
}

/**
 * @brief
 *
 * 该槽函数用于 UI 界面内数据的定时更新
 * 刷新率由 refreshRate 决定
 * 更新的控件包括 Label
 *
 * @param 无
 * @return 无
 */
void UIEventHandler::updateUI()
{
    // 数据更新
    m_ui->num_label->setText(QString("三角函数周期内的数据点个数: %1").arg(m_registryData.T));
    m_ui->frequency_label->setText(QString("数据每秒生成组数: %1").arg(m_registryData.FREQUENCY_PER_SECOND));
    m_ui->count_label->setText(QString("数据每组生成个数: %1").arg(m_registryData.DATA_COUNT));
    m_ui->fftCount_label->setText(QString("FFT 输入数据量: %1").arg(m_registryData.FFT_N));
    m_ui->fftFrequency_label->setText(QString("FFT 基本频率: %1 Hz").arg(m_registryData.FFT_F));
    m_ui->realFrequency_label->setText(QString("三角函数的实际频率: %1 Hz").arg(m_registryData.FFT_F / m_registryData.T));
    m_ui->realT_label->setText(QString("三角函数的实际周期: %1 s").arg(m_registryData.T / m_registryData.FFT_F));
    m_ui->udpReceive_label->setText(QString("报文接收总数: %1").arg(m_registryData.udpPacket));
    m_ui->accX_label->setText(QString("X轴：%1").arg(m_registryData.accX));
    m_ui->accY_label->setText(QString("Y轴：%1").arg(m_registryData.accY));
    m_ui->accZ_label->setText(QString("Z轴：%1").arg(m_registryData.accZ));
    // 计数时间更新
    lcdShowTime(m_ui->dataSave_lcdNumber, save_seconds);
//    lcdShowTime(m_ui->dataSave_lcdNumber, average_seconds);
}

/**
 * @brief
 *
 * 该槽函数用于数据保存
 * dataX 对应有效数据，dataY 对应保存时间，目前采用的时间格式是程序运行时间，double型，每个点的时间都不同
 * 将数据按指定格式保存成 QString 格式，写入文件
 * 该槽函数目前被 dataPlotter 中的 UI 刷新函数调用，频率不高
 * 后续考虑加入时间戳
 *
 * @param 无
 * @return 无
 */
void UIEventHandler::dataSave(QVector<double> dataX, QVector<double> dataY)
{
    QTextStream m_textStream(&m_file);
    for(int i = 0; i < dataX.size(); i++){
        m_registryData.saveN++;
        // 获取当前时间戳
        QDateTime current_date_time = QDateTime::currentDateTime();
        QString current_date = current_date_time.toString("yyyy.MM.dd.hh.mm.ss.zzz");
        // 写入一行数据
        QString writeForm = QString(current_date + " %1 %2\n").arg(dataX[i]).arg(dataY[i]);
        m_textStream << writeForm;
    }
}
