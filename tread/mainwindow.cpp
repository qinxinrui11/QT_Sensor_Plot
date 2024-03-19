#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 声明一个绘图框，用来绘制时域图像
    m_dataPlotter = new DataPlotter(ui->time_plot, this);
    m_dataPlotter->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

