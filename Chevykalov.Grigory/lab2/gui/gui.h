#ifndef __GUI_H_
#define __GUI_H_

#include <string>
#include <QMainWindow>
#include <QtGui>
#include <QTableView>
#include <QTimer>
#include <QApplication>

namespace Ui {
    class MainWindow;
}

struct GState {
    int wolfN;
    int goatN;
    std::string goatSt;
};

using sendClb = void(*)(int num);
using getClb = bool(*)(GState* st);
using tClb = int(*)(void);
using isSmthClb = bool(*)(void);

class GUI : public QMainWindow {
Q_OBJECT
public:
    GUI(sendClb SendFunc, getClb GetFunc, tClb TimeFunc, isSmthClb IsTerminated,
            isSmthClb IsConnected, isSmthClb IsNeedToSend);

    void SetTitle(const char *windowName);

    ~GUI();

private slots:
    void sendNum();
    void regAction();

private:
    Ui::MainWindow *_ui;

    QTimer *_timer;
    QStandardItemModel *_model;

    sendClb _send;
    getClb _get;
    tClb _time;
    isSmthClb _isTerminated;
    isSmthClb _isConnected;
    isSmthClb _isNeedToSend;

    GUI() = delete;
};

#endif //!__GUI_H_