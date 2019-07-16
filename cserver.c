/* David Messick	  	*/
/* June 19, 2019	  	*/
/* CS3411		 	  	*/
/* Assignment 4	cserver	*/

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "ccommon.h"

/*Wrapper to print more easily*/
void printWrap(char buf[1024]) {
	write(1, buf, strlen(buf));
}

/*Creates a socket to listen on*/
void createListenSocket(int *sockFD, struct sockaddr_in *serverAddress) {
	socklen_t length;
	char buf[1024];
		
	*sockFD = socket(AF_INET, SOCK_STREAM, 0);
	if (*sockFD == -1) {
		sprintf(buf, "*Error creating listen socket*\n");
		printWrap(buf);
		exit(1);
	}
	bzero((char *) serverAddress, sizeof(serverAddress)); /*Zeros out the serverAddress struct*/
	serverAddress->sin_family = (short) AF_INET; /*Sets the family*/
	serverAddress->sin_addr.s_addr = htonl(INADDR_ANY); /*Allows anyone to connect*/
	serverAddress->sin_port = htons(0); /* bind() will give a unique port.*/
	
	if (bind(*sockFD, (struct sockaddr *)serverAddress, sizeof(struct sockaddr)) == -1) {
		sprintf(buf, "*Unable to bind to socket*\n");
		printWrap(buf);
		exit(1);
	}
	
	if (listen(*sockFD, MAXCLIENTS) == -1) {
		sprintf(buf, "*Error with listen*\n");
		printWrap(buf);
		exit(1);
	}
	
	length = sizeof(struct sockaddr_in); /*Sets length to size of struct serverAddress*/
	getsockname(*sockFD, (struct sockaddr *)serverAddress, &length); /*Gets the socket's name*/
	sprintf(buf, "CSERVER Listening on port number %i\n", ntohs(serverAddress->sin_port));
	printWrap(buf);
}

/*Accepts a connection*/
void acceptConnect(fd_set *goodFDs, int *maxFD, int sockFD, struct sockaddr_in *clientAddress) {
	socklen_t addressLength;
	int newSockFD;
	char buf[1024];
	
	addressLength = sizeof(struct sockaddr_in);
	if((newSockFD = accept(sockFD, (struct sockaddr *)clientAddress, &addressLength)) == -1) {
		sprintf(buf, "*Error with Accept*\n");
		printWrap(buf);
		exit(1);
	}else {
		FD_SET(newSockFD, goodFDs);
		if(newSockFD > *maxFD){
			*maxFD = newSockFD;
		}
		sprintf(buf, "*New connection from %s on port %d ", inet_ntoa(clientAddress->sin_addr), ntohs(clientAddress->sin_port));
		printWrap(buf);
	}
}

/*Sends the message to everyone*/
void sendAll(int destFD, int srcFD, int listenFD, int length, char *writeBuf, fd_set *goodFDs) {
	char buf[1024];
		
	if (FD_ISSET(destFD, goodFDs)){
		if ((destFD != listenFD) && (destFD != srcFD)) {
			if (write(destFD, writeBuf, length) == -1) {
				sprintf(buf, "*Error sending*\n");
				printWrap(buf);
			}
		}
	}
}

/*Does the sending and recieving*/
void doSendRecieve(int fd, fd_set *goodFDs, int listenFD, int maxFD, clientNameArray_t *clientNames) {
	int length, i;
	char readBuf[MSGSIZE + 1];
	char buf[1024];
	
	if ((length = read(fd, &readBuf, MSGSIZE)) <= 0) {
		if (length == 0) {
			sprintf(buf, "*User %s disconnected*\n", *clientNames[fd]);
			printWrap(buf);
		}else {
			sprintf(buf, "*Read error on fd %i*\n", fd);
			printWrap(buf);
		}
		close(fd);
		FD_CLR(fd, goodFDs);
	}
	else if (readBuf[0] == TXTMSG) {
		readBuf[length] = 0; /*Null terminates string*/
		sprintf(buf, "<%s> %s", *clientNames[fd], readBuf);
		printWrap(buf);
		for(i = 0; i <= maxFD; i++){
			sendAll(i, fd, listenFD, strlen(buf), buf, goodFDs);
		}	
	}
	else if (readBuf[0] == CONNECTMSG) {
		readBuf[length] = 0; /*Null terminates string*/
		sprintf(buf, "User %s connected*\n", &readBuf[1]);
		printWrap(buf);
		strncpy(*clientNames[fd], &readBuf[1], strlen(&readBuf[1]) + 1); /*Saves client name*/
	}
}

/*Main*/
int main(int argc, char **argv) {
	fd_set goodFDs;
	fd_set readFDs;
	int maxFD, i;
	int listenFD;
	char buf[1024];
	struct sockaddr_in serverAddress, clientAddress;
	clientNameArray_t clientNames;
	
	FD_ZERO(&goodFDs);
	FD_ZERO(&readFDs);
	createListenSocket(&listenFD, &serverAddress);
	FD_SET(listenFD, &goodFDs);
	
	maxFD = listenFD;
	while (true) {
		readFDs = goodFDs;
		if (select(maxFD + 1, &readFDs, NULL, NULL, NULL) == -1) {
			sprintf(buf, "*Error with Select*\n");
			printWrap(buf);
			exit(1);
		}
		
		for (i = 0; i <= maxFD; i++) {
			if (FD_ISSET(i, &readFDs)) {
				if (i == listenFD) {
					acceptConnect(&goodFDs, &maxFD, listenFD, &clientAddress);
				}
				else
					doSendRecieve(i, &goodFDs, listenFD, maxFD, &clientNames);
			}
		}
	}
	exit(0);
}