#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
     ui->setupUi(this);

    this->joyUpdateTimer = new QTimer(this);
    this->joyUiTimer = new QTimer(this);
    this->progUpdateTimer = new QTimer(this);
    this->status = new MatchStatus();
    //initialize player stack
    for(int i =0; i<4;i++)
    {
        clearPlayerCont(&(this->playerStack[i])); //make sure player doesn't have a controller linked to them
        this->playerStack[i].isConnected=false;
        this->playerStack[i].joy_fd = UNNASSIGNED;
        this->playerStack[i].socket = INVALID_SOCKET;
        this->playerStack[i].isReady = false;

        //special case for last member of stack
        if(i<3)
            this->playerStack[i].nextPlayer=this->playerStack +i;
        else
            this->playerStack[i].nextPlayer= NULL;
    }


    //connect function timers and start them
    connect (status->matchTimer, SIGNAL(timeout()), this, SLOT(updateGameTimer()));
    connect (status, SIGNAL(stateChanged()), this, SLOT(updateState()));
    connect (status, SIGNAL(matchOver()), this,SLOT(matchIsOver()));
    connect(joyUpdateTimer, SIGNAL(timeout()), this, SLOT(updateJoyVals()));  //update joystick values for all players with a connected joystick
    connect(joyUiTimer, SIGNAL(timeout()),this, SLOT(updateJoyGUI()));    //update gui if joystick is connected
    connect(progUpdateTimer, SIGNAL(timeout()), this, SLOT(updateProgBar()));   //reset the progress bar to 0 when robot search is done
    joyUpdateTimer->start(20);
    joyUiTimer->start(100);
    ui->curr_match_val->setText("NOT READY");
    ui->t_left_val->setPalette(Qt::red);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::updateProgBar()
{
    bool searching = false;
    //check if any of the threads are still running
    for(int i=0; i<255 && !searching;i++)
    {
        searching = this->runAllTheThreads[i].isRunning();
    }

    //reset UI if search is done
    if(!searching)
    {
        ui->Scan4robot->setEnabled(true);
        ui->Scan4robot->setText("Scan for Robots");
        ui->scanProg->setValue(ui->scanProg->minimum());
        this->progUpdateTimer->stop();
    }
}

void highSpeedNetMap::fastScanIps()
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    hints;
    char name[255],*ip;
    IN_ADDR subNet;
    PHOSTENT hostinfo;
    int iResult;


    //startup and get local IP
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
    }
    else
    {
        if( gethostname ( name, sizeof(name)) == 0) //get local hostname
        {
            if((hostinfo = gethostbyname(name)) != NULL) //get IP by hostname
            {
               int nCount = 0;
               while(hostinfo->h_addr_list[nCount]) // if hostname is associated with more than 1 IP then select the last one
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


    //change last bit of IP to match input paramater and load output var
    subNet.S_un.S_addr = inet_addr(ip);
    subNet.S_un.S_un_b.s_b4=this->targetIp;

    //try to connect to IP
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
        if (iResult == SOCKET_ERROR)
        {
            subNet.S_un.S_un_b.s_b1 = 0; //set leading bit to 0 so the result handler can tell this ip is invalid
            iResult = WSAGetLastError();
            if(iResult == WSAEADDRINUSE)
                qDebug("addr already in use");
            else if(iResult ==WSAEINTR)
                qDebug("The blocking Windows Socket 1.1 call was canceled through WSACancelBlockingCall.");
            else if(iResult ==WSAEINPROGRESS)
                qDebug("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.");
            else if(iResult == WSAEHOSTUNREACH)
                qDebug("A socket operation was attempted to an unreachable host.");
            else if(iResult == WSAETIMEDOUT)
                qDebug("An attempt to connect timed out without establishing a connection.");
            else if(iResult == WSAECONNREFUSED)
                qDebug("The attempt to connect was forcefully rejected.");
            else
                qDebug(QString::number(iResult).toLocal8Bit());
        }
        closesocket(ConnectSocket); //reset socket for next iteration
        ConnectSocket = INVALID_SOCKET;
        freeaddrinfo(result);
    }

    this->result = inet_ntoa(subNet);
    emit fastResultReady(this->result);
}

void MainWindow::on_clientList_itemClicked(QListWidgetItem *item)
{
    ui->ipBox->setText(item->text());
}

void MainWindow::on_Scan4robot_clicked()
{
    if(ui->Scan4robot->text() != "Scanning") //check that a scan is not already running
    {
        debug("searching network for clients");
        highSpeedNetMap *threadStack[255];

        //clear the old results
        while(ui->clientList->count())
            ui->clientList->takeItem(0);

        //create the threads and link their signals to the main thread, then start them
        for(int i =0; i<255; i++)
        {
            threadStack[i] = new highSpeedNetMap;
            threadStack[i]->targetIp=i;
            threadStack[i]->moveToThread(&(runAllTheThreads[i]));
            connect(&(runAllTheThreads[i]), SIGNAL(finished()), threadStack[i], SLOT(deleteLater()));
            connect(&(runAllTheThreads[i]), SIGNAL(started()),threadStack[i] , SLOT(fastScanIps()));
            connect(threadStack[i], SIGNAL(fastResultReady(QString)), this ,SLOT(handleFastResIP(QString)));
            connect(threadStack[i], SIGNAL(fastResultReady(QString)),&(runAllTheThreads[i]), SLOT(quit()));
            runAllTheThreads[i].start();
        }

        ui->Scan4robot->setText("Scanning");//change search button to let the user know the scan has started
        ui->Scan4robot->setEnabled(false); //prevent people from pressing the button again until the search is done. Re-enable in another function
        this->progUpdateTimer->start(250); //start periodic check to see if the scan is done
    }
}

void MainWindow::on_connect2robot_clicked()  //THIS MESS SHOULD BE MOVED TO IT'S OWN THREAD
{
    if(ui->playerTabs->currentIndex()<4) // check that the user isn't on the debug tab
        {
        struct player *currPlayer = &(this->playerStack[ui->playerTabs->currentIndex()]);
        WSADATA wsaData;
        struct addrinfo hints;
        int iResult;
        int ipAlreadyTaken = -1;

        //check that the IP the user is trying to connect to isn't already asigned to another player
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
                else
                {
                    //set socket recv and snd timeout to 200ms
                    DWORD dwTime = 200;
                    setsockopt(currPlayer->socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&dwTime, sizeof(dwTime));
                    setsockopt(currPlayer->socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&dwTime, sizeof(dwTime));
                }

                //try to connect
                iResult = ::connect(currPlayer->socket, currPlayer->add_inf->ai_addr, (int)currPlayer->add_inf->ai_addrlen);
                if (iResult == SOCKET_ERROR)
                {
                    debug("Connection failed to ip: " + ui->ipBox->text()+  " with error: " + QString::number(WSAGetLastError()));
                    currPlayer->socket = INVALID_SOCKET;
                    closesocket(currPlayer->socket);
                }
                else
                {
                    //update player info and UI
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

        if(currPlayer->isConnected)
        {
            //check if valid connection to robot was created
            char recvBuff[255];
            int attempts = 0;
            char *stringLoc;

            //attempt to interrogate robot for the passphrase until the passphrase is returned or the max number of retries has been reched
            while( !(stringLoc = strstr(recvBuff, INTERROGATION_RESPONSE)) && attempts++ <MAX_INTERROGATION_RETRIES)
            {
                send(currPlayer->socket, CLIENT_INTERROGATION_PHRASE, sizeof(CLIENT_INTERROGATION_PHRASE), 0);
                recv(currPlayer->socket,recvBuff, 255, 0);
            }

            //if the robot did not pass the interrogation then disconnect it
            if(!stringLoc)
            {
                debug("Robot at IP: " + ui->ipBox->text() + " did not respond to interrogation with correct passphrase");
                disConRobot(ui->playerTabs->currentIndex());
            }
        }
    }
    else
    {
        debug("You need to select the player tab which you wish to connect this IP to");
    }
}

void MainWindow::handleFastResIP(QString res)
{
    IN_ADDR resNfo;
    resNfo.S_un.S_addr = inet_addr(res.toLocal8Bit());
    if(resNfo.S_un.S_un_b.s_b1)
    {
        //update UI with data from thread
        QString output = "located Robot at: ";
        this->debug(output + inet_ntoa(resNfo));
        ui->clientList->addItem(inet_ntoa(resNfo));
        ui->clientList->sortItems();
    }
    ui->scanProg->setValue(ui->scanProg->value()+1);
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
    for(int i=0; i<4;i++) //for each player
    {
        if(this->playerStack[i].joy_fd != UNNASSIGNED)//if there is a joystick connected
        {
            //update joystick state
            int err = XInputGetState(this->playerStack[i].joy_fd, &(this->playerStack[i].controller));
            if(err == ERROR_DEVICE_NOT_CONNECTED)//if no joystick connected
            {
                clearPlayerCont(&(this->playerStack[i]));//disconnect joystick
            }

            //if player's socket is connected to a robot push data across the network bridge to the robot and the player is ready
            if(this->playerStack[i].socket != INVALID_SOCKET && ((this->playerStack[i].isReady && !this->status->ready) || this->status->teleop))
            {
                QString outputBuff = "9!";
                //packJoystick state into string by converting the 16 bit number into two char variables, first char is int15:8 second char is int7:0
                outputBuff.append(',');
                outputBuff.append(QString::number(((this->playerStack[i].controller.Gamepad.sThumbLX + 32768)/255)));
                outputBuff.append(',');
                outputBuff.append(QString::number(((this->playerStack[i].controller.Gamepad.sThumbLY + 32768)/255)));
                outputBuff.append(',');
                outputBuff.append(QString::number(((this->playerStack[i].controller.Gamepad.sThumbRX + 32768)/255)));
                outputBuff.append(',');
                outputBuff.append(QString::number(((this->playerStack[i].controller.Gamepad.sThumbRY + 32768)/255)));
                outputBuff.append(',');
                outputBuff.append(QString::number(((this->playerStack[i].controller.Gamepad.bLeftTrigger + 32768)/255)));
                outputBuff.append(',');
                outputBuff.append(QString::number(((this->playerStack[i].controller.Gamepad.bRightTrigger + 32768)/255)));
                outputBuff.append(',');
                outputBuff.append(QString::number((this->playerStack[i].controller.Gamepad.wButtons>>8)));
                outputBuff.append(',');
                outputBuff.append(QString::number((this->playerStack[i].controller.Gamepad.wButtons&0xFF)));
                outputBuff.append('?');

                outputBuff[2] = outputBuff.length();//update datagram length
                int sendErr = send(this->playerStack[i].socket, outputBuff.toLocal8Bit(), outputBuff.length(), 0);
                if(sendErr != outputBuff.length())
                {
                    //SHOULD HANDLE ERRORS 
                    if(!sendErr) //if no data was sent assume socket is broken and disconect robot from GUI
                    {
                        disConRobot(i);   
                    }
                }
            }
        }
    }
}
void MainWindow::clearPlayerCont(struct player *play)
{
   play->joy_fd = UNNASSIGNED;
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
                    visablePButtons[0]->setText("Unlink Controller");
                    retVal = true;
            }
        }
        if(playerStack[playerNum].joy_fd == UNNASSIGNED)
        {
            debug("Could not find controller for player " + QString::number(ui->playerTabs->currentIndex()+1));
        }
    }
    else //player already has a controller assigned to them so we are going to unlink it from them
    {
        playerStack[playerNum].joy_fd = UNNASSIGNED;
        visablePButtons[0]->setText("Link Controller");
        debug("Unlinked controller from player " + QString::number(ui->playerTabs->currentIndex()+1));
    }
    return retVal;
}

void MainWindow::disConRobot(int playerNum)
{
    if(this->playerStack[playerNum].isConnected) //only run if player is already connected to a robot
    {
        this->playerStack[playerNum].address = "";
        this->playerStack[playerNum].isConnected=false;
        this->playerStack[playerNum].isReady = false;
        closesocket(this->playerStack[playerNum].socket);
        this->playerStack[playerNum].socket = INVALID_SOCKET;
        freeaddrinfo(this->playerStack[playerNum].add_inf);

        QList<QLabel *> visableLabel =ui->playerTabs->currentWidget()->findChildren<QLabel*>(); //create list of visable label widgets
        for(int i=0; i< visableLabel.length(); i++)
        {
            if(visableLabel[i]->objectName().endsWith("val_ipAdd"))
            {
                visableLabel[i]->setText("NONE");
            }
            else if(visableLabel[i]->objectName().endsWith("val_robStat"))
            {
                visableLabel[i]->setText("UNKNOWN");
            }
            else if(visableLabel[i]->objectName().endsWith("val_robMod"))
            {
                visableLabel[i]->setText("UNKNOWN");
            }
            else if(visableLabel[i]->objectName().endsWith("val_teamCol"))
            {
                visableLabel[i]->setText("UNKNOWN");
                this->playerStack[playerNum].teamColor=0;
            }
        }
    }
}
void MainWindow::toggleRobotReady()
{
    struct player *currPlayer = &(this->playerStack[ui->playerTabs->currentIndex()]);
    QList<QLabel *> visableLabel =ui->playerTabs->currentWidget()->findChildren<QLabel*>();
    QLabel *readyLabel;
    for(int i=0; i < visableLabel.length(); i++)
    {
        if(visableLabel[i]->objectName().endsWith("val_robStat"))
            readyLabel = visableLabel[i];
    }
    if(currPlayer->isConnected)
    {
        if(currPlayer->isReady)
        {
            currPlayer->isReady = false;
            readyLabel->setText("Not Ready");
        }
        else
        {
            currPlayer->isReady = true;
            readyLabel->setText("Ready");
        }
    }
}
void MainWindow::toggleRobotTeam(void)
{
    struct player *currPlayer = &(this->playerStack[ui->playerTabs->currentIndex()]);
    QList<QLabel *> visableLabel =ui->playerTabs->currentWidget()->findChildren<QLabel*>();
    QLabel *readyLabel;
    for(int i=0; i < visableLabel.length(); i++)
    {
        if(visableLabel[i]->objectName().endsWith("val_teamCol"))
            readyLabel = visableLabel[i];
    }
    if(currPlayer->isConnected)
    {
        if(currPlayer->teamColor == BLUE)
        {
            currPlayer->teamColor = RED;
            readyLabel->setText("RED");
        }
        else
        {
            currPlayer->teamColor = BLUE;
            readyLabel->setText("BLUE");
        }
    }
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

void MainWindow::on_p1_disCon_clicked()
{
    disConRobot(ui->playerTabs->currentIndex());
}

void MainWindow::on_p2_disCon_clicked()
{
    disConRobot(ui->playerTabs->currentIndex());
}

void MainWindow::on_p3_disCon_clicked()
{
    disConRobot(ui->playerTabs->currentIndex());
}

void MainWindow::on_p4_disCon_clicked()
{
    disConRobot(ui->playerTabs->currentIndex());
}

void MainWindow::on_p1_chng_stat_clicked()
{
    toggleRobotReady();
}

void MainWindow::on_p2_chng_stat_clicked()
{
    toggleRobotReady();
}

void MainWindow::on_p3_chng_stat_clicked()
{
    toggleRobotReady();
}

void MainWindow::on_p4_chng_stat_clicked()
{
    toggleRobotReady();
}

void MainWindow::on_p1_chng_team_clicked()
{
    toggleRobotTeam();
}

void MainWindow::on_p2_chng_team_clicked()
{
    toggleRobotTeam();
}

void MainWindow::on_p3_chng_team_clicked()
{
    toggleRobotTeam();
}

void MainWindow::on_p4_chng_team_clicked()
{
    toggleRobotTeam();
}

void MainWindow::on_readyMatch_clicked()
{
    int numReadyPlayers;
    for(numReadyPlayers=0;numReadyPlayers<4 && playerStack[numReadyPlayers].isReady;numReadyPlayers++);
    if(numReadyPlayers==1 && !status->matchStarted)
    {
        ui->curr_match_val->setText("READY");
        status->ready=true;
    }
    else status->ready=false;
}

void MainWindow::on_startMatch_clicked()
{
    if(status->ready && !status->matchStarted)
    {
        status->resetGameTimer(AUTO);
        status->matchTimer->start(1000);
        status->matchStarted=true;
        ui->t_left_val->setPalette(Qt::blue);
        ui->t_left_val->display(status->gameTimer);
        ui->curr_match_val->setText("AUTO");
    }
}
void MainWindow::updateGameTimer()
{
    ui->t_left_val->display(status->gameTimer);
}
void MainWindow::updateState()
{
    ui->curr_match_val->setText("TELEOP");
     ui->t_left_val->setPalette(Qt::green);
}
void MainWindow::matchIsOver()
{
    QList<QLabel *> visableLabel;
    ui->curr_match_val->setText("NOT READY");
    ui->t_left_val->setPalette(Qt::red);
    for(int i = 0; i<4;i++)
    {
        playerStack[i].isReady=false;

    }
    for(int j = 0; j < ui->playerTabs->count(); j++)
    {
       visableLabel =ui->playerTabs->widget(j)->findChildren<QLabel*>();
        for(int i = 0 ; i < visableLabel.length(); i++)
        {
            if(visableLabel[i]->objectName().endsWith("val_robStat"))
                visableLabel[i]->setText("NOT READY");
        }
    }
}
