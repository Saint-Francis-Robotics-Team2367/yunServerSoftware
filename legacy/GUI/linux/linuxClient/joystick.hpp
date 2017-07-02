#include <linux/joystick.h>
#include <sys/ioctl.h>
#include "joystick.cpp"

int initializeJoystick(struct player *play);
void updateJoyValues(struct player *play);
