#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mario = new Player(this);
}

MainWindow::~MainWindow()
{
    delete mario;
    delete ui;
}
