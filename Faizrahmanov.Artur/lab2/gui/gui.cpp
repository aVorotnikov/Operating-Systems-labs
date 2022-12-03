#include "gui.h"
#include "utils/configuration.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <random>
#include <thread>

Gui &Gui::getInstance()
{
    static Gui instance;
    return instance;
}

void Gui::init(isRunningCallback isRunning, stopRunningCallback stopRunning, sendMessageCallback sendMessage)
{
    this->isRunning = isRunning;
    this->stopRunning = stopRunning;
    this->sendMessage = sendMessage;

    QObject::connect(&gameHistory, &GameHistory::addMessage,
                         &gameHistory, &GameHistory::onMessageAdded);
}

int Gui::run()
{
    char *argv[] = {(char *)"gui"};
    int argc = 1;
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl)
        {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.rootContext()->setContextProperty(QStringLiteral("MAX_WOLF_NUMBER"), QVariant::fromValue(Configuration::Wolf::MAX_NUMBER));
    engine.rootContext()->setContextProperty("gui", this);
    engine.rootContext()->setContextProperty("gameHistory", &gameHistory);

    engine.load(url);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]()
            { sendNewWolfMessage(generateWolfNumber()); });
    timer->start(Configuration::ROUND_TIME_IN_MS);

    int val = app.exec();

    stopRunning();

    return val;
}

void Gui::getMessageFromHost(const std::string& msg)
{
    
    emit Gui::getInstance().gameHistory.addMessage(QString::fromStdString(msg));
}

void Gui::sendNewWolfMessage(const unsigned int &newNum)
{
    timer->stop();

    if (isRunning())
    {
        sendMessage(newNum);

        timer->start(Configuration::ROUND_TIME_IN_MS);
    }
    else
    {
        gameHistory.addMessage("Game Over!");
    }
}

unsigned int Gui::generateWolfNumber()
{
    std::random_device seeder;

    std::mt19937 rng(seeder());
    std::uniform_int_distribution<int> gen(Configuration::Wolf::MIN_NUMBER, Configuration::Wolf::MAX_NUMBER);

    return gen(rng);
}

int GameHistory::rowCount(const QModelIndex &p) const
{
    return messages.size();
}

QVariant GameHistory::data(const QModelIndex &index, int role) const
{
    if (role == MESSAGE_ROLE)
    {
        return messages[index.row()];
    }

    return QVariant();
}

QHash<int, QByteArray> GameHistory::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[MESSAGE_ROLE] = "message";
    return roles;
}

void GameHistory::onMessageAdded(const QString&msg)
{
    beginResetModel();
    messages.push_back(msg);
    endResetModel();
}