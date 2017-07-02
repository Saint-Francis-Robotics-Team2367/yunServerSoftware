#ifndef ROBOTINFO_H
#define ROBOTINFO_H

#include <QTime>
#include<stdint.h>
#include<joystickhandler.h>
typedef struct
{
 qint16 lX;
 qint16 lY;
 qint16 rX;
 qint16 rY;
 qint16 lT;
 qint16 rT;
 buttonArry buttons;
}JoystickData;
class RobotInfo
{
public:
    RobotInfo();
    void updateVal(QString name, JoystickData data);
    QString getName();
    void setName(QString name);
    QTime getUpdate();
    JoystickData getJoystickData();
    void setJoystickData(JoystickData data);
    int getJoyIndex() const;
    void setJoyIndex(int value);
    void updateTime(QTime t);

private:
    QTime lastUpdate;
    JoystickData joystickData;
    QString name;
    int joyIndex;
};

#endif // ROBOTINFO_H
