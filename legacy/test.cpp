#include "networkCom.cpp"
#include "joystick.cpp"
int main()
{
	#ifdef DEBUG


	#endif
	struct player play1;
	strcpy(play1.address, "192.168.137.126");
	play1.port=8888;
	if(!createSocket(&play1)){
		puts("could not connect");
	}
	if(connect(&play1))printf("Connection Successful\n");
	else printf("Connection failed\n");
	memset(play1.buffer,0,512);
	strcpy(play1.joyLoc,"/dev/input/js0");
	initializeJoystick(&play1);
	while(1)
	{
		//if(handshake(&play1))
		//{
			updateJoyValues(&play1);
			//printf("Handshake successful\n");
			putc (*(play1.buffer),stdout);
			sendJoyPos(&play1);
//			sendBeginMatch(&play1);
//			sleep(1);
//			sendBeginTeleop(&play1);
//			sleep(1);
//			sendEndMatch(&play1);
//		}
//		else
//		{		
//			printf("Handshake failed\n");
//		}
//		sleep(5);
	}
	
	return 1;
}
