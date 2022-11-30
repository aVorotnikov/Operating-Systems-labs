#pragma once

#include <QObject>
#include <QTimer>
#include <functional>
#include <vector>
#include <string>
#include <QAbstractListModel>

#include "utils/message.h"

class GameHistory : public QAbstractListModel
{
    Q_OBJECT
private:
    const int MESSAGE_ROLE = Qt::UserRole + 1;

    std::vector<QString> messages;

public:
    GameHistory() = default;
    ~GameHistory() = default;

    int rowCount(const QModelIndex &p) const final;
    QVariant data(const QModelIndex &index, int role) const final;
    QHash<int, QByteArray> roleNames() const final;

public slots:
    void onMessageAdded(const QString& msg);

signals:
    void addMessage(const QString& msg);
};

class Gui : public QObject
{
    Q_OBJECT
private:
    using isRunningCallback = std::function<bool()>;
    using stopRunningCallback = std::function<void()>;
    using sendMessageCallback = std::function<void(unsigned short thrownNum)>;

    isRunningCallback isRunning;
    stopRunningCallback stopRunning;
    sendMessageCallback sendMessage;

    QTimer *timer;

    GameHistory gameHistory;

public:
    static Gui &getInstance();

    void init(isRunningCallback isRunning, stopRunningCallback stopRunning, sendMessageCallback sendMessage);

    int run();

    static void getMessageFromHost(const std::string& msg);

private:
    Gui() = default;
    Gui(Gui &w) = delete;
    Gui &operator=(const Gui &w) = delete;

    unsigned int generateWolfNumber();
public slots:
    void sendNewWolfMessage(const unsigned int &newNum);

signals:
};