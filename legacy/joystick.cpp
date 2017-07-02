#include "joystick.hpp"

int initializeJoystick(struct player *play)
{
	 if( ( play->joy_fd = open( play->joyLoc , O_RDONLY)) == -1 )
        {
                printf( "Couldn't open joystick\n" );
                return -1;
        }
        ioctl( play->joy_fd, JSIOCGAXES, &play->num_of_axis );
        ioctl( play->joy_fd, JSIOCGBUTTONS, &play->num_of_buttons );
        ioctl( play->joy_fd, JSIOCGNAME(80), &play->name_of_joystick );

        play->axis = (int *) calloc( play->num_of_axis, sizeof( int ) );
        play->button = (char *) calloc( play->num_of_buttons, sizeof( char ) );

        printf("Joystick detected: %s\n\t%d axis\n\t%d buttons\n\n"
                , play->name_of_joystick
                , play->num_of_axis
                , play->num_of_buttons );

        fcntl( play->joy_fd, F_SETFL, O_NONBLOCK );   /* use non-blocking mode */
}
void updateJoyValues(struct player *play)
{
	read(play->joy_fd, &play->js, sizeof(struct js_event));
               
                        /* see what to do with the event */
	switch (play->js.type & ~JS_EVENT_INIT)
	{
	case JS_EVENT_AXIS:
		play->axis   [ play->js.number ] = play->js.value;
		break;
	case JS_EVENT_BUTTON:
		play->button [ play->js.number ] = play->js.value;
		break;
	}
}
