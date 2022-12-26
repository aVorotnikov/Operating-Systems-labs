#include "gui_window.h"
#include "ui_gui_form.h"
#include "host/shared_game_state.h"
#include <QTime>
#include <string>

GUIWindow::GUIWindow(std::shared_ptr<SharedGameState> &gameState, SafeQueuePusher<HostMessage> &&hostPusher) : QWidget(), gameState(gameState), hostPusher(hostPusher)
{
    ui = new Ui::Form();
    messageCheckTimer = new QTimer(this);
    inputWaitTimer = new QTimer(this);
    timerBarUpdateTimer = new QTimer(this);
    ui->setupUi(this);
    connect(ui->valueSelectSlider, SIGNAL(valueChanged(int)), this, SLOT(updateWolfNumber(int)));
    connect(ui->valueSelectSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateWolfNumber(int)));
    connect(ui->startStopPushButton, SIGNAL(clicked()), this, SLOT(start()));
    connect(ui->valueConfirmPushButton, SIGNAL(clicked()), this, SLOT(valueConfirmed()));
    connect(ui->valueConfirmRandomPushButton, SIGNAL(clicked()), this, SLOT(randomPressed()));
    setWolfNumberControlsEnabled(false);
    srand(QTime::currentTime().msec());

    inputWaitTimer->setSingleShot(true);
    inputWaitTimer->setInterval(INPUT_WAIT_TIMER_INTERVAL);
    connect(inputWaitTimer, SIGNAL(timeout()), this, SLOT(inputWaitTimeout()));

    ui->timeRemainingProgressBar->setMaximum(INPUT_WAIT_TIMER_INTERVAL);
    timerBarUpdateTimer->setInterval(TIMER_BAR_UPDATE_INTERVAL);
    connect(timerBarUpdateTimer, SIGNAL(timeout()), this, SLOT(updateTimerBar()));

    messageCheckTimer->setInterval(MESSAGE_CHECK_TIMER_INTERVAL);
    connect(messageCheckTimer, SIGNAL(timeout()), this, SLOT(messageCheck()));

    randomizeWolfNumber();

    messageCheckTimer->start();
    pushMessage(GUIMessage(GUIMessage::GMT_UPDATE_TABLE));
}

GUIWindow::~GUIWindow()
{
    delete ui;
}

void GUIWindow::pushMessage(const GUIMessage &message)
{
    fromHostMessageQueue.push(message);
}

void GUIWindow::messageCheck()
{
    while (fromHostMessageQueue.size() != 0)
    {
        GUIMessage message = fromHostMessageQueue.front();
        fromHostMessageQueue.pop();
        switch (message.messageType)
        {
        case GUIMessage::GMT_END:
            hostPusher.push(HostMessage(HostMessage::HMT_END));
            close();
            return;
        case GUIMessage::GMT_UPDATE_TABLE:
            fromHostMessageQueue.pop_repeat(GUIMessage::GMT_UPDATE_TABLE);
            updateGoatsStatusTable();
            break;
        case GUIMessage::GMT_NEW_ROUND:
            inputStart();
            break;
        case GUIMessage::GMT_GAME_OVER:
            gameOver();
            break;
        case GUIMessage::GMT_SHOW_LOG:
            logText(message.logText);
            break;
        default:
            // TODO
            break;
        }
    }
}

void GUIWindow::updateTimerBar()
{
    int inputWaitRemaining = inputWaitTimer->remainingTime();
    ui->timeRemainingProgressBar->setValue(inputWaitRemaining);
    ui->timeRemainingProgressBar->setFormat(QString::fromStdString(std::to_string(float(inputWaitRemaining) / 1000).substr(0, 4)));
}

void GUIWindow::randomPressed()
{
    randomizeWolfNumber();
}

void GUIWindow::valueConfirmed()
{
    timerBarUpdateTimer->stop();
    inputWaitTimer->stop();
    setWolfNumberControlsEnabled(false);
    gameState->wolfNumber = wolfNumber;
    hostPusher.push(HostMessage(HostMessage::HMT_GUI_INPUT_END));
}

void GUIWindow::inputWaitTimeout()
{
    randomizeWolfNumber();
    valueConfirmed();
}

void GUIWindow::updateWolfNumberUI()
{
    ui->valueSelectSpinBox->setValue(wolfNumber);
    ui->valueSelectSlider->setValue(wolfNumber);
}

void GUIWindow::updateWolfNumber(int wolfNumber)
{
    this->wolfNumber = wolfNumber;
    updateWolfNumberUI();
}

QString GUIWindow::goatStateStr(GOAT_STATE state)
{
    switch (state)
    {
    case GOAT_STATE::ALIVE:
        return QString("Жив");
    case GOAT_STATE::DEAD:
        return QString("Мёртв");
    default:
        return QString("Error");
    }
}

QString GUIWindow::goatEventStr(GOAT_EVENT event)
{
    switch (event)
    {
    case GOAT_EVENT::NONE:
        return QString("NONE");
    case GOAT_EVENT::DIED:
        return QString("Умер");
    case GOAT_EVENT::HIDEN:
        return QString("Спрятался");
    case GOAT_EVENT::REVIVED:
        return QString("Ожил");
    case GOAT_EVENT::THROW_REQUESTED:
        return QString("Ждем бросок");
    case GOAT_EVENT::THROW_RECEIVED:
        return QString("Бросил");
    default:
        return QString("Error");
    }
}

void GUIWindow::updateGoatsStatusTable()
{
    auto goatTable = gameState->getGoatTable();
    while (ui->goatsStatusTableWidget->rowCount() > 0)
    {
        ui->goatsStatusTableWidget->removeRow(0);
    }
    for (decltype(goatTable.size()) i = 0; i < goatTable.size(); i++)
    {
        ui->goatsStatusTableWidget->insertRow(i);
        ui->goatsStatusTableWidget->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(std::to_string(goatTable[i].id))));
        ui->goatsStatusTableWidget->setItem(i, 1, new QTableWidgetItem(goatStateStr(goatTable[i].state)));
        ui->goatsStatusTableWidget->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(std::to_string(goatTable[i].thrownNumber))));
        ui->goatsStatusTableWidget->setItem(i, 3, new QTableWidgetItem(goatEventStr(goatTable[i].lastEvent)));
    }
}

void GUIWindow::logText(const std::string &text)
{
    ui->plainTextEdit->appendPlainText(QString::fromStdString(text));
}

int getRandomNumber()
{
    return rand() % 100 + 1;
}

void GUIWindow::randomizeWolfNumber()
{
    wolfNumber = getRandomNumber();
    updateWolfNumberUI();
}

void GUIWindow::inputStart()
{
    timerBarUpdateTimer->start();
    inputWaitTimer->start();
    setWolfNumberControlsEnabled(true);
}

void GUIWindow::setWolfNumberControlsEnabled(bool enabled)
{
    ui->valueConfirmPushButton->setEnabled(enabled);
    ui->valueConfirmRandomPushButton->setEnabled(enabled);
    ui->valueSelectSlider->setEnabled(enabled);
    ui->valueSelectSpinBox->setEnabled(enabled);
}

void GUIWindow::start()
{
    ui->startStopPushButton->setEnabled(false);
    hostPusher.push(HostMessage(HostMessage::HMT_START_GAME));
}

void GUIWindow::gameOver()
{
    timerBarUpdateTimer->stop();
    inputWaitTimer->stop();
    setWolfNumberControlsEnabled(false);
    ui->startStopPushButton->setEnabled(true);
}

void GUIWindow::closeEvent(QCloseEvent *event)
{
    hostPusher.push(HostMessage(HostMessage::HMT_END));
    event->accept();
}
