#ifndef MATCHSTATUS_H
#define MATCHSTATUS_H
#include <QTimer>
#define TELEOP true
#define AUTO false

class MatchStatus : public QObject
{
    Q_OBJECT
public:
    MatchStatus();
    bool teleop;
    bool matchStarted;
    bool ready;
    int gameTimer;
    QTimer *matchTimer;
    void resetGameTimer(bool mode);
private:
    static const int matchLen = 120;
    static const int autoLen = 15;
    static const int preGame = 5;
private slots:
    void decreaseTime();

signals:
    void timeChanged();
    void matchOver();
    void stateChanged();
};


#endif // MATCHSTATUS_H
