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
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <net/if.h>

//STATES

#define S_ENTRY 9001
#define S_GAME_CONTROL 9002
#define S_NO_CONNECT 9003
#define S_SCAN 9004
#define S_AUTO 9005
#define S_TELEOP 9006
#define S_EXIT	9007

#define HANDSHAKE "1"
#define DATA 2

#define RED_TEAM 1
#define BLUE_TEAM 2

using namespace std;

int state = S_ENTRY;
char address[12];
int port;

//PLAYER



struct client
{
	int id;
	int socket;
	bool isConnected;
};

//initialize all players
struct player player1;
struct player player2;
struct player player3;
struct player player4;
struct player player5;
struct player player6;
struct client client1;
struct client client2;
struct client client3;
struct client client4;
struct client client5;

//create all sockets
int createSocket(struct player play);
int createSocket(struct client play);
int handshake(struct player play);
int handshake(struct client cli);

int main()
{
	//clock_t timer = clock();
	while (1)
	{
		if (state == S_ENTRY)
		{
			createSocket(player1);
			createSocket(player2);
			createSocket(player3);
			createSocket(player4);
			createSocket(player5);
			createSocket(player6);
			createSocket(client1);
			createSocket(client2);
			createSocket(client3);
			createSocket(client4);
			createSocket(client5);
			state = S_SCAN;

		}
		if (state == S_GAME_CONTROL)
		{

		}
		if (state == S_NO_CONNECT)
		{
			//if(connectToServer(player.address[],player.port))initialize();
			//else state=S_ENTRY;
		}
		if (state == S_SCAN)
		{
			//GET LOCAL IP
			struct ifaddrs * ifAddrStruct = NULL;
			struct ifaddrs * ifa = NULL;
			void * tmpAddrPtr = NULL;

			getifaddrs(&ifAddrStruct);
			char addressBuffer[INET_ADDRSTRLEN];
			for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
			{
				if (ifa->ifa_addr->sa_family == AF_INET)
				{ // check it is IP4
				  // is a valid IP4 Address
					tmpAddrPtr =
							&((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;

					inet_ntop(AF_INET, tmpAddrPtr, addressBuffer,
					INET_ADDRSTRLEN);
					printf("'%s': %s\n", ifa->ifa_name, addressBuffer);
				}
			}
			if (ifAddrStruct != NULL) freeifaddrs(ifAddrStruct);//remember to free ifAddrStruct

			int ip1, ip2, ip3, ip4;
			sscanf(addressBuffer, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
			struct sockaddr_in scanServer;
			//Create socket
			char scanServerAddress[18];
			bool ipAssigned;
			for (int i = 0; i < 256; i++)
			{
				ipAssigned = false;
				sprintf(scanServerAddress, "%d.%d.%d.%d", ip1, ip2, ip3, i);
				printf("Address: %s Scanning %f%\n", scanServerAddress,
						i / 256. * 100);
				scanServer.sin_addr.s_addr = inet_addr(scanServerAddress);
				scanServer.sin_family = AF_INET;
				scanServer.sin_port = htons(8888);
				if (!player1.isConnected && !ipAssigned)
				{
					if (connect(player1.socket, (struct sockaddr *) &scanServer,
							sizeof(scanServer)) < 0 && handshake(player1))
					{
						player1.address = scanServer.sin_addr.s_addr;
						player1.isConnected = true;
						ipAssigned = true;
					}
				}
				else if (!player2.isConnected && !ipAssigned)
				{
					if (connect(player1.socket, (struct sockaddr *) &scanServer,
							sizeof(scanServer)) < 0 && handshake(player2))
					{
						player1.address = scanServer.sin_addr.s_addr;
						player1.isConnected = true;
						ipAssigned = true;
					}
				}
				else if (!player3.isConnected && !ipAssigned)
				{
					if (connect(player1.socket, (struct sockaddr *) &scanServer,
							sizeof(scanServer)) < 0 && handshake(player3))
					{
						player1.address = scanServer.sin_addr.s_addr;
						player1.isConnected = true;
						ipAssigned = true;
					}
				}
				else if (!player4.isConnected && !ipAssigned)
				{
					if (connect(player1.socket, (struct sockaddr *) &scanServer,
							sizeof(scanServer)) < 0 && handshake(player4))
					{
						player1.address = scanServer.sin_addr.s_addr;
						player1.isConnected = true;
						ipAssigned = true;
					}
				}
				else if (!player5.isConnected && !ipAssigned)
				{
					if (connect(player1.socket, (struct sockaddr *) &scanServer,
							sizeof(scanServer)) < 0 && handshake(player5))
					{
						player1.address = scanServer.sin_addr.s_addr;
						player1.isConnected = true;
						ipAssigned = true;
					}
				}
				else if (!player6.isConnected && !ipAssigned)
				{
					if (connect(player1.socket, (struct sockaddr *) &scanServer,
							sizeof(scanServer)) < 0 && handshake(player6))
					{
						player1.address = scanServer.sin_addr.s_addr;
						player1.isConnected = true;
						ipAssigned = true;
					}
				}
			}
			state = S_EXIT;
		}
		if (state == S_AUTO)
		{

		}
		if (state == S_TELEOP)
		{

		}
		if (state == S_EXIT)
		{
			return 1;
		}

	}
}

int initialize()
{
	return 1;
}

int initializeJoystick(struct player play)
{
	if ((play.joy_fd = open(play.joyLoc, O_RDONLY)) == -1)
	{
		printf("Couldn't open joystick\n");
		return -1;
	}
	ioctl(play.joy_fd, JSIOCGAXES, &play.num_of_axis);
	ioctl(play.joy_fd, JSIOCGBUTTONS, &play.num_of_buttons);
	ioctl(play.joy_fd, JSIOCGNAME(80), &play.name_of_joystick);

	play.axis = (int *) calloc(play.num_of_axis, sizeof(int));
	play.button = (char *) calloc(play.num_of_buttons, sizeof(char));

	printf("Joystick detected: %s\n\t%d axis\n\t%d buttons\n\n",
			play.name_of_joystick, play.num_of_axis, play.num_of_buttons);

	fcntl(play.joy_fd, F_SETFL, O_NONBLOCK); /* use non-blocking mode */
	return 1;
}
int getJoyValues(struct player *play)
{
	read(play->joy_fd, &play->js, sizeof(struct js_event));

	/* see what to do with the event */
	switch (play->js.type & ~JS_EVENT_INIT)
	{
	case JS_EVENT_AXIS:
		play->axis[play->js.number] = play->js.value;
		break;
	case JS_EVENT_BUTTON:
		play->button[play->js.number] = play->js.value;
		break;
	}
	for (int x = 0; x < 6; x++)
	{
		if (play->button[x]) play->varBut = play->varBut | (1 << x);
	}
	if (play->button[9]) play->varBut = play->varBut | (1 << 9);
	if (play->button[10]) play->varBut = play->varBut | (1 << 10);
	return 1;
}
int sendJoyValues(struct player play)
{
	char message[1000];
	sprintf(message, "%d,%d,%d,%d,%d,%d,%d,%d,%c", play.axis[0], play.axis[1],
			play.axis[2], play.axis[3], play.axis[4], play.axis[5],
			play.axis[6], play.axis[7], play.varBut);
	if (send(play.socket, message, strlen(message), 0) < 0)
	{
		puts("Send failed");
		return 0;
	}
	return 1;
}
int createSocket(struct player play)
{
	play.socket = socket(AF_INET, SOCK_STREAM, 0);
	if (play.socket == -1)
	{
		printf("Player%d Socket creation failed", play.id);
		return 0;
	}
	return 1;
}
int createSocket(struct client cli)
{
	cli.socket = socket(AF_INET, SOCK_STREAM, 0);
	if (cli.socket == -1)
	{
		printf("Client%d Socket creation failed", cli.id);
		return 0;
	}
	return 1;
}
int handshake(struct player play)
{
	if (send(play.socket, HANDSHAKE, strlen(HANDSHAKE), 0) < 0) return 0;
	char returnMessage[20];
	recv(play.socket, returnMessage, 20, 0);
	if (returnMessage == "lancer") ;
	{
		recv(play.socket, returnMessage, 20, 0);
		if (returnMessage == "1")
		{
			play.isConnected = true;
			return 1;
		}
	}
	return 0;
}
int handshake(struct client cli)
{
	if (send(cli.socket, HANDSHAKE, strlen(HANDSHAKE), 0) < 0) return 0;
	char returnMessage[20];
	recv(cli.socket, returnMessage, 20, 0);
	if (returnMessage == "lancer") ;
	{
		recv(cli.socket, returnMessage, 20, 0);
		if (returnMessage == "1")
		{
			cli.isConnected = true;
			return 1;
		}
	}
	return 0;
}
