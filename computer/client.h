#ifndef CLIENT_H
#define CLIENT_H
#include <QObject>
#include <QtNetwork>
#include <udpsend.h>

#define DEBUG /* comment out this line to lower the verbosity of the program */
#define IN_PORT 2367
#define HOST_LISTEN_PORT 2380
#define BROADCAST_PORT 23005

#ifndef D_MSG
    #if  (defined(DEBUG) || defined(GLOBAL_DEBUG))
        #define D_MSG(a) qDebug()<<a;
    #else
        #define D_MSG(a)
    #endif
#endif

class Client : public UdpSend
{
    Q_OBJECT
public:
    Client();
    ~Client();
    bool isConnected();
    void sendGameSyncToHost(QByteArray dgram);
private:
    QUdpSocket *commSock;
    QUdpSocket *broadSock;
    bool connectedToHost;
    QHostAddress *hostAddr;
public slots:
    void connectToHost(QString addr);
    void receivedCommPacket();
    void receivedBroadPacket();
signals:
    void connectRequest(QString);
    void formattedPacket(QString);

};

#endif // CLIENT_H
