#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <host.h>
#include <client.h>
#include <joystickhandler.h>

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
    Host *host;
    Client *client;
    QTimer *broadcastTimer;
    QTimer *joystickTimer;
    QTimer *robotSendTimer;
    QTimer *robotTimeoutUi;
    QList<JoyStickHandler*> joyList;
public slots:
    void startClient();
    void check4Host();
    void startHost();
    void updateClientList();
    void checkStartMatch();
    void robotComTimeout();
    void displayDbgMsg(QByteArray dgram);
private slots:
    void on_btn_ForceMatchStart_clicked();
    void on_p1_linkCont_clicked();
    void on_p2_linkCont_clicked();
    void on_p3_linkCont_clicked();
    void on_p4_linkCont_clicked();
    void on_p5_linkCont_clicked();
    void on_p6_linkCont_clicked();

    void updateDropdowns();

    void updateJoyVals();
    void on_btn_stopMatch_clicked();
    void on_p2_linkRob_clicked();
    void on_p1_linkRob_clicked();
    void on_p3_linkRob_clicked();
    void on_p4_linkRob_clicked();
    void on_p5_linkRob_clicked();
    void on_p6_linkRob_clicked();


};

#endif // MAINWINDOW_H
