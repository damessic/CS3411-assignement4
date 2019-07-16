/* David Messick	  		*/
/* June 19, 2019	  		*/
/* CS3411					*/
/* Assignment 4	ccommon.h	*/

#ifndef CCOMMON

#define CCOMMON

#define MAXCLIENTS 16
#define MAXCLIENTNAME 20
#define MSGSIZE 130
#define CONNECTMSG 1
#define TXTMSG 2
#define EXITMSG 3

typedef char clientNameArray_t[MAXCLIENTS][MAXCLIENTNAME];

#endif