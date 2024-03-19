#ifndef DATAPLOTTER_H
#define DATAPLOTTER_H

#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QElapsedTimer>
#include "datagenerator.h"

class QCustomPlot;

class DataPlotter : public QWidget
{
    Q_OBJECT
public:
    explicit DataPlotter(QCustomPlot *customPlot, QWidget *parent = nullptr);

private slots:
    void updatePlot();  // 数据接口，double型的vector数组
    void dataBuffer(QVector<double> data);

private:
    QCustomPlot *m_customPlot;  // QcustomPlot对象
    QVector<double> m_bufferX;  // X轴数据缓冲区
    QVector<double> m_bufferY;  // Y轴数据缓冲区
    QTimer *m_timer;            // 定时器对象
    QElapsedTimer m_timerMainWindow;     // 程序计时对象

    DataGenerator *m_dataGenerator; // 数据生成对象
};

#endif // DATAPLOTTER_H
