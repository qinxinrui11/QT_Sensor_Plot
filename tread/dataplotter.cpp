#include "dataplotter.h"
#include "qcustomplot.h"
#include <QDateTime>

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
DataPlotter::DataPlotter(QCustomPlot *customPlot, PlotRange range, quint8 refreshRate, bool clear_flag, QWidget *parent) : QWidget(parent)
{
    /****************************************/
    m_customPlot = customPlot;

    // 初始化 Plot 量程
    m_customPlot->xAxis->setRange(range.xmin, range.xmax);    // X轴为时间(s)
    m_customPlot->yAxis->setRange(range.ymin, range.ymax);    // Y轴为加速度值(g)

    // 创建一个新图层
    m_customPlot->addGraph();
    m_customPlot->addGraph();
    m_customPlot->addGraph();

    // 设置曲线颜色,宽度（也可以设置类型）
    QPen pen;
    pen.setWidth(1);
    pen.setColor(Qt::blue);
    m_customPlot->graph(0)->setPen(pen); // line color blue for first graph
    pen.setColor(Qt::red);
    m_customPlot->graph(1)->setPen(pen); // line color red for second graph
    pen.setColor(Qt::green);
    m_customPlot->graph(2)->setPen(pen); // line color green for third graph

    // 设置图例
    m_customPlot->graph(0)->setName("Ax");
    m_customPlot->graph(1)->setName("Ay");
    m_customPlot->graph(2)->setName("Az");
    m_customPlot->legend->setVisible(true);

    // 定义 QCustomPlot 控件参数
    m_customPlot->setInteractions(QCP::iRangeDrag   // 可平移
                         | QCP::iRangeZoom          // 可滚轮缩放
                         | QCP::iSelectLegend       // 可选中图例
                         | QCP::iSelectPlottables); // 可选中曲线
    /****************************************/
    // 初始化定时器，每隔一段时间触发绘图操作
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DataPlotter::updatePlot);
    m_timer->start(1000 / refreshRate); // 设置定时器间隔(ms)

    m_clear_flag = clear_flag;
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
    if(m_clear_flag){
        m_customPlot->graph(0)->data().data()->clear();
        m_customPlot->graph(1)->data().data()->clear();
        m_customPlot->graph(2)->data().data()->clear();
    }
    if(!m_bufferX.isEmpty() && (!m_bufferY.isEmpty())){
        // 设置绘图数据
        if(m_bufferY.size()){
            QVector<double> accx, accy, accz;
            for(auto it : m_bufferY){
                accx.append(it[0]);
                accy.append(it[1]);
                accz.append(it[2]);
            }
            m_customPlot->graph(0)->addData(m_bufferX, accx);
            m_customPlot->graph(1)->addData(m_bufferX, accy);
            m_customPlot->graph(2)->addData(m_bufferX, accz);
        }

        // 发送保存的数据
        if(m_registryData.dataSave_flag || m_registryData.dataAverage_flag || m_registryData.autoAverage_flag)
            emit sendData(m_bufferX, m_bufferY);

        // 重新绘制
        m_customPlot->replot();

        m_bufferX.clear();
        m_bufferY.clear();
    }else{
//        qDebug() << "buffer is empty";
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
void DataPlotter::dataBuffer(QVector<QVector<double>> data)
{
    // 计算x坐标（数据接收时间）(us级)
    auto now = std::chrono::high_resolution_clock::now();

    auto runningtime = std::chrono::duration_cast<std::chrono::microseconds>(now - m_startTime);
    qint64 runningTime = static_cast<quint64>(runningtime.count()); // 程序运行时间(us)

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastTime);
    qint64 interval = static_cast<quint64>(duration.count());       // 与上一次接收的时间间隔(us)

    m_lastTime = now;

    // 将收到的数据转存到缓冲区中
    for(int i = 0; i < data.size(); i++){
        m_bufferX.append(((i + 1) * (double(interval) / data.size()) + runningTime - interval) / 1000000);
        m_bufferY.append(data[i]);
    }
}

/**
 * @brief 接收数据并放入缓冲区
 *
 * 槽函数，直接接受 x 和 y 值
 *
 * @param data  double 型的 QVector 数组
 * @return 无
 */
void DataPlotter::dataBuffer2(QVector<double> dataX, QVector<QVector<double>> dataY)
{
    // 将收到的数据转存到缓冲区中
    for(int i = 0; i < dataX.size(); i++){
        m_bufferX.append(dataX[i]);
        QVector<double> tmp;
        tmp.append(dataY[i][0]);
        tmp.append(dataY[i][1]);
        tmp.append(dataY[i][2]);
        m_bufferY.append(tmp);
    }
}
