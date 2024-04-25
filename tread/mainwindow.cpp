#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "uieventhandler.h"

#if _MSC_VER >= 1600    // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif

//#define GENERATE_DATA 1   // 是否使用自定义生成的数据

/**
 * @brief MainWindow 构造函数
 *
 * 此函数包括程序内部各功能类的创建、信号搭建、变量初始化。
 * 包括：
 * 绘图类 DataPlotter、数据生成类 DataGenerator、FFT变换类 FftProcess、
 * UI处理类 UIEventHandler、UDP 数据处理类 UdpProcess
 *
 * @param 无
 * @return 无
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 界面初始化
    // 设置主场景应用图标
//    setWindowIcon(QPixmap(":/HTLEFT.png"));
    // 设置窗口标题
    setWindowTitle("轨道探伤车振动测量分析系统");

    // 开启记录程序运行时间
    m_elapsedTimer.start();

    // 创建一个注册表对象，实现程序自启动时从注册表文件中读取参数，并配置默认值
    m_setting = new QSettings("IVA23.ini", QSettings::IniFormat);   // 配置参数保存在 IVA23.ini 下，在 debug 目录下
    // 读取注册表中参数默认值
    Modifyregistry();

    // 程序数据初始化配置
//    m_registryData.FREQUENCY_PER_SECOND = 200;  // 数据每秒生成的组数
//    m_registryData.DATA_COUNT = 100;            // 每组数据的个数
//    m_registryData.T = 12000;                   // 三角函数的周期，等于 FREQUENCY_PER_SECOND * DATA_COUNT 时1s能绘制一个周期
//    m_registryData.FFT_F = m_registryData.FREQUENCY_PER_SECOND * m_registryData.DATA_COUNT; // FFT 输入数据的频率
    m_registryData.FFT_N = 200;                 // FFT 变换所需的数据量
    m_registryData.FFT_F = workFreq_map[m_registryData.workFreqIndex];// FFT 变换基本频率，同通信频率
    m_registryData.udpPacket = 0;               // UDP 接收报文总数复位
    m_registryData.udpN = 0;                    // UDP 接收报文有效数总数复位
    m_registryData.udpData = 0;                 // UDP 接收的有效数据总数复位
    m_registryData.timeRange.xmin = 0;          // 图像坐标范围
    m_registryData.timeRange.xmax = 60;
    m_registryData.timeRange.ymin = -accRange_map[m_registryData.accRangeIndex];
    m_registryData.timeRange.ymax = accRange_map[m_registryData.accRangeIndex];
    m_registryData.freqRange.xmin = 0 - 0.05 * m_registryData.FFT_F;
    m_registryData.freqRange.xmax = 0.55 * m_registryData.FFT_F;
    m_registryData.freqRange.ymin = 0 - 0.25;
    m_registryData.freqRange.ymax = accRange_map[m_registryData.accRangeIndex] + 0.5;
    m_registryData.UDPConnect_flag = false;     // 默认 UDP 不连接
    m_registryData.dataSave_flag = false;       // 默认数据不保存
    m_registryData.dataAverage_flag = false;    // 默认数据不均值
    m_registryData.startFFT_flag = false;       // 默认 FFT 不运算

    // 创建一个定时器，记录程序运行时间
    m_timer = new QTimer();
    connect(m_timer, &QTimer::timeout, this, &MainWindow::showRunTime); // 以秒为单位显示程序运行时间
    m_timer->start(1000);

    // 创建一个绘图框，用来绘制时域图像
    m_dataPlotter = new DataPlotter(ui->time_plot, m_registryData.timeRange, PLOT_REFRESH_RATE, false, this);
    ui->time_plot->xAxis->setLabel("时间(s)");        // 添加坐标轴名称
    ui->time_plot->yAxis->setLabel("加速度值(g)");     // 添加坐标轴名称
    m_dataPlotter->show();

    // 创建一个绘图框，用来绘制频域图像
    m_freqPlotter = new DataPlotter(ui->freq_plot, m_registryData.freqRange, PLOT_REFRESH_RATE, true, this);
    ui->freq_plot->xAxis->setLabel("频率(Hz)");       // 添加坐标轴名称
    ui->freq_plot->yAxis->setLabel("加速度值(g)");     // 添加坐标轴名称
    m_freqPlotter->show();

#ifndef GENERATE_DATA
    // UDP 数据处理对象
    m_udpProcess = new UdpProcess(m_registryData.self);
    connect(m_udpProcess, &UdpProcess::sendAccData, m_dataPlotter, &DataPlotter::dataBuffer);
    connect(m_udpProcess, &UdpProcess::udpSendMessageBox, this, &MainWindow::showMessageBox);  // 报警框
    connect(m_udpProcess, &UdpProcess::UDPConnectFailed, [&]() {ui->startUDP_pushButton->setText("开始通讯");});
#endif
#ifdef GENERATE_DATA
    // 创建 DataGenerator 数据生成对象
    m_dataGenerator = new DataGenerator(&m_registryData.FREQUENCY_PER_SECOND, &m_registryData.DATA_COUNT, &m_registryData.T, this); // FREQUENCY_PER_SECOND 次每秒，每次生成 DATA_COUNT 个数据
    connect(m_dataGenerator, &DataGenerator::newDataGenerated, m_dataPlotter, &DataPlotter::dataBuffer1);
    // 开启时钟，启动数据生成
    m_dataGenerator->start();
#endif

    // 创建 fft 变换对象
    m_fftProcess = new FftProcess(&m_registryData.FFT_N, &m_registryData.FFT_F);
    // 开启新线程
    // 注册 QVector<double> 类型
    qRegisterMetaType<QVector<double>>("QVector<QVector<double>>");
    m_threadFFT = new QThread();
    m_fftProcess->moveToThread(m_threadFFT);
#ifdef GENERATE_DATA
    connect(m_dataGenerator, &DataGenerator::newDataGenerated, m_fftProcess, &FftProcess::FftCalculate);
#endif
#ifndef GENERATE_DATA
    connect(m_udpProcess, &UdpProcess::sendAccData, m_fftProcess, &FftProcess::FftCalculate);
#endif
    connect(m_fftProcess, &FftProcess::FftResult, m_freqPlotter, &DataPlotter::dataBuffer2);
    m_threadFFT->start();

    // 生成拖动控件，在线改变参数值
    // 创建一个 UI 控件处理对象，用来处理数据更新、指令配置等
    UIEventHandler *m_uiEventHandler = new UIEventHandler(ui, WIDGET_REFRESH_RATE, this);
    connect(m_uiEventHandler, &UIEventHandler::UISendMessageBox, this, &MainWindow::showMessageBox);    // 报警框
    connect(m_uiEventHandler, &UIEventHandler::UDPSendStart, m_udpProcess, &UdpProcess::startSend);     // UDP 开启通讯
    connect(m_uiEventHandler, &UIEventHandler::UDPSendStop, m_udpProcess, &UdpProcess::stopSend);       // UDP 暂停通讯
    connect(m_uiEventHandler, &UIEventHandler::sendCommand, m_udpProcess, &UdpProcess::sendCommand);    // 下位机指令配置
    connect(m_dataPlotter, &DataPlotter::sendData, m_uiEventHandler, &UIEventHandler::dataReceive);     // 数据接收处理
}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief 读取注册表内参数
 *
 * 此函数将从.ini文件中读取注册表内参数，若未保存过，则采用函数内设置的值
 *
 * @param 无
 * @return 无
 */
void MainWindow::Modifyregistry()
{
    QString ip;
    // 主机 IP
    ip = m_setting->value(KEY_SELF_IP, "192.168.88.154").toString();
    m_registryData.self.IP = ip;
    qDebug() << "self IP:" << m_registryData.self.IP;
    // 主机端口
    m_registryData.self.port = m_setting->value(KEY_SELF_PORT, 8234).toUInt();
    qDebug() << "self port:" << m_registryData.self.port;
    // 目标 IP
    ip = m_setting->value(KEY_TARGET_IP, "192.168.88.25").toString();
    m_registryData.target.IP = ip;
    qDebug() << "target IP:" << m_registryData.target.IP;
    // 目标端口
    m_registryData.target.port = m_setting->value(KEY_TARGET_PORT, 20108).toUInt();
    qDebug() << "target port:" << m_registryData.target.port;
    // 数据发送频率下标
    m_registryData.workFreqIndex = m_setting->value(KEY_WORK_FREQUENCY, 0).toUInt();
    qDebug() << "workfrequency:" << workFreq_map[m_registryData.workFreqIndex] << "Hz";
    // 滤波器截止频率下标
    m_registryData.deadFreqIndex = m_setting->value(KEY_DEAD_FREQUENCY, 3).toUInt();
    qDebug() << "deadfrequency:" << deadFreq_map[m_registryData.deadFreqIndex] << "Hz";
    // 加速度计量程范围
    m_registryData.accRangeIndex = m_setting->value(KEY_ACC_RANGE, 1).toUInt();
    qDebug() << "accRange: ±" << accRange_map[m_registryData.accRangeIndex] << "g";
}

/**
 * @brief 显示程序运行时间
 *
 * 此函数使用 LcdNumber 显示 QElapsedTimer 记录的程序运行时间
 *
 * @param 无
 * @return 无
 */
void MainWindow::showRunTime()
{
    int runSeconds = m_elapsedTimer.elapsed() / 1000;
    int hour, minute, seconds;
    hour = runSeconds / 3600;
    minute = (runSeconds - hour * 3600) / 60;
    seconds = (runSeconds - hour * 3600) % 60;
    QString strNumber;
    strNumber.sprintf("%02d:%02d:%02d", hour, minute, seconds);
    ui->runTime_lcdNumber->display(strNumber);
}


void MainWindow::showMessageBox(QString type, QString message)
{
    QMessageBox::warning(this, type, message);
}
