#include "datagenerator.h"
#include <QtMath>
#include <QRandomGenerator>

#define T 300000   // 数据生成的周期，等于 FREQUENCY_PER_SECOND * DATA_COUNT 时1s能绘制一个周期

DataGenerator::~DataGenerator(){
    delete m_timer;
}

void DataGenerator::start(){
    m_timer->start(1000 / this->m_frequencyPerSecond);
}

void DataGenerator::stop(){
    m_timer->stop();
}

void DataGenerator::generateData(){
    QVector<double> data;

    for(int i = 0; i < m_dataCount; i++){
        double value = qSin(2 * M_PI / T * m_counter); // 示例数据
        double random = 0;  // 加入随机数
//        random = QRandomGenerator::global()->generateDouble(); // [0,1]的随机数
        data.append(value + random * 0.3);
        m_counter += 1;
    }

    emit newDataGenerated(data);
}
