#ifndef STRUCT_SERV_CLIENT_H
#define STRUCT_SERV_CLIENT_H

#include <xinput.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define UNNASSIGNED -1
#define LOCAL_HOST "127.0.0.1"
#define RED 100
#define BLUE 101
#define NO_GAMEPAD_LINKED -1

#define CLIENT_INTERROGATION_PHRASE "1!3"
#define INTERROGATION_RESPONSE "lancer"
#define MAX_INTERROGATION_RETRIES 3
#define MAX_SEND_RETRIES 1
#define MAX_RECV_RETIRES 1


struct player
{
	//SERVER INFO
	int id;
    QString address;
	int port;
	int playerNum;
    int teamName;
    SOCKET socket;
	bool isConnected;
	int teamColor;

	//JOYSTICK INFO
	char joyLoc[15];
	int joy_fd, *axis, num_of_axis, num_of_buttons, x;
    char *button, name_of_joystick[80];
	int varBut;
	struct player *nextPlayer;
	bool isReady;
	char buffer[512];
	bool buffFull;
	struct sockaddr_in server;
    struct addrinfo *add_inf;
    XINPUT_STATE controller;

};

struct client
{
	float mainTime, startTime, autoEndTime, teleEndTime;
	int id;
	int socket;
	bool isConnected;
	char buffer[512];
	bool buffFull;
	bool isReady;
	struct client *nextClient;
	struct player player;

};

#endif
