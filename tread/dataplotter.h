﻿#ifndef DATAPLOTTER_H
#define DATAPLOTTER_H

#include <QWidget>
#include <QVector>
#include <QTimer>
#include <chrono>
#include "datagenerator.h"
#include "appdata.h"

class QCustomPlot;

class DataPlotter : public QWidget
{
    Q_OBJECT
public:
    explicit DataPlotter(QCustomPlot *customPlot, PlotRange range, quint8 refreshRate, bool clear_flag, QWidget *parent = nullptr);

public slots:
    void dataBuffer(QVector<QVector<double>> data);
    void dataBuffer2(QVector<double> dataX, QVector<QVector<double>> dataY);

private slots:
    void updatePlot();  // 数据接口，double型的vector数组

private:
    QCustomPlot *m_customPlot;  // QcustomPlot对象
    QVector<double> m_bufferX;  // X轴数据缓冲区
    QVector<QVector<double>> m_bufferY; // Y轴数据（多个）缓冲区
    QTimer *m_timer;            // 定时器对象
    bool m_clear_flag;          // 图像刷新前是否清除的 flag, 0 不清

    std::chrono::high_resolution_clock::time_point m_startTime = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point m_lastTime = std::chrono::high_resolution_clock::now();

signals:
    void sendData(QVector<double> dataX, QVector<QVector<double>> dataY);
};

#endif // DATAPLOTTER_H
