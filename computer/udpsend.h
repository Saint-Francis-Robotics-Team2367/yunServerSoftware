#ifndef UDPSEND_H
#define UDPSEND_H
#include <QObject>
#include <QtNetwork>

class UdpSend : public QObject
{
    Q_OBJECT
public:
    explicit UdpSend();
    ~UdpSend();
public slots:
    //void sendVals(QString data, QHostAddress host);
    void sendVals(QString data);
};

#endif // UDPSEND_H
