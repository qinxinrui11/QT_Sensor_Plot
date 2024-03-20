#include "mainwindow.h"
#include "ui_mainwindow.h"

#define UI_REFRESH_RATE 25 // UI界面每秒更新的次数

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 程序数据初始化
    FREQUENCY_PER_SECOND = 200; // 数据每秒生成的组数
    DATA_COUNT = 100; // 每组数据的个数
    T = 12000;   // 三角函数的周期，等于 FREQUENCY_PER_SECOND * DATA_COUNT 时1s能绘制一个周期
    FFT_N = 4000; // FFT 变换所需的数据量
    FFT_F = FREQUENCY_PER_SECOND * DATA_COUNT; // FFT 输入数据的频率

    // 声明一个绘图框，用来绘制时域图像
    PlotRange dataRange; // 设置 plot 的量程
    dataRange.xmin = 0; dataRange.xmax = 60; dataRange.ymin = -6; dataRange.ymax = 6;
    m_dataPlotter = new DataPlotter(ui->time_plot, dataRange, UI_REFRESH_RATE, false, this);
    m_dataPlotter->show();

    // 创建 DataGenerator 数据生成对象
    m_dataGenerator = new DataGenerator(&FREQUENCY_PER_SECOND, &DATA_COUNT, &T, this); // FREQUENCY_PER_SECOND 次每秒，每次生成 DATA_COUNT 个数据
    connect(m_dataGenerator, &DataGenerator::newDataGenerated, m_dataPlotter, &DataPlotter::dataBuffer1);
    // 开启时钟，启动数据生成
    m_dataGenerator->start();

    // 声明一个绘图框，用来绘制频域图像
    PlotRange freqRange; // 设置 plot 的量程
    freqRange.xmin = -100; freqRange.xmax = 1000; freqRange.ymin = -2; freqRange.ymax = 10;
    m_freqPlotter = new DataPlotter(ui->freq_plot, freqRange, UI_REFRESH_RATE, true, this);
    m_freqPlotter->show();

    // 创建 fft 变换对象
    m_fftProcess = new FftProcess(&FFT_N, &FFT_F);
    // 开启新线程
    // 注册 QVector<double> 类型
    qRegisterMetaType<QVector<double>>("QVector<double>");
    m_thread = new QThread();
    m_fftProcess->moveToThread(m_thread);
    connect(m_dataGenerator, &DataGenerator::newDataGenerated, m_fftProcess, &FftProcess::FftCalculate);
    connect(m_fftProcess, &FftProcess::FftResult, m_freqPlotter, &DataPlotter::dataBuffer2);
    m_thread->start();

    // 生成拖动控件，在线改变参数值
    m_scrollBars.append(ui->num_scroll);
    connect(ui->num_scroll, &QScrollBar::valueChanged, this, &MainWindow::onScrollBarValueChanged);
    m_scrollBars.append(ui->Frequency_scroll);
    connect(ui->Frequency_scroll, &QScrollBar::valueChanged, this, &MainWindow::onScrollBarValueChanged);
    m_scrollBars.append(ui->Count_scroll);
    connect(ui->Count_scroll, &QScrollBar::valueChanged, this, &MainWindow::onScrollBarValueChanged);
    m_scrollBars.append(ui->FFT_Count_scroll);
    connect(ui->FFT_Count_scroll, &QScrollBar::valueChanged, this, &MainWindow::onScrollBarValueChanged);
    m_labels.append(ui->num_Label);
    m_labels[0]->setText(QStringLiteral("三角函数周期内的数据点个数: %1").arg(T));
    m_labels.append(ui->Frequency_Label);
    m_labels[1]->setText(QStringLiteral("数据每秒生成组数: %1").arg(FREQUENCY_PER_SECOND));
    m_labels.append(ui->Count_Label);
    m_labels[2]->setText(QStringLiteral("数据每组生成个数: %1").arg(DATA_COUNT));
    m_labels.append(ui->FFT_Count_Label);
    m_labels[3]->setText(QStringLiteral("FFT 输入数据量: %1").arg(FFT_N));
    m_labels.append(ui->FFT_Frequency_Label);
    m_labels[4]->setText(QStringLiteral("FFT 基本频率: %1 Hz").arg(FFT_F));
    m_labels.append(ui->Real_Frequency_Label);
    m_labels[5]->setText(QStringLiteral("三角函数的实际频率: %1 Hz").arg(FFT_F / T));
    m_labels.append(ui->Real_T_Label);
    m_labels[6]->setText(QStringLiteral("三角函数的实际周期: %1 s").arg(T / FFT_F));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onScrollBarValueChanged(int value)
{
    QScrollBar *scrollBar = qobject_cast<QScrollBar*>(sender());
    if(!scrollBar)
        return;

    int index = m_scrollBars.indexOf(scrollBar);
    qDebug() << index;
    switch(index){
        case 0:
            T = value;
            m_labels[0]->setText(QStringLiteral("三角函数周期内的数据点个数: %1").arg(T));
            m_labels[5]->setText(QStringLiteral("三角函数的实际频率: %1 Hz").arg(FFT_F / T));
            m_labels[6]->setText(QStringLiteral("三角函数的实际周期: %1 s").arg(T / FFT_F));
            break;
        case 1:
            FREQUENCY_PER_SECOND = value;
            m_labels[1]->setText(QStringLiteral("数据每秒生成组数: %1").arg(FREQUENCY_PER_SECOND));
            FFT_F = FREQUENCY_PER_SECOND * DATA_COUNT; // FFT 输入数据的频率
            m_labels[4]->setText(QStringLiteral("FFT 基本频率: %1 Hz").arg(FFT_F));
            m_labels[5]->setText(QStringLiteral("三角函数的实际频率: %1 Hz").arg(FFT_F / T));
            m_labels[6]->setText(QStringLiteral("三角函数的实际周期: %1 s").arg(T / FFT_F));
            m_dataGenerator->start();
            break;
        case 2:
            DATA_COUNT = value;
            m_labels[2]->setText(QStringLiteral("数据每组生成个数: %1").arg(DATA_COUNT));
            FFT_F = FREQUENCY_PER_SECOND * DATA_COUNT; // FFT 输入数据的频率
            m_labels[4]->setText(QStringLiteral("FFT 基本频率: %1 Hz").arg(FFT_F));
            m_labels[5]->setText(QStringLiteral("三角函数的实际频率: %1 Hz").arg(FFT_F / T));
            m_labels[6]->setText(QStringLiteral("三角函数的实际周期: %1 s").arg(T / FFT_F));
            break;
        case 3:
            FFT_N = value;
            m_labels[3]->setText(QStringLiteral("FFT 输入数据量: %1").arg(FFT_N));
            break;
        default:
            break;
    }
}
