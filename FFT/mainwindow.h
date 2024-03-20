#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScrollBar>
#include <QLabel>
#include <QThread>
#include "dataplotter.h"
#include "fftprocess.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    DataPlotter *m_dataPlotter;
    DataPlotter *m_freqPlotter;

    DataGenerator *m_dataGenerator;     // 数据生成对象
    FftProcess *m_fftProcess;           // FFT 处理对象

    QThread *m_thread;                  // 用于 FFT 计算的新线程

    QList<QScrollBar*> m_scrollBars;    // 存储滚动条
    QList<QLabel*> m_labels;            // 存储 Label

    quint16 FREQUENCY_PER_SECOND;       // 数据每秒生成的组数
    quint16 DATA_COUNT;                 // 每组数据的个数
    quint16 T;                          // 三角函数的周期，等于 FREQUENCY_PER_SECOND * DATA_COUNT 时1s能绘制一个周期

    quint32 FFT_N;                      // FFT 变换所需的数据量
    float FFT_F;                      // FFT 输入数据的频率

private slots:
    void onScrollBarValueChanged(int value);    // 通过滚动条改变内部参数的槽函数
};
#endif // MAINWINDOW_H
