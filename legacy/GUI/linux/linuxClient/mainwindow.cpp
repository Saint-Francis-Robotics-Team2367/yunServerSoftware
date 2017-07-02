

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "joystick.hpp"
#include "clientNetworkCom.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->timer1 = new QTimer(this);
    this->timer2 = new QTimer(this);
    //initialize player stack
    for(int i =0; i<4;i++)
    {
        this->playerStack[i].isConnected=false;
        this->playerStack[i].joy_fd = UNNASSIGNED;
        this->playerStack[i].nextPlayer =NULL;
        this->playerStack[i].socket = INVALID_SOCKET;
        this->playerStack[i].isReady = false;

        if(i<3)
            this->playerStack[i].nextPlayer=this->playerStack +i;
        else
            this->playerStack[i].nextPlayer= NULL;
    }


    ui->setupUi(this);
    clearPlayerCont(&(this->playerStack[0]));
    connect(timer1, SIGNAL(timeout()), this, SLOT(updateJoyVals()));
    connect(timer2, SIGNAL(timeout()),this, SLOT(updateJoyGUI()));

    timer1->start(3);
    timer2->start(100);

    //setup Player 1
}


MainWindow::~MainWindow()
{
    delete ui;
}

void netMap::scanIps(const QString &parameter)
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    hints;
    char name[255],*ip;
    IN_ADDR subNet;
    PHOSTENT hostinfo;
    int iResult,ipPos;
    static QString *threadRes = new QString();
    *threadRes = LOCAL_HOST;

    ipPos = parameter.toInt();
    //startup and get local IP
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
    }
    else
    {
        if( gethostname ( name, sizeof(name)) == 0)
        {
            if((hostinfo = gethostbyname(name)) != NULL)
            {
               int nCount = 0;
               while(hostinfo->h_addr_list[nCount])
               {
                    ip = inet_ntoa(*(struct in_addr *)hostinfo->h_addr_list[nCount]);;
                    nCount++;
               }
            }
        }
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;


    subNet.S_un.S_addr = inet_addr(ip); //convert to data struct
    subNet.S_un.S_un_b.s_b4=ipPos; //change last bit of ip

    iResult = getaddrinfo(inet_ntoa(subNet), DEFAULT_PORT, &hints, &result);
    if ( iResult == 0 )
    {
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(result->ai_family, result->ai_socktype,
            result->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %d\n", WSAGetLastError());
            WSACleanup();
        }

        iResult = ::connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
        if (iResult != SOCKET_ERROR)
        {

            *threadRes = QString::fromUtf8(inet_ntoa(subNet));
        }
        closesocket(ConnectSocket); //reset socket for next iteration
        ConnectSocket = INVALID_SOCKET;
        freeaddrinfo(result);
    }
    emit resultReady(*threadRes);
}





void MainWindow::deletePlayer(struct player *stack, struct player *del)
{
   while(stack != del && stack->nextPlayer)
       stack = stack->nextPlayer;
   if(stack->nextPlayer == del)
   {
       stack->nextPlayer = del->nextPlayer;
       free(del);
   }
}

void MainWindow::on_clientList_itemClicked(QListWidgetItem *item)
{
    ui->ipBox->setText(item->text());
}

void MainWindow::on_Scan4robot_clicked()
{
    if(!workerThread.isRunning())
    {
        debug("searching network for clients");

        //start search on new thread
        netMap *scanThread = new netMap;
        scanThread->moveToThread(&workerThread);
        connect(scanThread, SIGNAL(finished()), scanThread, SLOT(deleteLater()));
        connect(this, SIGNAL(scanNet(QString)), scanThread, SLOT(scanIps(QString)));
        connect(scanThread, SIGNAL(resultReady(QString)), this ,SLOT(handleResIP(QString)));
        this->currIp = 0;  //start search from ip 0
        workerThread.start();
        emit this->scanNet(QString::number(this->currIp));

        ui->Scan4robot->setText("Cancel");//change search button to cancel button
    }
    else
    {
        this->debug("cancelled search, current thread will finish execution before being stoped");
        ui->Scan4robot->setText("Scan for Robots"); //change button back to search
        workerThread.exit(); //cancel thread

    }
}

void MainWindow::deletePlayer(struct player *stack, int playerNum)
{
    struct player *mover = stack;
    while(mover->playerNum != playerNum && mover->nextPlayer)
    {
        stack = mover;
        mover = mover->nextPlayer;
    }
    if(mover->playerNum == playerNum)
    {
        stack->nextPlayer = mover->nextPlayer;
        free(mover);
    }
}


void MainWindow::on_connect2robot_clicked()
{
    struct player *currPlayer = &(this->playerStack[ui->playerTabs->currentIndex()]);
    WSADATA wsaData;
    struct addrinfo hints;
    int iResult;
    int ipAlreadyTaken = -1;

    for(int i=0; i<4 && ipAlreadyTaken == -1;i++)
    {
        if (ui->ipBox->text() == this->playerStack[i].address)
            ipAlreadyTaken =i;
    }
    if(currPlayer->socket == INVALID_SOCKET && ipAlreadyTaken == -1)
    {
        //startup
        iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

        ZeroMemory( &hints, sizeof(hints) );
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        iResult = getaddrinfo(ui->ipBox->text().toLocal8Bit(), DEFAULT_PORT, &hints, &(currPlayer->add_inf));
        if ( iResult == 0)
        {
            // Create a SOCKET for connecting to server
            currPlayer->socket = socket(currPlayer->add_inf->ai_family, currPlayer->add_inf->ai_socktype,currPlayer->add_inf->ai_protocol);
            if (currPlayer->socket == INVALID_SOCKET) {
                debug("socket failed with error: " + QString::number(WSAGetLastError()));
                WSACleanup();
            }

            iResult = ::connect(currPlayer->socket, currPlayer->add_inf->ai_addr, (int)currPlayer->add_inf->ai_addrlen);
            if (iResult == SOCKET_ERROR)
            {
                debug("Connection failed to ip: " + ui->ipBox->text()+  " with error: " + QString::number(WSAGetLastError()));
                currPlayer->socket = INVALID_SOCKET;
                closesocket(currPlayer->socket);
            }
            else
            {
                currPlayer->isConnected =true;
                currPlayer->address = ui->ipBox->text().toLocal8Bit();
                QList<QLabel *> visableLabel =ui->playerTabs->currentWidget()->findChildren<QLabel*>(); //create list of visable label widgets
                for(int i=0; i< visableLabel.length(); i++)
                {
                    if(visableLabel[i]->objectName().endsWith("val_ipAdd"))
                    {
                        visableLabel[i]->setText(ui->ipBox->text());
                    }
                    else if(visableLabel[i]->objectName().endsWith("val_robStat"))
                    {
                        visableLabel[i]->setText("Not Ready");
                    }
                    else if(visableLabel[i]->objectName().endsWith("val_robMod"))
                    {
                        visableLabel[i]->setText("Idle");
                    }
                    else if(visableLabel[i]->objectName().endsWith("val_teamCol"))
                    {
                        visableLabel[i]->setText("Red");
                        currPlayer->teamColor=RED;
                    }
                }
            }
            //NEED TO HANDLE CONNECTION ERRORS HERE!
        }
        else
        {
            freeaddrinfo(currPlayer->add_inf);
            debug("could not get address info, failed with error code: " + QString::number(WSAGetLastError()));
        }
    }
    else
    {
        if(ipAlreadyTaken == -1)
            debug("player's socket is alread linked to robot at ip: " + currPlayer->address);
        else
            debug("IP address already linked to player " + QString::number(ipAlreadyTaken+1));
    }
}

void MainWindow::handleResIP(const QString &res)
{
    this->debug("Scanned IP " + QString::number(this->currIp));
    ui->scanProg->setValue(this->currIp);
    if(res!=LOCAL_HOST)
        ui->clientList->addItem(res);
    this->currIp++;
    if(currIp<=255)emit this->scanNet(QString::number(this->currIp));
}
void MainWindow::debug(const char* input)
{
    ui->debugList->addItem(input);
}
void MainWindow::debug(QString input)
{
    ui->debugList->addItem(input);
}
void MainWindow::updateJoyVals()
{
    for(int i=0; i<4;i++)
    {
        if(this->playerStack[i].isReady)
        {
            int err = XInputGetState(this->playerStack[i].joy_fd, &(this->playerStack[i].controller));
            if(err == ERROR_DEVICE_NOT_CONNECTED)
            {
                clearPlayerCont(&(this->playerStack[i]));
            }
        }
    }
}
void MainWindow::clearPlayerCont(struct player *play)
{
   debug("Controller DC'ED");
   play->isReady=false;
   play->joy_fd = -1;
}

void MainWindow::updateJoyGUI()
{

        QList<QLCDNumber *> visableLCD =ui->playerTabs->currentWidget()->findChildren<QLCDNumber *>(); //create list of visable LCD widgets
        QList<QLabel *> visableLabel =ui->playerTabs->currentWidget()->findChildren<QLabel*>(); //create list of visable label widgets
        for(int i =0; i<visableLCD.length(); i++) //update LCD data
        {
            if(visableLCD[i]->objectName().endsWith("val_leftX"))
            {
                visableLCD[i]->display(this->playerStack[ui->playerTabs->currentIndex()].controller.Gamepad.sThumbLX);
            }
            else if(visableLCD[i]->objectName().endsWith("val_leftY"))
            {
                visableLCD[i]->display(this->playerStack[ui->playerTabs->currentIndex()].controller.Gamepad.sThumbLY);
            }
            else if(visableLCD[i]->objectName().endsWith("val_rightX"))
            {
                visableLCD[i]->display(this->playerStack[ui->playerTabs->currentIndex()].controller.Gamepad.sThumbRX);
            }
            else if(visableLCD[i]->objectName().endsWith("val_rightY"))
            {
                visableLCD[i]->display(this->playerStack[ui->playerTabs->currentIndex()].controller.Gamepad.sThumbRY);
            }
            else
            {
                visableLCD[i]->display(0);
            }
        }
        for(int i=0; i<visableLabel.length(); i++) //display buttons pressed in 16bit binary
        {
            if(visableLabel[i]->objectName().endsWith("val_but"))
            {
                QString output = "0000000000000000";
                int buttons = this->playerStack[ui->playerTabs->currentIndex()].controller.Gamepad.wButtons;
                for(int j=0; j < 16; j++)
                {
                    if(buttons & 1<<j)
                        output[j] = '1';
                    else
                        output[j] = '0';
                }
                visableLabel[i]->setText(output);
            }
        }
}

bool MainWindow::bindJoystick(int playerNum)
{
    bool retVal = false;
    QList<QPushButton *> visablePButtons =ui->playerTabs->currentWidget()->findChildren<QPushButton *>();
    bool contConn[16];
    bool contFound = false;
    memset(contConn,0, sizeof(contConn));
    QList<QComboBox *> visableCombo =ui->playerTabs->currentWidget()->findChildren<QComboBox*>();
    QComboBox *currSel;

    for(int i=0; i<visableCombo.length();i++)  //find comboBox then save the user selected number
    {
        if(visableCombo[i]->objectName().endsWith("contSel"))
            currSel = visableCombo[i];
    }

    if(playerStack[playerNum].joy_fd == UNNASSIGNED) //check if player is already assigned a controller
    {
        if(currSel->currentText() == "any") //check if the user has specified a controller
        {
            for(int i=0; i<4;i++) //check which controllers are already assigned
            {
                if(this->playerStack[i].joy_fd != UNNASSIGNED)
                    contConn[this->playerStack[i].joy_fd] = true;
            }
            for(int i=0; i<16 && !contFound; i++) //look through controllers for a connected controller that isnt assigned
            {
                int error=XInputGetState(i,&(playerStack[playerNum].controller));
                if(error!=ERROR_DEVICE_NOT_CONNECTED && !contConn[i])
                {
                        debug("Successful connected player: " + QString::number(ui->playerTabs->currentIndex()+1)+ " to controller: " + QString::number(i));
                        playerStack[playerNum].joy_fd=i;
                        contFound=true;
                        playerStack[playerNum].isReady=true;
                        visablePButtons[0]->setText("Unlink Controller");
                }
            }
        }
        else //connect to the user selected controller
        {
            int userSelection = currSel->currentIndex()-1;

            for(int i=0; i<4;i++) //check which controllers are already assigned
            {
                if(this->playerStack[i].joy_fd != UNNASSIGNED)
                    contConn[this->playerStack[i].joy_fd] = true;
            }
            int error=XInputGetState(userSelection,&(playerStack[playerNum].controller));
            if(error!=ERROR_DEVICE_NOT_CONNECTED && !contConn[userSelection])
            {
                    debug("Successful connected player " + QString::number(ui->playerTabs->currentIndex()+1)+ " to controller: " + QString::number(userSelection));
                    playerStack[playerNum].joy_fd=userSelection;
                    playerStack[playerNum].isReady=true;
                    visablePButtons[0]->setText("Unlink Controller");
                    retVal = true;
            }
        }
        if(playerStack[playerNum].joy_fd == UNNASSIGNED)
        {
            debug("Could not find controller for player " + QString::number(ui->playerTabs->currentIndex()+1));
        }
    }
    else //already has a controller assigned to them so we are going to unlink it from them
    {
        playerStack[playerNum].joy_fd = UNNASSIGNED;
        playerStack[playerNum].isReady = false;
        visablePButtons[0]->setText("Link Controller");
        debug("Unlinked controller from player " + QString::number(ui->playerTabs->currentIndex()+1));
    }
    return retVal;
}

void MainWindow::on_p1_conCont_clicked()
{
    bindJoystick(ui->playerTabs->currentIndex());
}

void MainWindow::on_p2_conCont_clicked()
{
    bindJoystick(ui->playerTabs->currentIndex());
}

void MainWindow::on_p3_conCont_clicked()
{
     bindJoystick(ui->playerTabs->currentIndex());
}

void MainWindow::on_p4_conCont_clicked()
{
     bindJoystick(ui->playerTabs->currentIndex());
}
