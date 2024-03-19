#include "dataplotter.h"
#include "qcustomplot.h"
#include <QDateTime>
#include <chrono>

#define FREQUENCY_PER_SECOND 400 // 数据每秒生成的组数
#define DATA_COUNT 100 // 每组数据的个数

#define UI_REFRESH_RATE 10 // UI界面每秒更新的次数

/**
 * @brief 绘图类的初始化
 *
 * 此函数用于配置父类传来的 QcustomPlot 参数。
 * 使用数据缓冲区+定时器的方式绘制全部数据。
 * 声明一个数据生成类，频率和个数由宏定义设置。
 *
 * @param *customPlot   UI 界面的 QCustomPlot 控件
 * @param *parent       QWidget 父类
 * @return 无
 */
DataPlotter::DataPlotter(QCustomPlot *customPlot, QWidget *parent) : QWidget(parent)
{
    /****************************************/
    // 定义 QCustomPlot 控件参数
    m_customPlot = customPlot;
    m_customPlot->setInteractions(QCP::iRangeDrag   // 可平移
                         | QCP::iRangeZoom          // 可滚轮缩放
                         | QCP::iSelectLegend       // 可选中图例
                         | QCP::iSelectPlottables); // 可选中曲线

    // 初始化 Plot 量程
    m_customPlot->xAxis->setRange(0,100);   // X轴为时间(s)
    m_customPlot->yAxis->setRange(-3,3);    // Y轴为加速度值(g)

    // 创建一个新图层
    m_customPlot->addGraph();
    /****************************************/
    // 初始化定时器，每隔一段时间触发绘图操作
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DataPlotter::updatePlot);
    m_timer->start(1000 / UI_REFRESH_RATE); // 设置定时器间隔(ms)
    /****************************************/
    // 创建 DataGenerator 对象并连接信号
    m_dataGenerator = new DataGenerator(FREQUENCY_PER_SECOND, DATA_COUNT, this); // 200次每秒，每次生成100个数据
    connect(m_dataGenerator, &DataGenerator::newDataGenerated, this, &DataPlotter::dataBuffer);

    // 开启时钟，启动数据生成
    m_dataGenerator->start();
    /****************************************/
    // 开启程序运行计时器
    m_timerMainWindow.start();
}

/**
 * @brief 定时更新 Plot
 *
 * 此函数被定时器调用，刷新频率由宏定义配置。
 * 会用 qDebug 输出数据接收情况以及程序运行时间。
 *
 * @param 无
 * @return 无
 */
void DataPlotter::updatePlot()
{
    if(!m_bufferX.isEmpty() && !m_bufferY.isEmpty()){
        // 设置绘图数据
        m_customPlot->graph(0)->addData(m_bufferX, m_bufferY);

        // 重新绘制
        m_customPlot->replot();

        m_bufferX.clear();
        m_bufferY.clear();
    }else{
        qDebug() << "buffer is empty";
    }
}

/**
 * @brief 接收数据并放入缓冲区
 *
 * 槽函数，被数据生成类 DataGenerator 中的信号 newDataGenerated 触发。
 * 根据数据量计算出合适的X轴坐标——接收到数据时的秒数。
 *
 * @param data  double 型的 QVector 数组
 * @return 无
 */
void DataPlotter::dataBuffer(QVector<double> data)
{
    // 计算x坐标（数据接收时间）(us级)
    static auto start = std::chrono::high_resolution_clock::now();

    auto now = std::chrono::high_resolution_clock::now();
    auto runningtime = std::chrono::duration_cast<std::chrono::microseconds>(now - start);
    qint64 runningTime = static_cast<quint64>(runningtime.count()); // 程序运行时间(us)

    static auto last = now;
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - last);
    last = now;
    qint64 interval = static_cast<quint64>(duration.count()); // 与上一次接收的时间间隔(us)

    // 将收到的数据转存到缓冲区中
    for(int i = 0; i < data.size(); ++i){
        m_bufferX.append(((i + 1) * (double(interval) / data.size()) + runningTime - interval) / 1000000);
        m_bufferY.append(data[i]);
    }
}
