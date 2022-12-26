#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void WriteLog(const std::string& msg);
    int GetNumber();
    void SetPid(pid_t pid);
    ~MainWindow();

signals:
    void on_enterNumButton_clicked();

private:
    Ui::MainWindow *ui;
    const int timeout = 3000;
};
#endif // MAINWINDOW_H
