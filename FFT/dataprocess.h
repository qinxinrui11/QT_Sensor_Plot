#ifndef DATAPROCESS_H
#define DATAPROCESS_H

#include <QThread>
#include <QVector>
#include <chrono>
#include <QMutex>

class DataProcess : public QObject
{
    Q_OBJECT
public:
    explicit DataProcess(QObject *parent = nullptr) : QObject(parent) {};

signals:
    void dataUpdated(QVector<double> dataX, QVector<double> dataY);

public slots:
    void sendData();
    void processData(QVector<double> data);

private:
    std::chrono::high_resolution_clock::time_point m_startTime = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point m_lastTime = std::chrono::high_resolution_clock::now();
    QVector<double> m_bufferX;
    QVector<double> m_bufferY;
};

#endif // DATAPROCESS_H
