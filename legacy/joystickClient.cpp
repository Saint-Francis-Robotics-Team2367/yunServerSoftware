#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <string.h>

#define JOY_DEV "/dev/input/js0"

int main()
{
	//set up of socket stuff

	int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
     
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr("192.168.137.146");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");

	//end socket stuff

	int joy_fd, *axis=NULL, num_of_axis=0, num_of_buttons=0, x;
	char *button=NULL, name_of_joystick[80];
	struct js_event js;

	if( ( joy_fd = open( JOY_DEV , O_RDONLY)) == -1 )
	{
		printf( "Couldn't open joystick\n" );
		return -1;
	}
	ioctl( joy_fd, JSIOCGAXES, &num_of_axis );
	ioctl( joy_fd, JSIOCGBUTTONS, &num_of_buttons );
	ioctl( joy_fd, JSIOCGNAME(80), &name_of_joystick );

	axis = (int *) calloc( num_of_axis, sizeof( int ) );
	button = (char *) calloc( num_of_buttons, sizeof( char ) );

	printf("Joystick detected: %s\n\t%d axis\n\t%d buttons\n\n"
		, name_of_joystick
		, num_of_axis
		, num_of_buttons );

	fcntl( joy_fd, F_SETFL, O_NONBLOCK );	/* use non-blocking mode */
	clock_t timer = clock();
	while( 1 ) 	/* infinite loop */
	{
		read(joy_fd, &js, sizeof(struct js_event));
		
			/* see what to do with the event */
		switch (js.type & ~JS_EVENT_INIT)
		{
			case JS_EVENT_AXIS:
				axis   [ js.number ] = js.value;
				break;
			case JS_EVENT_BUTTON:
				button [ js.number ] = js.value;
				break;
		}

		if((clock() -timer)> 0.03 * CLOCKS_PER_SEC)
		{
		timer = clock();

		char varBut = 0;
		for(int x=0; x<6; x++)
		{
			if(button[x])varBut=varBut|(1<<x);
		}
		if(button[9])varBut=varBut|(1<<9);
		if(button[10])varBut=varBut|(1<<10);
		sprintf(message, "%d,%d,%d,%d,%d,%d,%d,%d,%c",axis[0],axis[1],axis[2],axis[3],axis[4],axis[5],axis[6],axis[7],varBut);
		fflush(stdout);	
		if( send(sock , message , strlen(message) , 0) < 0)
      		{
         		  puts("Send failed");
          		  return 1;
       		}

		}
	

	}

	close( joy_fd );	/* too bad we never get here */
	return 0;
}
