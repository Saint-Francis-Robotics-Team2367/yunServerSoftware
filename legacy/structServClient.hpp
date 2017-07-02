#include <linux/joystick.h>
#include <arpa/inet.h>
struct player
{
	//SERVER INFO
	int id;
	char address[128];
	int port;
	int playerNum;
	int teamName;
	int socket;
	bool isConnected;
	int teamColor;
	//JOYSTICK INFO
	char joyLoc[15];
	int joy_fd, *axis, num_of_axis, num_of_buttons, x;
	char *button, name_of_joystick[80];
	struct js_event js;
	int varBut;
	struct player *nextPlayer;
	bool isReady;
	char buffer[512];
	bool buffFull;
	struct sockaddr_in server;

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
