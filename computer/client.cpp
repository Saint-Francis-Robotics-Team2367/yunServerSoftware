#include "client.h"

Client::Client()
{
    this->connectedToHost = false;
    this->commSock = new QUdpSocket(this);
    this->broadSock = new QUdpSocket(this);
    if(!this->commSock->bind(IN_PORT))
       D_MSG("Failed to bind in port");
    if(!this->broadSock->bind(BROADCAST_PORT))
       D_MSG("Failed to bind to broadcast");
    connect(this->broadSock,SIGNAL(readyRead()),this,SLOT(receivedBroadPacket()));
    connect(this->commSock,SIGNAL(readyRead()),this,SLOT(receivedCommPacket()));
    connect(this,SIGNAL(connectRequest(QString)),this,SLOT(connectToHost(QString)));

}
Client::~Client()
{
    disconnect(this->broadSock,SIGNAL(readyRead()),this,SLOT(receivedBroadPacket()));
    disconnect(this->commSock,SIGNAL(readyRead()),this,SLOT(receivedCommPacket()));
    disconnect(this,SIGNAL(connectRequest(QString)),this,SLOT(connectToHost(QString)));
    this->commSock->close();
    this->broadSock->close();
    delete this->broadSock;
    delete this->commSock;
}
void Client::receivedBroadPacket()
{
    QByteArray datagram;
    datagram.resize(this->broadSock->pendingDatagramSize());
    QHostAddress sender;
    quint16 port;
    this->broadSock->readDatagram(datagram.data(),datagram.size(),&sender,&port);
    QString messStr = QString::fromUtf8(datagram.data());
    QString sendStr = sender.toString();
    //D_MSG(messStr);
    //D_MSG(sendStr);
    if(messStr.indexOf(sendStr)!=-1 && !this->connectedToHost)
    {
        emit connectRequest(sendStr);
    }
}

void Client::receivedCommPacket()
{
    D_MSG("Received data packet");
    while(this->commSock->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(this->commSock->pendingDatagramSize());
        QHostAddress sender;
        quint16 port;
        this->commSock->readDatagram(datagram.data(),datagram.size(),&sender,&port);
        QString messStr = QString::fromUtf8(datagram.data());
        D_MSG(messStr);
        if(messStr.indexOf("connected")!=-1)
        {
            connectedToHost = true;
        }
        if(messStr.startsWith("gmd"))
        {
            emit formattedPacket(messStr);
        }

    }
}

void Client::connectToHost(QString addr)
{
    hostAddr = new QHostAddress(addr);
    QByteArray data = "CLI:name";
    if(this->commSock->writeDatagram(data,*this->hostAddr,HOST_LISTEN_PORT)==-1)
    {
        D_MSG("Failed to send datagram");
    }
}

bool Client::isConnected()
{
    return this->connectedToHost;
}
void Client::sendGameSyncToHost(QByteArray dgram)
{
    this->commSock->writeDatagram(dgram,*this->hostAddr,HOST_LISTEN_PORT);
}
