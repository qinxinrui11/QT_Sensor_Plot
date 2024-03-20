#include "dataprocess.h"

/**
 * @brief 发送缓冲区数据并清空
 *
 * 槽函数，被绘图类 DataPlotter 中的定时器触发，
 * 将缓冲区中的数据以信号的形式发送，然后清空缓冲区。
 *
 * @param 无
 * @return 无
 */
void DataProcess::sendData()
{
    emit dataUpdated(m_bufferX, m_bufferY);
    m_bufferX.clear();
    m_bufferY.clear();
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
void DataProcess::processData(QVector<double> data)
{
    // 计算x坐标（数据接收时间）(us级)
    auto now = std::chrono::high_resolution_clock::now();

    auto runningtime = std::chrono::duration_cast<std::chrono::microseconds>(now - m_startTime);
    qint64 runningTime = static_cast<quint64>(runningtime.count()); // 程序运行时间(us)

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastTime);
    qint64 interval = static_cast<quint64>(duration.count()); // 与上一次接收的时间间隔(us)

    m_lastTime = now;

    // 将收到的数据转存到缓冲区中
    for(int i = 0; i < data.size(); ++i){
        m_bufferX.append(((i + 1) * (double(interval) / data.size()) + runningTime - interval) / 1000000);
        m_bufferY.append(data[i]);
    }
}
