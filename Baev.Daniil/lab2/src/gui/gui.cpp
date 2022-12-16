#include <unistd.h>
#include <QList>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QString>
#include <QPalette>
#include <QStringList>
#include <sys/syslog.h>

#include "gui.h"
#include "../host/host.h"
#include "ui_MainWindow.h"

GUI::GUI(std::shared_ptr<GameState> gst): QMainWindow((QWidget*)0), _ui(new Ui::MainWindow), _gst(gst){
    _ui->setupUi(this);

    _ui->wolfNumber->setValidator(new QIntValidator(1, 100, this));
    _ui->tableView->setShowGrid(true);
    _ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _model = new QStandardItemModel(this);
    QStringList lbls = QString("Число козленка, Состояние козленка, Подключение").simplified().split(",");
    _model->setHorizontalHeaderLabels(lbls);
    _ui->tableView->setModel(_model);
    _ui->tableView->setColumnWidth(0, _ui->tableView->width() * 0.2);
    _ui->tableView->setColumnWidth(1, _ui->tableView->width() * 0.4);
    _ui->tableView->setColumnWidth(2, _ui->tableView->width() * 0.4);
    _ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    QObject::connect(_ui->wolfNumber, SIGNAL(returnPressed()), this, SLOT(wolfNumberEnter()));
    _timer = new QTimer(this);
    QObject::connect(_timer, SIGNAL(timeout()), this, SLOT(viewModel()));
    _timer->start(100);
}


void GUI::wolfNumberEnter() {
    bool ok;
    int num = _ui->wolfNumber->text().toInt(&ok, 10);
    if (ok) {
        emit wolfNumberSend(num);
        _ui->wolfNumber->clear();
        _ui->statusbar->showMessage(QString("Вы ввели: %1").arg(num));
    }
}

void GUI::gameover(){
    _ui->statusbar->showMessage("Конец игры");
}

void GUI::SetTitle(const char *windowName) {
    setWindowTitle(windowName);
}

QString GUI::goatStateStr(GOAT_STATE state){
    switch (state){
    case GOAT_STATE::ALIVE:
        return QString("Жив");
    case GOAT_STATE::DEAD:
        return QString("Мёртв");
    default:
        return QString("Error");
    }
}

QString GUI::connectStatusStr(CONNECT_STATUS state){
    switch (state){
    case CONNECT_STATUS::CONNECTION:
        return QString("Подключен");
    case CONNECT_STATUS::DISCONNECTION:
        return QString("Нет подключения");
    case CONNECT_STATUS::LOAD:
        return QString("Ожидание...");
    default:
        return QString("Error");
    }
}

void GUI::viewModel(){
    if(Host::_isTerminated)
        this->close();
    _ui->lastWolfNumber->display(_gst->wolfNumber.load());
    _ui->aliveNumber->display(_gst->aliveNumber.load());
    _ui->deadNumber->display(_gst->deadNumber.load());
    if(_gst->time.load() >= 0){
        _ui->moveTime->display(_gst->time.load());
    }

    _model->removeRows(0, _model->rowCount());
    for (size_t i = 0; i < _gst->goats.len(); ++i){
        QList<QStandardItem *> l;
        l.append(new QStandardItem(QString("%1").arg(_gst->goats[i]->number)));
        l.append(new QStandardItem(QString(goatStateStr(_gst->goats[i]->state))));
        l.append(new QStandardItem(QString(connectStatusStr(_gst->goats[i]->clientStatus))));
        _model->appendRow(l);
    }
}

GUI::~GUI() {
    delete _ui;
    delete _model;
    delete _timer;
}