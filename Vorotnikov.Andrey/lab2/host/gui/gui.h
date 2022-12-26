#pragma once

#include <string>
#include <QMainWindow>
#include <QtGui>
#include <QTableView>
#include <QTimer>
#include <QApplication>

namespace Ui
{

class MainWindow;

}

struct GameState
{
    int wolfNumber;
    int goatNumber;
    std::string goatState;
};

using SendNumberCallback = std::function<void (int num)>;
using GetGameStateCallback = std::function<bool (GameState& st)>;
using TimerCallback = std::function<int (void)>;
using EventCallback = std::function<bool (void)>;

class Gui : public QMainWindow
{
Q_OBJECT
public:
    Gui(
        SendNumberCallback sendNumber, GetGameStateCallback getGameState, TimerCallback timer,
        EventCallback isTerminated, EventCallback isConnected, EventCallback isNeedToSend);

    void SetTitle(const char *windowName);

    ~Gui();

private slots:
    void SendNum();
    void RegAction();

private:
    Ui::MainWindow* mainWindow_;
    QTimer* timerItem_;
    QStandardItemModel* model_;

    const SendNumberCallback sendNumber_;
    const GetGameStateCallback getGameState_;
    const TimerCallback timer_;
    const EventCallback isTerminated_;
    const EventCallback isConnected_;
    const EventCallback isNeedToSend_;

    Gui() = delete;
};
