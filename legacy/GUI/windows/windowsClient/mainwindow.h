#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <winsock2.h>
#include <ws2tcpip.h>

#include <QMainWindow>
#include <QTimer>
#include <QListWidgetItem>
#include <QThread>

#include <iostream>
#include <vector>
#include <sstream>

#include "matchstatus.h"
#include<structServClient.hpp>

#include <xinput.h>
#include <stdlib.h>
#include <stdio.h>


#define WIN32_LEAN_AND_MEAN

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "8888"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    struct player playerStack[4];
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    MatchStatus *status;
    QTimer *joyUpdateTimer;
    QTimer *joyUiTimer;
    QTimer *progUpdateTimer;
    QThread workerThread;
    QThread runAllTheThreads[255];
    void debug(const char* input);
    void debug(QString input);

    int currIp;
private slots:
    void on_clientList_itemClicked(QListWidgetItem *item);
    void on_Scan4robot_clicked();
    void on_connect2robot_clicked();
    void handleFastResIP(QString res);
    void updateJoyVals();
    void updateJoyGUI();
    void updateProgBar();
    void on_p1_conCont_clicked();
    void on_p2_conCont_clicked();

    void on_p3_conCont_clicked();

    void on_p4_conCont_clicked();

    void on_p1_disCon_clicked();

    void on_p2_disCon_clicked();

    void on_p3_disCon_clicked();

    void on_p4_disCon_clicked();

    void on_p1_chng_stat_clicked();

    void on_p2_chng_stat_clicked();

    void on_p3_chng_stat_clicked();

    void on_p4_chng_stat_clicked();

    void on_p4_chng_team_clicked();

    void on_p2_chng_team_clicked();

    void on_p3_chng_team_clicked();

    void on_p1_chng_team_clicked();

    void on_readyMatch_clicked();

    void on_startMatch_clicked();
    void updateGameTimer();
    void updateState();
    void matchIsOver();
private:
    Ui::MainWindow *ui;
    void clearPlayerCont(struct player *play);
    bool bindJoystick(int playerNum);
    void disConRobot(int playerNum);
    void toggleRobotReady(void);
    void toggleRobotTeam(void);

signals:
      void fastScanNet();
};


class highSpeedNetMap : public QObject
{
     Q_OBJECT
public:
    int targetIp;
    QString result;
public slots:
    void fastScanIps();
signals:
     void fastResultReady(QString);
};


#endif // MAINWINDOW_H

