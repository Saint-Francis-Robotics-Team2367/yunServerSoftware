#ifndef JOYSTICKHANDLER_H
#define JOYSTICKHANDLER_H

#include<stdint.h>
#include <QObject>
#include <QDebug>

#define JOYSTICK_NOT_CONNECTED -10

#define DEBUG /* comment out this line to lower the verbosity of the program */


#if  defined(DEBUG) || defined(GLOBAL_DEBUG)
#define D_MSG(a) qDebug()<<a
#else
#define D_MSG(a)
#endif

#if defined(__WIN32) || defined(__WIN64) || defined(__WINNT)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include<xinput.h>
    #pragma message( "This version of the library is designed for xbox 360 controllers and requires XInput" )
#elif __linux
    #include <stdio.h>
    #include <stdlib.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <linux/joystick.h>
    #include <string.h>
    #pragma message( "This version of the library is designed for controllers and requires linux stuff" )
#elif __APPLE__
extern "C" {
//  #include <LibstemJoystick/gamepad/Gamepad.h>
}
#endif

enum Button
{
    DPAD_UP,
    DPAD_DOWN,
    DPAD_LEFT,
    DPAD_RIGHT,
    START,
    BACK,
    LEFT_THUMB,
    RIGHT_THUMB,
    LEFT_SHOULDER,
    RIGHT_SHOULDER,
    A,
    B,
    X,
    Y
};

typedef union
{
  struct
  {
    bool DPAD_UP        : 1;
    bool DPAD_DOWN      : 1;
    bool DPAD_LEFT      : 1;
    bool DPAD_RIGHT     : 1;
    bool START          : 1;
    bool BACK           : 1;
    bool LEFT_THUMB     : 1;
    bool RIGHT_THUMB    : 1;
    bool LEFT_SHOULDER  : 1;
    bool RIGHT_SHOULDER : 1;
    bool A              : 1;
    bool B              : 1;
    bool X              : 1;
    bool Y              : 1;
  }indvBttn;
  struct
  {
      uint8_t lByte;
      uint8_t hByte;
  }byte;
  uint16_t bttns;

}buttonArry;

class JoyStickHandler :  public QObject
{
    Q_OBJECT
private:
   int axisCount;
   int16_t *axisVal;
   int buttonCount;
   buttonArry *buttonVal;

    #if defined(__WIN32) || defined(__WIN64) || defined(__WINNT)
        int joy_dx_index;
        XINPUT_STATE controller;
    #elif __linux
   char joyLoc[15];
   int joy_fd, *axis, num_of_axis, num_of_buttons, x;
   char *button, name_of_joystick[80];
   struct js_event js;
   int varBut;

#elif __APPLE__
#endif

public:
    JoyStickHandler();
    ~JoyStickHandler();
    buttonArry readBttn(int index);
    qint16 readRawBttn(int index);
    qint16 readAxis(int index);
    void initJoystick(int index);
    void updateJoystick();
    void rumbleJoystick(unsigned int lMtr, unsigned int rMtr);
    void rumbleJoystick(int lMtr,int rMtr);

    static void jsAttached(struct Gamepad_device * device, void * context);
    static void jsRemoved(struct Gamepad_device * device, void * context);
    static void jsTriggerMoved(struct Gamepad_device * device, unsigned int axisID, float value, float lastValue, double timestamp, void * context);
    static void jsButtonDown(struct Gamepad_device * device, unsigned int buttonID, double timestamp, void * context);

signals:
    void joystickMissing(int index);
};

#endif // JOYSTICKHANDLER_H
