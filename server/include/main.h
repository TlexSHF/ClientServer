#ifndef CLIENTSERVER_MAIN_H
#define CLIENTSERVER_MAIN_H

#define OK 0
#define ERROR 1
#define QUIT 1

/* SERVER SIDE APPLICATION */

//Struct for initialization data
typedef struct _INITDATA {
    int iPort;
    char *szID;
} INITDATA;


INITDATA *initialize(int argc, char **argv);
void openToClient(INITDATA *initData);
int handshake(int sock, char *szID);
void communicate(int sock);
void readFromClient(int sock, char *szBuffer, int iBuffSize);
void writeToClient(int sock, char *szMessage);
int checkIfNumber(char *szText);

#endif //CLIENTSERVER_MAIN_H
