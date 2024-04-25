#ifndef FFTPROCESS_H
#define FFTPROCESS_H

#include <QObject>
#include <QVector>

class FftProcess : public QObject
{
    Q_OBJECT
public:
    explicit FftProcess(quint32 *n, float *fs) : m_FS(fs), m_nums(n) {};

signals:
    void FftResult(QVector<double> freq, QVector<QVector<double>> ampl);

public slots:
    void FftCalculate(QVector<QVector<double>> data);

private:
    float *m_FS;                            // 数据的采样频率
    quint32 *m_nums;                        // fft 最大一次计算所用的数据量
    QVector<QVector<double>> m_dataBuffer;    // fft 输入数据缓冲区
    QVector<QVector<double>> ampl;          // fft 计算出的幅值
    QVector<double> freq;                   // fft 计算出的频率
};

#endif // FFTPROCESS_H
