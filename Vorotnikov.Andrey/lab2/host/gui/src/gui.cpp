#include "../gui.h"
#include "../ui_MainWindow.h"

#include <QList>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QString>
#include <QPalette>
#include <QStringList>

Gui::Gui(
    SendNumberCallback sendNumber, GetGameStateCallback getGameState, TimerCallback timer,
    EventCallback isTerminated, EventCallback isConnected, EventCallback isNeedToSend)
:
    QMainWindow((QWidget*)0), mainWindow_(new Ui::MainWindow),
    sendNumber_(sendNumber), getGameState_(getGameState), timer_(timer),
    isTerminated_(isTerminated), isConnected_(isConnected), isNeedToSend_(isNeedToSend)
{
    mainWindow_->setupUi(this);

    mainWindow_->lineEdit->setValidator(new QIntValidator(1, 100, this));
    QObject::connect(mainWindow_->lineEdit, SIGNAL(returnPressed()), this, SLOT(SendNum()));

    mainWindow_->tableView->setShowGrid(true);
    mainWindow_->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    model_ = new QStandardItemModel(this);
    QStringList tableHeader = QString("Волк, Козленок, Состояние козленка").simplified().split(",");
    model_->setHorizontalHeaderLabels(tableHeader);
    mainWindow_->tableView->setModel(model_);
    mainWindow_->tableView->setColumnWidth(0, mainWindow_->tableView->width() / 5);
    mainWindow_->tableView->setColumnWidth(1, mainWindow_->tableView->width() / 4);
    mainWindow_->tableView->setColumnWidth(2, mainWindow_->tableView->width() / 2);
    mainWindow_->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    mainWindow_->lcdNumber->setSegmentStyle(QLCDNumber::Flat);
    QPalette p = mainWindow_->lcdNumber->palette();
    p.setColor(p.WindowText, Qt::red);
    mainWindow_->lcdNumber->setPalette(p);

    timerItem_ = new QTimer(this);
    QObject::connect(timerItem_, SIGNAL(timeout()), this, SLOT(RegAction()));
    timerItem_->start(100);
}

void Gui::SetTitle(const char *windowName)
{
    setWindowTitle(windowName);
}

void Gui::SendNum()
{
    if (!timer_() || !isNeedToSend_())
        return;

    bool converted;
    int number = mainWindow_->lineEdit->text().toInt(&converted, 10);
    if (converted)
        sendNumber_(number);
    mainWindow_->lineEdit->clear();
}

void Gui::RegAction()
{
    static bool needToClear = false;

    if (isTerminated_())
        this->close();

    if (isConnected_())
    {
        mainWindow_->statusbar->showMessage("Client connected");
        if (needToClear)
        {
            model_->removeRows(0, model_->rowCount());
            needToClear = false;
        }
    }
    else
    {
        mainWindow_->statusbar->showMessage("Client not connected");
        if (!needToClear)
            needToClear = true;
    }

    int t = timer_();
    t = t < 0 ? 0 : t;
    mainWindow_->lcdNumber->display(t);

    GameState state;
    if (getGameState_(state))
    {
        QList<QStandardItem*> l;
        l.append(new QStandardItem(QString("%1").arg(state.wolfNumber)));
        l.append(new QStandardItem(QString("%1").arg(state.goatNumber)));
        l.append(new QStandardItem(QString(state.goatState.c_str())));
        model_->appendRow(l);
    }
}

Gui::~Gui()
{
    delete mainWindow_;
}
