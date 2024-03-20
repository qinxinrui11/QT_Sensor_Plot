#ifndef DATAPLOTTER_H
#define DATAPLOTTER_H

#include <QWidget>
#include <QVector>
#include <QTimer>
#include <chrono>
#include "datagenerator.h"

class QCustomPlot;

struct PlotRange
{
    double xmin;
    double xmax;
    double ymin;
    double ymax;
};

class DataPlotter : public QWidget
{
    Q_OBJECT
public:
    explicit DataPlotter(QCustomPlot *customPlot, PlotRange range, quint8 refreshRate, bool clear_flag, QWidget *parent = nullptr);

public slots:
    void dataBuffer1(QVector<double> data);
    void dataBuffer2(QVector<double> dataX, QVector<double> dataY);

private slots:
    void updatePlot();  // 数据接口，double型的vector数组

private:
    QCustomPlot *m_customPlot;  // QcustomPlot对象
    QVector<double> m_bufferX;  // X轴数据缓冲区
    QVector<double> m_bufferY;  // Y轴数据缓冲区
    QTimer *m_timer;            // 定时器对象
    bool m_clear_flag;          // 图像刷新前是否清除的flag,0不清

    std::chrono::high_resolution_clock::time_point m_startTime = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point m_lastTime = std::chrono::high_resolution_clock::now();
};

#endif // DATAPLOTTER_H
