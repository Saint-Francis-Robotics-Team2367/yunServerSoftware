#ifndef GAMEDATA_H
#define GAMEDATA_H
#include <QObject>
#include <QTimer>
#include <QtNetwork>
#include <udpsend.h>
#include <joystickhandler.h>

typedef union {
    unsigned char data[200];
    struct {
        char signature[3];
        char delimiter;
        unsigned char gameTime;
    }comps;
} allGameData;

class GameData : public QObject
{
    Q_OBJECT

public:
    explicit GameData(JoyStickHandler *joystickRef);
    ~GameData();

    QTimer gameDataTimer;

    float getGameTime();
    int  getGameSyncs();

    void startGameUpdates();
    void endGameUpdates();

signals:
    void signalToHost(QByteArray data);
public slots:
    void updateGameData();

private:
   float gameTime;
   int gameSyncs;

   JoyStickHandler *jsData;

   QString robotNames[6];
   bool joystickConnections[6];
   char clientConnections[6];
   int robotConnections[6];

};

#endif // GAMEDATA_H
