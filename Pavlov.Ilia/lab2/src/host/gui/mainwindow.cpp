#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "../host.h"

#include <string>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::WriteLog(const std::string& msg) {
    ui->gameLogText->append(msg.c_str());
}

void MainWindow::SetPid(pid_t pid) {
    ui->hostPidText->setText(std::to_string(pid).c_str());
}

static int _random(int minNum, int maxNum) {
  return int((1.0*rand() + 1) / (1.0*RAND_MAX + 1) * (maxNum - minNum) + minNum);
}

int MainWindow::GetNumber() {
    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(this, &MainWindow::on_enterNumButton_clicked, &loop, &QEventLoop::quit );
    connect( &timer, &QTimer::timeout, &loop, &QEventLoop::quit );
    ui->enterNumButton->setEnabled(true);
    timer.start(timeout);
    loop.exec();
    int num;
    if(timer.isActive()) {
        num = ui->numSpin->value();
        ui->gameLogText->append(("Вы ввели " + std::to_string(num)).c_str());
    }
    else {
        num = _random(0, 101);
        ui->gameLogText->append(("Число выбрано случайно " + std::to_string(num)).c_str());
    }
    ui->enterNumButton->setEnabled(false);
    return num;
}
