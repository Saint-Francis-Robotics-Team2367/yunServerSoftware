#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    host = new Host();
    this->broadcastTimer = new QTimer();
    this->robotSendTimer = new QTimer();
    this->joystickTimer = new QTimer();
    this->robotTimeoutUi = new QTimer();
    this->robotTimeoutUi->setInterval(1000);
    this->broadcastTimer->setInterval(200);
    this->robotSendTimer->setInterval(20);
    this->joystickTimer->setInterval(10);
    for(int i = 0;i<4;i++)
    {
        this->joyList.append(new JoyStickHandler());
        this->joyList.at(i)->initJoystick(i);

    }
    this->ui->mainTabs->removeTab(this->ui->mainTabs->count()-1);
    this->ui->mainTabs->removeTab(this->ui->mainTabs->count()-1);
    //connect menu bar buttons
    connect(ui->actionStart_as_Host,SIGNAL(triggered()),this,SLOT(check4Host()));
    connect(ui->actionStart_as_Player,SIGNAL(triggered()),this,SLOT(startClient()));


    connect(this->host,SIGNAL(clientAdded()),this,SLOT(updateClientList()));
    connect(this->host, SIGNAL(robotAdded()), this, SLOT(checkStartMatch()));
    connect(this->host,SIGNAL(receivedDbgMsg(QByteArray)),this,SLOT(displayDbgMsg(QByteArray)));

    connect(this->host,SIGNAL(robotAdded()),this,SLOT(updateDropdowns()));
    connect(this->joystickTimer,SIGNAL(timeout()),this,SLOT(updateJoyVals()));
    connect(this->robotSendTimer,SIGNAL(timeout()),this->host,SLOT(sendRobotSync()));

    connect(this->robotTimeoutUi, SIGNAL(timeout()), this, SLOT(robotComTimeout()));
    this->robotTimeoutUi->start();

}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::startClient()
{
    this->client = new Client();

    //disable menu buttons after a selection
    this->ui->actionStart_as_Host->setEnabled(false);
    this->ui->actionStart_as_Player->setEnabled(false);
    connect(this->broadcastTimer,SIGNAL(timeout()),this,SLOT(updateJoyVals()));

}
void MainWindow::check4Host()
{
    //check that a host doesn't already exist
    this->client = new Client();

    //disable menu buttons after a selection
    this->ui->actionStart_as_Host->setEnabled(false);
    this->ui->actionStart_as_Player->setEnabled(false);

    //wait for 5 seconds then check if a host beacon came in
    QTimer::singleShot(5000, this, SLOT(startHost()));
}

void MainWindow::startHost()
{
    if(this->client->isConnected())
    {
        //Host was found!
        delete this->client;

        //re-enable menu buttons
        this->ui->actionStart_as_Host->setEnabled(true);
        this->ui->actionStart_as_Player->setEnabled(true);
    }
    else
    {
        //no other host is available so we will start up as the host
        connect(this->broadcastTimer,SIGNAL(timeout()),host,SLOT(sendBroadcast()));
        this->broadcastTimer->start();
        delete this->client;
    }
}
void MainWindow::updateClientList()
{
    QList<QComboBox *> comboBoxes =ui->mainTabs->findChildren<QComboBox*>(); //create list of combo boxes
    for(int i=0; i<comboBoxes.length(); i++)
    {
        if(comboBoxes[i]->objectName().endsWith("_cb"))
        {
            comboBoxes[i]->clear();
            comboBoxes[i]->addItems(this->host->getClientNames());
        }
    }
}

void MainWindow::checkStartMatch() {
        if (this->host->getRobots()->count() == 4)
            this->ui->btn_startMatch->setEnabled(true);
        else
            this->ui->btn_startMatch->setEnabled(false);
}

void MainWindow::on_btn_ForceMatchStart_clicked()
{
    D_MSG("Game started");
    ui->btn_stopMatch->setEnabled(true);
    this->joystickTimer->start();
    this->robotSendTimer->start();
}

void MainWindow::on_p1_linkCont_clicked()
{
    ui->p1_log->append("Controller " + QString::number(ui->mainTabs->currentIndex()-1) + " linked successfully");
    ui->p1_linkCont->setEnabled(false);
    ui->txt_game_p1_joystick->setStyleSheet("background-color:rgba(10, 255, 10, 0.75);");
    this->host->getMasterList()->at(ui->mainTabs->currentIndex()-1)->setJoyIndex(ui->mainTabs->currentIndex()-1);
}

void MainWindow::on_p2_linkCont_clicked()
{
    ui->p2_log->append("Controller " + QString::number(ui->mainTabs->currentIndex()-1) + " linked successfully");
    ui->p2_linkCont->setEnabled(false);
    ui->txt_game_p2_joystick->setStyleSheet("background-color:rgba(10, 255, 10, 0.75);");
    this->host->getMasterList()->at(ui->mainTabs->currentIndex()-1)->setJoyIndex(ui->mainTabs->currentIndex()-1);
}

void MainWindow::on_p3_linkCont_clicked()
{
    ui->p3_log->append("Controller " + QString::number(ui->mainTabs->currentIndex()-1) + " linked successfully");
    ui->p3_linkCont->setEnabled(false);
    ui->txt_game_p3_joystick->setStyleSheet("background-color:rgba(10, 255, 10, 0.75);");
    this->host->getMasterList()->at(ui->mainTabs->currentIndex()-1)->setJoyIndex(ui->mainTabs->currentIndex()-1);
}

void MainWindow::on_p4_linkCont_clicked()
{
    ui->p4_log->append("Controller " + QString::number(ui->mainTabs->currentIndex()-1) + " linked successfully");
    ui->p4_linkCont->setEnabled(false);
    ui->txt_game_p4_joystick->setStyleSheet("background-color:rgba(10, 255, 10, 0.75);");
    this->host->getMasterList()->at(ui->mainTabs->currentIndex()-1)->setJoyIndex(ui->mainTabs->currentIndex()-1);
}

void MainWindow::updateJoyVals()
{
    for(int i = 0;i<this->host->getMasterList()->size();i++)
    {
        int index = this->host->getMasterList()->at(i)->getJoyIndex();
        if(index!=-1)
        {
            this->joyList.at(index)->updateJoystick();
            JoystickData data;
            data.lX = this->joyList.at(index)->readAxis(0);
            data.lY = this->joyList.at(index)->readAxis(1);
            data.rX = this->joyList.at(index)->readAxis(2);
            data.rY = this->joyList.at(index)->readAxis(3);
            data.lT = this->joyList.at(index)->readAxis(4);
            data.rT = this->joyList.at(index)->readAxis(5);
            data.buttons = this->joyList.at(index)->readBttn(index);
            this->host->getMasterList()->at(i)->setJoystickData(data);
        }
    }
}

void MainWindow::on_btn_stopMatch_clicked()
{

    this->robotSendTimer->stop();
    this->joystickTimer->stop();
    ui->btn_stopMatch->setEnabled(false);
}

void MainWindow::updateDropdowns()
{

    ui->p1_name_cb->addItem(this->host->getRobots()->last().name);
    ui->p2_name_cb->addItem(this->host->getRobots()->last().name);
    ui->p3_name_cb->addItem(this->host->getRobots()->last().name);
    ui->p4_name_cb->addItem(this->host->getRobots()->last().name);
}

void MainWindow::on_p1_linkRob_clicked()
{
    QString name = ui->p1_name_cb->currentText();
    ui->gb_game_player1->setTitle("Player 1: " + name);
    ui->p1_log->append("Connected to " + name);
    ui->txt_game_p1_robcom->setStyleSheet("background-color:rgba(10, 255, 10, 0.75);");
    this->host->getMasterList()->at(ui->mainTabs->currentIndex()-1)->setName(name);

}

void MainWindow::on_p2_linkRob_clicked()
{
    QString name = ui->p2_name_cb->currentText();
    ui->p2_log->append("Connected to " + name);
    ui->gb_game_player2->setTitle("Player 2: " + name);
    ui->txt_game_p2_robcom->setStyleSheet("background-color:rgba(10, 255, 10, 0.75);");
    this->host->getMasterList()->at(ui->mainTabs->currentIndex()-1)->setName(name);
}

void MainWindow::on_p3_linkRob_clicked()
{
    QString name = ui->p3_name_cb->currentText();
    ui->p3_log->append("Connected to " + name);
    ui->gb_game_player3->setTitle("Player 3: " + name);
    ui->txt_game_p3_robcom->setStyleSheet("background-color:rgba(10, 255, 10, 0.75);");
    this->host->getMasterList()->at(ui->mainTabs->currentIndex()-1)->setName(name);
}

void MainWindow::on_p4_linkRob_clicked()
{
    QString name = ui->p4_name_cb->currentText();
    ui->p4_log->append("Connected to " + name);
    ui->gb_game_player4->setTitle("Player 4: " + name);
    ui->txt_game_p4_robcom->setStyleSheet("background-color:rgba(10, 255, 10, 0.75);");
    this->host->getMasterList()->at(ui->mainTabs->currentIndex()-1)->setName(name);
}

void MainWindow::robotComTimeout()
{
    //robot 1
    const int maxTimeout = 6;
    if(this->host->getMasterList()->at(0)->getUpdate().secsTo(QTime::currentTime())> maxTimeout)
    {
        ui->txt_game_p1_robcom->setStyleSheet("background-color:rgba(255, 10, 10, 0.75);");
    }
    else
    {
        ui->txt_game_p1_robcom->setStyleSheet("background-color:rgba(10, 255, 10, 0.75);");
    }
    //robot 2
    if(this->host->getMasterList()->at(1)->getUpdate().secsTo(QTime::currentTime())> maxTimeout)
    {
        ui->txt_game_p2_robcom->setStyleSheet("background-color:rgba(255, 10, 10, 0.75);");
    }
    else
    {
        ui->txt_game_p2_robcom->setStyleSheet("background-color:rgba(10, 255, 10, 0.75);");
    }
    //robot 3
    if(this->host->getMasterList()->at(2)->getUpdate().secsTo(QTime::currentTime())> maxTimeout)
    {
        ui->txt_game_p3_robcom->setStyleSheet("background-color:rgba(255, 10, 10, 0.75);");
    }
    else
    {
        ui->txt_game_p3_robcom->setStyleSheet("background-color:rgba(10, 255, 10, 0.75);");
    }
    //robot 4
    if(this->host->getMasterList()->at(3)->getUpdate().secsTo(QTime::currentTime())> maxTimeout)
    {
        ui->txt_game_p4_robcom->setStyleSheet("background-color:rgba(255, 10, 10, 0.75);");
    }
    else
    {
        ui->txt_game_p4_robcom->setStyleSheet("background-color:rgba(10, 255, 10, 0.75);");
    }
}
void MainWindow::displayDbgMsg(QByteArray dgram)
{
    QString msg(dgram);
    QString robName = msg.section(":",1,1);
    int player = -1;
    for(int i = 0;i<this->host->getMasterList()->size();i++)
    {
        if(robName==this->host->getMasterList()->at(i)->getName())
        {
            player = i;
        }
    }
    if(player!=-1)
    {
        switch(player)
        {
            case 0: ui->p1_log->append(msg);
            case 1: ui->p2_log->append(msg);
            case 2: ui->p3_log->append(msg);
            case 3: ui->p4_log->append(msg);
        }
    }
}

void MainWindow::on_p5_linkRob_clicked()
{

}

void MainWindow::on_p5_linkCont_clicked()
{

}

void MainWindow::on_p6_linkRob_clicked()
{

}

void MainWindow::on_p6_linkCont_clicked()
{

}
