#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <winsock2.h>
#include <ws2tcpip.h>

#include <QMainWindow>
#include <QTimer>
#include <QListWidgetItem>
#include <QThread>

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
    struct player playerStack;
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QTimer *timer1;
    QTimer *timer2;
    QThread workerThread;
    void debug(const char* input);
    void debug(QString input);

    int currIp;
private slots:
    void on_clientList_itemClicked(QListWidgetItem *item);
    void on_Scan4robot_clicked();
    void on_connect2robot_clicked();
    void handleResIP(const QString &res);
    void on_cancelButton_clicked();
    void updateJoyVals();
    void updateJoyGUI();
    void on_p1_conCont_clicked();

private:



    Ui::MainWindow *ui;
    void pushPlayerStack(struct player *stack);
    void popPlayerStack(struct player *stack);
    void deletePlayer(struct player *stack, struct player *del);
    void deletePlayer(struct player *stack, int playerNum);
    void clearPlayerCont(struct player *play);

signals:
      void scanNet(const QString &);
};
class netMap : public QObject
{
    Q_OBJECT
    QThread workerThread;
public slots:
    void scanIps(const QString &parameter);

signals:
    void resultReady(const QString &result);
};

#endif // MAINWINDOW_H
