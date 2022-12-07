#include <unistd.h>
#include <QList>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QString>
#include <QPalette>
#include <QStringList>

#include "gui.h"
#include "ui_MainWindow.h"

GUI::GUI(sendClb SendFunc, getClb GetFunc, tClb TimeFunc, isSmthClb IsTerminated, isSmthClb IsConnected,
            isSmthClb IsNeedToSend) : QMainWindow((QWidget*)0), _ui(new Ui::MainWindow),
        _send(SendFunc), _get(GetFunc), _time(TimeFunc), _isTerminated(IsTerminated), _isConnected(IsConnected),
        _isNeedToSend(IsNeedToSend) {
    _ui->setupUi(this);

    _ui->lineEdit->setValidator(new QIntValidator(1, 100, this));
    QObject::connect(_ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(sendNum()));

    _ui->tableView->setShowGrid(true);
    _ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _model = new QStandardItemModel(this);
    QStringList lbls = QString("Волк, Козленок, Состояние козленка").simplified().split(",");
    _model->setHorizontalHeaderLabels(lbls);
    _ui->tableView->setModel(_model);
    _ui->tableView->setColumnWidth(0, _ui->tableView->width() / 5);
    _ui->tableView->setColumnWidth(1, _ui->tableView->width() / 4);
    _ui->tableView->setColumnWidth(2, _ui->tableView->width() / 2);
    _ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    _ui->lcdNumber->setSegmentStyle(QLCDNumber::Flat);
    QPalette p = _ui->lcdNumber->palette();
    p.setColor(p.WindowText, Qt::red);
    _ui->lcdNumber->setPalette(p);

    _timer = new QTimer(this);
    QObject::connect(_timer, SIGNAL(timeout()), this, SLOT(regAction()));
    _timer->start(100);
}

void GUI::SetTitle(const char *windowName) {
    setWindowTitle(windowName);
}

void GUI::sendNum() {
    if (!_time() || !_isNeedToSend())
        return;

    bool ok;
    int num = _ui->lineEdit->text().toInt(&ok, 10);
    if (ok) {
        _send(num);
        _ui->lineEdit->clear();
    }
}

void GUI::regAction() {
    static bool needToClear = false;

    if (_isTerminated())
        this->close();

    if (_isConnected()) {
        _ui->statusbar->showMessage("Client connected");
        if (needToClear) {
            _model->removeRows(0, _model->rowCount());
            needToClear = false;
        }
    }
    else {
        _ui->statusbar->showMessage("Client not connected");
        if (!needToClear)
            needToClear = true;
    }

    int t = _time();
    t = t < 0 ? 0 : t;
    _ui->lcdNumber->display(t);

    GState st;
    if (_get(&st)) {
        QList<QStandardItem *> l;
        l.append(new QStandardItem(QString("%1").arg(st.wolfN)));
        l.append(new QStandardItem(QString("%1").arg(st.goatN)));
        l.append(new QStandardItem(QString(st.goatSt.c_str())));
        _model->appendRow(l);
    }
}

GUI::~GUI() {
    delete _ui;
}