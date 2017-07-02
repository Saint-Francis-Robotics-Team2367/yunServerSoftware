#include "robotinfo.h"

RobotInfo::RobotInfo()
{
    this->joyIndex = -1;
    for(int i =0;i<6;i++)
    {
        this->joystickData.buttons.bttns=0;
        this->joystickData.lX=0;
        this->joystickData.lY=1;
        this->joystickData.rX=2;
        this->joystickData.rY=3;
        this->joystickData.lT=4;
        this->joystickData.rT=5;
    }
    this->lastUpdate = QTime::currentTime().addSecs(-10);
}
void RobotInfo::updateVal(QString name, JoystickData data)
{
    this->name=name;
    this->joystickData=data;
    this->lastUpdate=QTime::currentTime();
}


QString RobotInfo::getName()
{
    return this->name;
}

QTime RobotInfo::getUpdate()
{
    return this->lastUpdate;
}
JoystickData RobotInfo::getJoystickData()
{
    return this->joystickData;
}
int RobotInfo::getJoyIndex() const
{
    return joyIndex;
}

void RobotInfo::setJoyIndex(int value)
{
    joyIndex = value;
}
void RobotInfo::setJoystickData(JoystickData data)
{
    this->lastUpdate=QTime::currentTime();
    this->joystickData = data;
}
void RobotInfo::setName(QString name)
{
    this->name = name;
}

void RobotInfo::updateTime(QTime t)
{
    this->lastUpdate = t;
}
