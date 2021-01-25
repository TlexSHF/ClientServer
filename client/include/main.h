#ifndef CLIENTSERVER_MAIN_H
#define CLIENTSERVER_MAIN_H

#define OK 0
#define ERROR 1
#define QUIT 1

/* CLIENT SIDE APPLICATION */

int initialize(int argc, char **argv);
void connectToServer(int iPort);
int handshake(int sock);
void communicate(int sock);
void readFromServer(int sock, char *szBuffer, int iBuffSize);
void writeToServer(int sock, char *szMessage);
int checkIfNumber(char *szText);

#endif //CLIENTSERVER_MAIN_H
