#include "host.h"
#define SINT_2_UINT(a) ((a+32768)*255/65535)
Host::Host()
{
    this->broadCastSock = new QUdpSocket();;
    this->commSock = new  QUdpSocket();
    this->clients = new QList<ConnectedClient>;
    this->robots = new QList<ConnectedRobot>;
    this->commSock->bind(HOST_LISTENING_PORT); 
    this->masterList = new QList<RobotInfo*>();
    //this->gameData = new GameData();
    for(int i=0;i<6;i++)
    {
        masterList->append(new RobotInfo());
    }
    connect(this->commSock, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(this,SIGNAL(receivedValidDgram(QByteArray)),this,SLOT(parseDgram(QByteArray)));
}
Host::~Host()
{
    this->broadCastSock->close();
    this->commSock->close();
    delete this->commSock;
    delete this->broadCastSock;
}
void Host::sendBroadcast()
{
    QByteArray datagram;
    QList<QHostAddress> data = QNetworkInterface::allAddresses();
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) //send a broadcast on all network connections
    {
        //check that connection is not a loopback and supports our protcol
        if (address.protocol() == QAbstractSocket::IPv4Protocol && (address != QHostAddress(QHostAddress::LocalHost)))
        {
             datagram.append(address.toString()); //write connection ip to datagram
             this->broadCastSock->writeDatagram(datagram, QHostAddress::Broadcast, BROADCAST_PORT); //broadcast datagram
             datagram.clear(); //clear the datagram for the next iteration
        }
    }
}

void Host::sendGameSync(QByteArray dgram)
{
    for(int i = 0;i<this->clients->size();i++)
    {
        this->commSock->writeDatagram(dgram,this->clients->at(i).addr,this->clients->at(i).port);
    }
}

void Host::readData()
{
    QByteArray datagram;
    datagram.resize(this->commSock->pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;

    this->commSock->readDatagram(datagram.data(), datagram.size(),&sender, &senderPort);
    D_MSG(datagram.data());
    this->checkValidDgram(datagram,sender,senderPort);
}
bool Host::checkValidDgram(QByteArray dgram, QHostAddress sender, quint16 senderPort)
{
    if(dgram.startsWith("gmd")) //checks for game synce flag
    {
        emit receivedValidDgram(dgram);
        return true;
    }
    else if(dgram.startsWith("CLI")) //checks for client flag
    {
        ConnectedClient cli;
        cli.addr = sender;
        cli.port = senderPort;
        QString name = QString::fromUtf8(dgram.data());
        cli.name = name.section(':',1);
        bool dupCli = false;
        for(int i = 0;i<clients->size();i++) //checks that the client is not already registered
        {
            if(clients->at(i).name==cli.name)
            {
                D_MSG("DUPLICATE CLIENT");
            }
        }
        if(!dupCli)
        {
            clients->append(cli);
            QByteArray dgram = "CLI:connected";
            this->broadCastSock->writeDatagram(dgram,sender,senderPort);
            emit clientAdded();
            return true;
        }
    }
    else if(dgram.startsWith("ROB"))
    {
        ConnectedRobot rob;
        rob.addr = sender;
        rob.port = HOST_LISTENING_PORT;
        QString name = QString::fromUtf8(dgram.data());
        rob.name = name.section(':',1);
        bool dupRob = false;
        for(int i = 0;i<robots->size();i++) //checks that the client is not already registered
        {
            if(robots->at(i).name==rob.name)
            {
                dupRob = true;
                D_MSG("DUPLICATE ROBOT");

                for(int j = 0; j<4;j++)
                {
                    if(this->masterList->at(j)->getName() == rob.name)
                    {
                        this->masterList->at(j)->updateTime(QTime::currentTime());
                    }
                }
                break;
            }
        }
        if(!dupRob)
        {
            robots->append(rob);
            QByteArray dgram = "ROB:connected";
            this->broadCastSock->writeDatagram(dgram,sender,BROADCAST_PORT);
            emit robotAdded();
            return true;
        }
    }
    else if(dgram.startsWith("DBG:"))
    {
        emit receivedDbgMsg(dgram);
        return true;
    }
    return false;
}
QList<QString> Host::getClientNames()
{
    QList<QString> names;
    for(int i =0;i<this->clients->size();i++)
    {
        D_MSG(clients->at(i).name);
        names.append(clients->at(i).name);

    }
    return names;
}
void Host::parseDgram(QByteArray dgram)
{
    QString header = QString::fromUtf8(dgram.data()).section("#",0,0);
    QStringList allData = QString::fromUtf8(dgram.data()).section("#",1,1).split(";");
    for(int i =0;i<allData.size();i++)
    {
        QStringList playerInfo = allData.at(i).split(":");
        int index = playerInfo.at(0).at(1).digitValue()-1;
        QString name = playerInfo.at(1);
        JoystickData data;
        data.lX = qint16(playerInfo.at(4).toInt());
        data.lY = qint16(playerInfo.at(5).toInt());
        data.rX = qint16(playerInfo.at(6).toInt());
        data.rY = qint16(playerInfo.at(7).toInt());
        data.lT = qint16(playerInfo.at(8).toInt());
        data.rT = qint16(playerInfo.at(9).toInt());
        data.buttons.bttns = quint16(playerInfo.at(10).toInt());
        this->masterList->at(index)->updateVal(name,data);
    }

}
void Host::sendRobotSync()
{
    for(int playerNum =0;playerNum<6;playerNum++)
    {
        for(int x = 0;x<this->robots->size();x++)
        {
            if(robots->at(x).name==this->masterList->at(playerNum)->getName())
            {
                if(playerNum<this->masterList->size())
                {
                    QTime currTime = QTime::currentTime();
                    if(currTime.msecsTo(this->masterList->at(playerNum)->getUpdate())<300 && this->masterList->at(playerNum)->getJoyIndex()!=-1)
                    {
                        QString dgram;
                        QTextStream stream(&dgram);
                        quint8 leftY = SINT_2_UINT(this->masterList->at(playerNum)->getJoystickData().lY);
                        quint8 rightX = SINT_2_UINT(this->masterList->at(playerNum)->getJoystickData().rX);
                        quint8 leftTrig = SINT_2_UINT(this->masterList->at(playerNum)->getJoystickData().lT);
                        quint8 rightTrig = SINT_2_UINT(this->masterList->at(playerNum)->getJoystickData().rT);
                        stream << "!ROB#"
                               << leftY << ":"
                               << rightX << ":"
                               << leftTrig << ":"
                               << rightTrig << "?";
                        D_MSG(dgram.toUtf8());
                        this->commSock->writeDatagram(dgram.toUtf8(),robots->at(x).addr,robots->at(x).port);
                    }
                    else
                    {

                    }
                }
            }
        }
    }
}
QList<RobotInfo*> *Host::getMasterList() const
{
    return masterList;
}

void Host::setMasterList(QList<RobotInfo*> *value)
{
    masterList = value;
}

QList<ConnectedRobot> *Host::getRobots() const
{
    return robots;
}

void Host::setRobots(QList<ConnectedRobot> *value)
{
    robots = value;
}

QList<ConnectedClient> *Host::getClients() const
{
    return clients;
}

void Host::setClients(QList<ConnectedClient> *value)
{
    clients = value;
}

