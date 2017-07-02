#include "matchstatus.h"

MatchStatus::MatchStatus()
{
    this->matchTimer = new QTimer();
    teleop = false;
    matchStarted = false;
    ready = false;
    connect(this->matchTimer,SIGNAL(timeout()), this, SLOT(decreaseTime()));
}
void MatchStatus::resetGameTimer(bool mode)
{
    if(mode==AUTO)this->gameTimer=this->autoLen;
    else this->gameTimer=this->matchLen-this->autoLen;
}
void MatchStatus::decreaseTime()
{
    this->gameTimer--;
    if(this->gameTimer <= 0)
    {
        if(!this->teleop)
        {
            this->teleop=true;
            emit stateChanged();
            this->resetGameTimer(TELEOP);
        }
        else
        {
            this->matchStarted=false;
            this->ready=false;
            this->teleop=false;
            this->matchTimer->stop();
            emit matchOver();
        }
    }
}
