#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <broadcast.h>
#include <client.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Broadcast *udpBroadcast;
    Client *client;
    QTimer *timer;
public slots:
    void startClient();
};

#endif // MAINWINDOW_H