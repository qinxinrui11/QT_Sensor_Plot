#ifndef UIEVENTHANDLER_H
#define UIEVENTHANDLER_H

#include <QObject>
#include <QScrollBar>
#include <QLineEdit>
#include <QComboBox>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"

class MainWindow;

class UIEventHandler : public QObject
{
    Q_OBJECT
public:
    UIEventHandler(Ui::MainWindow *ui, quint8 refreshRate, MainWindow *mainWindow);

public slots:
    void ScrollBarValueChanged(int value);                          // 通过 ScrollBar 滚动条改变内部参数的槽函数
    void LineEditValueChanged();                                    // 通过 LineEdit 改变内部参数的槽函数
    void ComboBoxValueChanged();                                    // 通过 ComboBox 改变内部参数的槽函数
    void SliderValueChanged(int value);                             // 通过 Slider 改变图像范围的槽函数
    void PushButtonValueChanged();                                  // 通过 PushButton 按钮改变内部执行状态
    void dataSave(QVector<double> dataX, QVector<double> dataY);    // 数据保存

private:
    Ui::MainWindow *m_ui;
    MainWindow *m_mainWindow;

    QList<QLineEdit*> m_lineEdits;                          // LineEdit 列表
    QList<QComboBox*> m_comboBoxs;                          // ComboBox 列表

    QTimer *m_timer;                                        // UI 刷新的定时器对象
    QTimer *save_timer;                                     // 记录数据保存时间的定时器对象

    QFile m_file;                                           // 数据保存的文件对象

    quint32 save_seconds;                                   // 数据保存的时间(s)

private:
    void ModifyParameter();                                 // 参数初始化
    void updateUI();                                        // UI 界面定时更新

signals:
    void UISendMessageBox(QString type, QString context);   // 报警框信息
    void UDPSendStart(My_HostAddress self);                 // 开启 UDP 通讯
    void UDPSendStop();                                     // 暂停 UDP 通讯
    void sendCommand(QByteArray, My_HostAddress, My_HostAddress);   // 下位机指令
};

#endif // UIEVENTHANDLER_H
