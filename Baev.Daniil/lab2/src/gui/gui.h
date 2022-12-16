#pragma once

#include <string>
#include <QMainWindow>
#include <QtGui>
#include <QTableView>
#include <QApplication>
#include <QTimer>
#include <memory>
#include "model.hpp"

namespace Ui {
    class MainWindow;
}

class GUI : public QMainWindow {
Q_OBJECT
public:
    GUI(std::shared_ptr<GameState> gst);

    void SetTitle(const char *windowName);

    ~GUI();
signals:
    void wolfNumberSend(int number);
private slots:
    void wolfNumberEnter();
    void viewModel();
public slots:
    void gameover();
private:
    Ui::MainWindow *_ui;

    QStandardItemModel *_model;

    QTimer *_timer;

    std::shared_ptr<GameState> _gst;

    QString goatStateStr(GOAT_STATE state);
    QString connectStatusStr(CONNECT_STATUS state);

    GUI(const GUI&) = delete;
    GUI& operator=(const GUI&) = delete;
};