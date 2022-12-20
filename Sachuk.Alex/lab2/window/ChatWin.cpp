#include <QWidget>

#include "string.h"
#include "ui_ChatWin.h"
#include "ChatWin.h"

void ChatWin::initTimer() {
    timer = new QTimer(this);

    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateTimer()));
    timer->start(30);
}

ChatWin::ChatWin(std::string name, 
        read_callback_t readCallback, 
        write_callback_t writeCallback, 
        controll_callback_t controllCallback) : 
        QMainWindow((QWidget*)0),
        ui(new Ui::ChatWin),
        windowName(name),
        winReadCallback(readCallback),
        winWriteCallback(writeCallback),
        winControlCallback(controllCallback) 
{
    ui->setupUi(this);
    initTimer();
    setWindowTitle(windowName.c_str());
}


std::string ChatWin::constructMessage() {
  std::string res = windowName + ": " + ui->msgText->text().toLocal8Bit().data();
  return res;
}

void ChatWin::sendMessage() {
    Message msg;
    std::string msgText = constructMessage();
    size_t len = msgText.size() > MAX_CHAR_LENGTH ? MAX_CHAR_LENGTH : msgText.size();
    strncpy(msg.text, msgText.c_str(), len);
    winReadCallback(msg);
}

void ChatWin::updateTimer() {
    if (!winControlCallback())
        close();

    Message msg = {0};
    while (winWriteCallback(&msg))
      ui->msgsHist->addItem(msg.text);
}

ChatWin::~ChatWin() {
    delete ui;
}
