#pragma once
#include <QWidget>
#include <QTimer>
#include <memory>
#include "utils/goat_info.h"
#include "utils/safe_queue.h"
#include "utils/message.h"

namespace Ui
{
    class Form;
}
class SharedGameState;

class GUIWindow : public QWidget
{
    Q_OBJECT
public:
    virtual ~GUIWindow();
    explicit GUIWindow(std::shared_ptr<SharedGameState> &gameState, SafeQueuePusher<HostMessage> &&hostPusher);
    void pushMessage(const GUIMessage &message);

private:
    std::shared_ptr<SharedGameState> gameState;
    SafeQueuePusher<HostMessage> hostPusher;
    SafeQueue<GUIMessage> fromHostMessageQueue;
    Ui::Form *ui;

    static constexpr uint MESSAGE_CHECK_TIMER_INTERVAL = 100;
    QTimer *messageCheckTimer;

    int wolfNumber;
    static constexpr const uint INPUT_WAIT_TIMER_INTERVAL = 3000;
    static constexpr const uint TIMER_BAR_UPDATE_INTERVAL = 10;
    QTimer *inputWaitTimer;
    QTimer *timerBarUpdateTimer;
    void updateWolfNumberUI();

    void updateGoatsStatusTable();
    void logText(const std::string &text);
    void randomizeWolfNumber();
    void inputStart();
    void setWolfNumberControlsEnabled(bool enabled);
private slots:
    void messageCheck();
    void updateTimerBar();
    void updateWolfNumber(int wolfNumber);
    void start();
    void gameOver();
    void inputWaitTimeout();
    void randomPressed();
    void valueConfirmed();

private:
    static QString goatStateStr(GOAT_STATE state);
    static QString goatEventStr(GOAT_EVENT event);
    void closeEvent(QCloseEvent *event);
};