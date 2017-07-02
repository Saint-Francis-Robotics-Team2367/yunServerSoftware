#include "clientNetworkCom.cpp"
#ifndef CLIENT_NETWORK_COM_H
#define CLIENT_NETWORK_COM_H



#define MAX_CLIENTS 5
#define MAX_BUFF 100
#define FORMATING_ERROR -1
#define BUFFER_FULL -8
#define TIME_SET 7

#define HANDSHAKE 1
#define READYING 2
#define UN_READYING 3
#define TIME 4
#define ANNOUNCE_SERVER 5
#define BEGIN_MATCH 6
#define BEGIN_TELEOP 7
#define END_MATCH 8
#define JOY_STICK 9

#define AUTO_LEN 15
#define MATCH_LEN 120

#include "structServClient.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <string.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <net/if.h>
#include <linux/joystick.h>
#include <time.h>
#include <stddef.h>
#include <cstddef>
#include <errno.h>



int sendString(char *message, struct client *clients);
int sendString(char *message, struct player *player);
int recvString(struct client *client);
int readyUp(struct client *client);
int getConnectionType(struct client *client);
void announceServers(struct client *client);
void sendBeginMatch(struct player *player);
void sendBeginTeleop(struct player *player);
void sendEndMatch(struct player *player);
int createSocket(struct client *cli);
int createSocket(struct player *play);
int handshake(struct player *play);
int connect(struct player *play);
void sendJoyPos(struct player *player);


#endif // CLIENT_NETWORK_COM_H
