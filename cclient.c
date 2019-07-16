/* David Messick	  	*/
/* June 19, 2019	  	*/
/* CS3411		 	  	*/
/* Assignment 4	cclient	*/

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

/*Does the sending and recieving*/
void doSendRecieve(int serviceFD, int serverFD) {
	char writeBuf[MSGSIZE];
	char readBuf[MSGSIZE];
	char buf[1024];
	int length;
	
	if (serviceFD == 0) { /*If STDIN needs service*/
		length = read(serviceFD, &writeBuf, sizeof(writeBuf));
		writeBuf[length] = 0; /*Null terminates string*/
		if (strcmp(writeBuf , "EXIT\n") == 0) {
			close(serverFD);
			exit(0);
		} 
		else { /*Not "EXIT" msg, send to server*/
			readBuf[0] = TXTMSG; /*Setup text message protocol*/
			readBuf[1] = 0; /*Now properly formatted string*/
			strncat(readBuf, writeBuf, length); /*Add msg to end of readBuf*/
			write(serverFD, &readBuf, length + 1);
		}
	}
	else {  /*Not STDIN, get data and write*/
		length = read(serverFD, &readBuf, sizeof(readBuf));
		readBuf[length] = 0; /*Null terminates string*/
		sprintf(buf, "%s" , readBuf);
		printWrap(buf);
	}
}

/*Creates a listen socket*/
void createListenSocket(int *listenFD, struct sockaddr_in *serverAddress, int portNum, char *hostAddress, char *nickname) {
	char buf[1024];
	
	*listenFD = socket(AF_INET, SOCK_STREAM || SOCK_NONBLOCK, 0);
	if (*listenFD == -1) {
		sprintf(buf, "*Error creating listen socket*\n");
		printWrap(buf);
		exit(1);
	}
	
	bzero((char *) serverAddress, sizeof(serverAddress)); /*Zeros out the serverAddress struct*/
	serverAddress->sin_family = (short) AF_INET;
	serverAddress->sin_addr.s_addr = inet_addr(hostAddress);
	serverAddress->sin_port = htons(portNum);
	
	if(connect(*listenFD, (struct sockaddr *)serverAddress, sizeof(struct sockaddr)) == -1) {
		sprintf(buf, "*Error connecting to server*\n");
		printWrap(buf);
		exit(1);
	}
	
	buf[0] = CONNECTMSG; /*Start building connection msg*/
	buf[1] = 0;
	strncat(buf, nickname, strlen(nickname));
	
	if (write(*listenFD, &buf, strlen(buf)) == -1) {
		sprintf(buf, "*Error writing connection msg*\n");
		printWrap(buf);
		exit(1);
	}
}

/*Main*/	
int main(int argc, char **argv) {
	int serverFD, maxFD, i, portNum;
	char hostAddress[64];
	char nickname[MAXCLIENTNAME];
	char buf[1024];
	struct sockaddr_in serverAddress;
	fd_set goodFDs;
	fd_set readFDs;
	
	if (strlen(argv[1]) <= sizeof(hostAddress))
		strcpy(hostAddress, argv[1]);
	else {
		sprintf(buf, "*Error: host address is too long*\n");
		printWrap(buf);
		exit(1);
	}
	
	portNum = atoi(argv[2]);
	
	if (strlen(argv[3]) <= sizeof(nickname))
		strcpy(nickname, argv[3]);
	else {
		sprintf(buf, "*Error: nickname is too long*\n");
		printWrap(buf);
		exit(1);
	}
	
	createListenSocket(&serverFD, &serverAddress, portNum, hostAddress, nickname);
	FD_ZERO(&goodFDs);
	FD_ZERO(&readFDs);
	FD_SET(0, &goodFDs);
	FD_SET(serverFD, &goodFDs);
	
	maxFD = serverFD;
	while (true) {
		readFDs = goodFDs;
		if(select(maxFD + 1, &readFDs, NULL, NULL, NULL) == -1) {
			sprintf(buf, "*Error with select*\n");
			printWrap(buf);
			exit(1);
		}
		
		for (i = 0; i <= maxFD; i++ ) {
			if (FD_ISSET(i, &readFDs)) {
				doSendRecieve(i, serverFD);
			}
		}
	}
	
	sprintf(buf, "*Exiting...*\n");
	printWrap(buf);
	sleep(1);
	close(serverFD);
	exit(0);
}