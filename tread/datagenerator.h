﻿#ifndef DATAGENERATOR_H
#define DATAGENERATOR_H

#include <QObject>
#include <QTimer>
#include <QVector>

// 模拟UDP数据接口
class DataGenerator : public QObject
{
    Q_OBJECT
public:
    explicit DataGenerator(quint16 *frequencyPerSecond, quint16 *dataCount, quint32 *t, QObject *parent = nullptr) :
        QObject(parent),
        m_frequencyPerSecond(frequencyPerSecond),
        m_dataCount(dataCount),
        T(t),
        m_timer(new QTimer(this)),
        m_counter(0){
        connect(m_timer, &QTimer::timeout, this, &DataGenerator::generateData);
    };

    ~DataGenerator();

    void start();

    void stop();

signals:
    void newDataGenerated(QVector<double> data);

private slots:
    void generateData();

private:
    quint16 *m_frequencyPerSecond;   // 每秒执行次数
    quint16 *m_dataCount;            // 每次生成的数据个数
    quint32 *T;                  // 三角函数的周期
    QTimer *m_timer;            // 定时器
    double m_counter;           // 计数器，用于生成数据
};

#endif // DATAGENERATOR_H
