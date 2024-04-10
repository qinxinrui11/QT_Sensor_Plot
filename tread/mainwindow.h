#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QSettings>
#include <QElapsedTimer>
#include "dataplotter.h"
#include "fftprocess.h"
#include "udpprocess.h"
#include "appdata.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    Ui::MainWindow *ui;

    DataPlotter *m_dataPlotter;             // 时域图像
    DataPlotter *m_freqPlotter;             // 频域图像

    QSettings *m_setting;                   // 注册表对象

    UdpProcess *m_udpProcess;               // UDP 数据处理对象

    DataGenerator *m_dataGenerator;         // 数据生成对象

    FftProcess *m_fftProcess;               // FFT 处理对象
    QThread *m_threadFFT;                   // 用于 FFT 计算的新线程

    QElapsedTimer m_elapsedTimer;           // 记录程序运行时间
    QTimer *m_timer;                        // 定时器调用 lcd 更新

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void Modifyregistry();
    void showRunTime();

private slots:
    void showMessageBox(QString type, QString message);       // 用于显示报警信息
};
#endif // MAINWINDOW_H
