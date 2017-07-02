#include "joystickhandler.h"

JoyStickHandler::JoyStickHandler()
{
#if defined(__WIN32) || defined(__WIN64) || defined(__WINNT)
    XInputEnable(TRUE); /*make sure xinput is enabled*/
     this->joy_dx_index = JOYSTICK_NOT_CONNECTED;
#elif __linux

#elif __APPLE__
    //Gamepad_deviceAttachFunc(jsAttached, NULL);
    //Gamepad_deviceRemoveFunc(jsRemoved, NULL);
    //Gamepad_buttonDownFunc(jsButtonDown, NULL);
    //Gamepad_axisMoveFunc(jsTriggerMoved, NULL);
    //Gamepad_init();
#endif

    //set up joystick assuming xbox 360 controller
    this->axisCount =6;
    this->axisVal = new int16_t[this->axisCount];
    this->buttonCount = 14;
    this->buttonVal = new buttonArry;

    for(int i = 0; i < this->axisCount; i++)
    {
        this->axisVal[i] = 0;
    }
    this->buttonVal->bttns = 0;
}

JoyStickHandler::~JoyStickHandler()
{
}

qint16 JoyStickHandler::readRawBttn(int index){
    return this->buttonVal[index].bttns;
}
qint16 JoyStickHandler::readAxis(int index){
    return this->axisVal[index];
}
buttonArry JoyStickHandler::readBttn(int index){
    return this->buttonVal[index];
}

#if defined(__WIN32) || defined(__WIN64) || defined(__WINNT)
void JoyStickHandler::initJoystick(int index)
{
    this->joy_dx_index = index;
}

void JoyStickHandler::updateJoystick()
{
    if(this->joy_dx_index != JOYSTICK_NOT_CONNECTED)
    {
        if(XInputGetState(this->joy_dx_index, &this->controller) != ERROR_DEVICE_NOT_CONNECTED)
        {
            this->axisVal[0] = this->controller.Gamepad.sThumbLX;
            this->axisVal[1] = this->controller.Gamepad.sThumbLY;
            this->axisVal[2] = this->controller.Gamepad.sThumbRX;
            this->axisVal[3] = this->controller.Gamepad.sThumbRY;
            this->axisVal[4] = this->controller.Gamepad.bLeftTrigger <<7;   /*The left and right triggers are scaled up from 8 bit to ~16 bit*/
            this->axisVal[5] = this->controller.Gamepad.bRightTrigger << 7; /*The left and right triggers are scaled up from 8 bit to ~16 bit*/
            this->buttonVal[0].bttns = this->controller.Gamepad.wButtons;
        }
        else
        {
            emit joystickMissing(this->joy_dx_index);
            this->joy_dx_index = JOYSTICK_NOT_CONNECTED;
        }
    }
}

void JoyStickHandler::rumbleJoystick(unsigned int lMtr, unsigned int rMtr)
{
    if(this->joy_dx_index != JOYSTICK_NOT_CONNECTED)
    {
        XINPUT_VIBRATION val;
        val.wLeftMotorSpeed = lMtr;
        val.wRightMotorSpeed = rMtr;

        if(XInputSetState(this->joy_dx_index, &val) == ERROR_DEVICE_NOT_CONNECTED)
        {
            emit joystickMissing(this->joy_dx_index);
        }
    }
}

#elif __linux
void JoyStickHandler::initJoystick(int index)
{
    QString location = "/dev/input/js" + index;
    if( ( this->joy_fd = open( location.toUtf8().data(), O_RDONLY)) == -1 )
    {
        D_MSG( "Couldn't open joystick\n" );
        emit joystickMissing(index);
    }
    ioctl( this->joy_fd, JSIOCGAXES, &this->num_of_axis );
    ioctl( this->joy_fd, JSIOCGBUTTONS, &this->num_of_buttons );
    ioctl( this->joy_fd, JSIOCGNAME(80), &this->name_of_joystick );

    this->axis = (int *) calloc( this->num_of_axis, sizeof( int ) );
    this->button = (char *) calloc( this->num_of_buttons, sizeof( char ) );

    D_MSG("Joystick detected");

    fcntl( this->joy_fd, F_SETFL, O_NONBLOCK );   /* use non-blocking mode */
}

void JoyStickHandler::updateJoystick()
{
    while(read(this->joy_fd, &this->js, sizeof(struct js_event))!=-1);

    {                    /* see what to do with the event */
        switch (this->js.type & ~JS_EVENT_INIT)
        {
        case JS_EVENT_AXIS:
            this->axis   [ this->js.number ] = this->js.value;
            break;
        case JS_EVENT_BUTTON:
            this->button [ this->js.number ] = this->js.value;
            break;
        }
    }

}

void JoyStickHandler::rumbleJoystick(int lMtr, int rMtr)
{

}
void JoyStickHandler::rumbleJoystick(unsigned int lMtr, unsigned int rMtr)
{

}
#elif __APPLE__


void JoyStickHandler::jsAttached(struct Gamepad_device * device, void * context) {

}
void JoyStickHandler::jsRemoved(struct Gamepad_device * device, void * context) {

}
void JoyStickHandler::jsTriggerMoved(struct Gamepad_device * device, unsigned int axisID, float value, float lastValue, double timestamp, void * context) {

}
void JoyStickHandler::jsButtonDown(struct Gamepad_device * device, unsigned int buttonID, double timestamp, void * context) {

}

void JoyStickHandler::initJoystick(int index)
{

}

void JoyStickHandler::updateJoystick()
{

}

void JoyStickHandler::rumbleJoystick(int lMtr, int rMtr)
{

}
void JoyStickHandler::rumbleJoystick(unsigned int lMtr, unsigned int rMtr)
{

}
#endif
