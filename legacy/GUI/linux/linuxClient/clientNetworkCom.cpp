
int sendString(char *message, struct client *clients)
{
	char* nullPos;
	if(!(nullPos = strchr(message,'\0'))) return FORMATING_ERROR;
	
	while(clients!=NULL)
	{
		int sendRet = send(clients->socket, message, strlen(message),0);
		if(sendRet == nullPos - message) printf("Send successful\n");
		else{
			printf("Error :%d ",sendRet);
			if(sendRet == EACCES) 
			{
				printf("Access Denied: Try running as root\n");
				return EACCES;
			}
			else if(sendRet == EAGAIN || sendRet == EWOULDBLOCK)
			{
				printf("Message too long\n");
				return EWOULDBLOCK;
			}
			else if(sendRet == ECONNRESET)
			{
				printf("Connecting reset by peer\n");
				return ECONNRESET;
			}
			
		}
		clients = clients->nextClient;
	}
	return nullPos - message;
}

int sendString(char *message, struct player *player)
{
	char* nullPos;
	if(!(nullPos = strchr(message,'\0'))) return FORMATING_ERROR;
	
	int sendRet = send(player->socket, message, strlen(message),0);
		
	if(sendRet == nullPos - message) printf(" ");
	else
	{
		printf("Error :%d ",sendRet);
		if(sendRet == EACCES) 
		{
			printf("Access Denied: Try running as root\n");
			return EACCES;
		}
		else if(sendRet == EAGAIN || sendRet == EWOULDBLOCK)
		{
			printf("Message too long\n");
			return EWOULDBLOCK;
		}
		else if(sendRet == ECONNRESET)
		{
			printf("Connecting reset by peer\n");
			return ECONNRESET;
		}

	}
	return nullPos - message;
}

int recvString(struct client *client)
{
	if(!client->buffFull)
	{
		int recvRet = recv(client->socket, client->buffer, MAX_BUFF, 0);
		if(recvRet == EAGAIN || recvRet == EWOULDBLOCK)
		{
			printf("Message too long\n");
			return EWOULDBLOCK;
		}
		else if(ECONNREFUSED)
		{
			printf("Connection refused\n");
			return ECONNREFUSED;
		}
		client->buffFull = true;
	}
	else return BUFFER_FULL;
}
int recvString(struct player *player)
{
	if(!player->buffFull)
	{
		int recvFlag = 1;
		printf("Buff not full\n");
		time_t startTime = time(NULL);
		printf("%f\n",difftime(time(NULL),startTime));
		while(difftime(time(NULL),startTime)<5 && recvFlag)
		{
			printf("In loop\n");
			int recvRet = recv(player->socket, player->buffer, MAX_BUFF, 0);
			printf("Received %d bytes\n",recvRet);
	
			if(recvRet == EAGAIN || recvRet == EWOULDBLOCK)
			{
				printf("Message too long\n");
				//return EWOULDBLOCK;
			}
			else if(recvRet == ECONNREFUSED)
			{
				printf("Connection refused\n");
				//return ECONNREFUSED;
			}
			else recvFlag=0;
		}
		printf("Exit Loop\n");
		player->buffFull = true;
	}
	else
	{
		printf("Buff full\n");
		return BUFFER_FULL;
	}
}
int getConnectionType(struct client *client)
{
	int connectType;	
	scanf(client->buffer, "%d!",&connectType);
	return connectType;
}
int readyUp(struct client *client)
{
	int numReadyPlayers = 0;
	for(struct client *mover = client; mover->nextClient; mover= mover->nextClient)
	{
		for(struct player *playMove = &mover->player; playMove->nextPlayer; playMove->nextPlayer)
		{
			numReadyPlayers +=mover->isReady;
			recvString(client);
			if(client->buffFull)
			{
				if(getConnectionType(client)==READYING)
				{
					mover->isReady=true;
					mover->buffFull = false;
				}
				else if(getConnectionType(client)==UN_READYING)
				{
					mover->isReady=false;
					mover->buffFull = false;
				}
			}	
		}
	}
	if(numReadyPlayers == MAX_CLIENTS)
	{
		char message[20];
		float time= (float)clock();
		client->startTime =time+5;
		sprintf(message,"4! %f %f",time,time+5);
		return TIME_SET;
	}
	else
	{
		for(struct client *mover = client; mover->nextClient; mover= mover->nextClient)
		{
			char message[] = "2!";
			sendString(message, mover);
		}
	}

}

void announceServers(struct client *client)
{
	int playPerClient=0;
	for(struct player *mover = &client->player; mover->nextPlayer; mover= mover->nextPlayer)
	{
		playPerClient++;
		mover->isReady=true;
	}
	char message[10];
	sprintf(message,"5! %d",playPerClient);
	sendString(message, client);
}

void setTime(struct client *client)
{
	char message[50];
	recvString(client);
	int temp;
	char temp2;
	if(getConnectionType(client)==TIME) sscanf(message, "%d%c %f %f",&temp,&temp2,&client->mainTime,&client->startTime);
	client->autoEndTime = client->startTime + AUTO_LEN;
	client->teleEndTime = client->startTime + MATCH_LEN;
}
void sendJoyPos(struct player *player)
{
	char message[255];
	for(int x=0; x<6; x++)
	{
		if(player->button[x])player->varBut=player->varBut|(1<<x);
	}
	if(player->button[9])player->varBut=player->varBut|(1<<9);
	if(player->button[10])player->varBut=player->varBut|(1<<10);
	
	sprintf(message, "9!%c,%d,%d,%d,%d,%d,%d,%d,%d,%c",0,player->axis[0],player->axis[1],player->axis[2],player->axis[3],player->axis[4],player->axis[5],player->axis[6],player->axis[7],player->varBut);
	#ifdef DEBUG
	strcpy(message, "9!0,1,2,3,4,5,6,7,8,a");
	#endif
	char *holder = strchr(message, '!');
	holder[1] = strlen(message);
	printf("Sent string: %s Length:%d\n",message,holder[1]);
	sendString(message, player);
}


void sendBeginMatch(struct player *player)
{
	char message[] = "6!";
	sendString(message,player);
}
void sendBeginTeleop(struct player *player)
{
	char message[] = "7!";
	sendString(message,player);
}
void sendEndMatch(struct player *player)
{
	char message[] = "8!";
	sendString(message,player);
}
int createSocket(struct player *play)
{
	play->socket = socket(AF_INET, SOCK_STREAM, 0);
	if (play->socket == -1)
	{
		printf("Player%d Socket creation failed\n", play->id);
		return 0;
	}
	return 1;
}
int createSocket(struct client *cli)
{
	cli->socket = socket(AF_INET, SOCK_STREAM, 0);
	if (cli->socket == -1)
	{
		printf("Client%d Socket creation failed\n", cli->id);
		return 0;
	}
	return 1;
}
int connect(struct player *play)
{
	play->server.sin_addr.s_addr = inet_addr(play->address);
	play->server.sin_family = AF_INET;
	play->server.sin_port = htons( play->port );
	if(connect(play->socket , (struct sockaddr *)&(play->server) , sizeof(play->server)) < 0)return 0;
	else 
	{
		printf("Socket: %d\n",play->socket);
		return 1;
	}
}
int handshake(struct player *play)
{
	char message[]="1!";
	printf("test1\n");
	sendString(message,play);
	recvString(play);
	printf("test2\n");
	play->buffFull=false;
	if (strstr(play->buffer,"lancer"))
	{
		return 1;
	}
	else
	{
		printf("incorrect handshake, receive: %s\n",play->buffer);	
		return 0;
	}

}
