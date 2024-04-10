#include "datagenerator.h"
#include <QtMath>
#include <QRandomGenerator>

DataGenerator::~DataGenerator(){
    delete m_timer;
}

void DataGenerator::start(){
    m_timer->start(1000 / *m_frequencyPerSecond);
}

void DataGenerator::stop(){
    m_timer->stop();
}

/**
 * @brief 数据自动生成
 *
 * 此函数按指定的速率 m_frequencyPerSecond 和组数 m_dataCount 生成数据
 *
 * @param 无
 * @return 无
 */
void DataGenerator::generateData(){
    QVector<double> data;

    for(int i = 0; i < *m_dataCount; i++){
        double value = 5 * qSin(2 * M_PI / *T * m_counter) - 3 * qCos(2 * M_PI / *T * 5 * m_counter); // 示例数据

        double random = 0;  // 加入随机噪声
        random = QRandomGenerator::global()->generateDouble(); // [0,1] 的随机数

        data.append(value + random * 2);
        m_counter += 1;
    }

    emit newDataGenerated(data);
}
