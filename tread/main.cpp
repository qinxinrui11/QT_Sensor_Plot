#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    a.setApplicationName("轨道探伤车振动测量分析系统");
//    a.setApplicationVersion("V24.4.16");
//    a.setOrganizationName("海天探索者");
    MainWindow w;
    w.showMaximized(); // 默认最大化窗口
    return a.exec();
}
