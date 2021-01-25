#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <ctype.h>
#include "main.h"

int main(int argc, char **argv) {
    int iPort = initialize(argc, argv);


    if (iPort == -1) {
        printf("Missing port. Use flag: -port [portNr] to execute\r\n");
    } else {
        connectToServer(iPort);
    }
    return 0;
}

/* VALIDATES FLAGS & INITIALIZES */
int initialize(int argc, char **argv) {
    int i;
    int iPort = -1; //Default value to determine if port has been set

    //Starting at 1 to ignore first arg filename
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-port") == 0) {
            //The flag entered was -port
            if (iPort != -1) {
                //Port already set
                printf("Cannot set port number twice. Using the first argument\r\n");
            } else {
                //Flag is valid and sets port
                //Using the argument next to the flag
                i++;
                //Checking that the argument is valid (a number)

                if (checkIfNumber(argv[i]) == 1) {
                    iPort = atoi(argv[i]);
                    printf("setting port to %i...\r\n", iPort);
                } else {
                    printf("Invalid argument. Port needs to be a valid number\r\n");
                    break;
                }
            }
        } else {
            //Flag is invalid
            printf("Unknown flag. Please check your spelling.\r\n");
            break;
        }
    }
    return iPort;
}

void connectToServer(int iPort) {
    int sock = 0;
    struct sockaddr_in serverAddr = {0};
    int connector;

    /* Create socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("ERROR opening socket");
    } else {
        /* CONNECT to socket */
        /* IPv4 or IPv6 sock struct as argument */
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(iPort);
        serverAddr.sin_addr.s_addr = htonl(0x7F000001); // 127.0.0.1 LONG
        connector = connect(sock, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
        if (connector < 0) {
            perror("ERROR connecting");
        } else {
            /* READ AND WRITE TO SERVER */
            if( handshake(sock) == QUIT) {
                printf("Closing connection\r\n");
            } else {
                printf("Connection stays open\r\n");
                communicate(sock);
            }
        }
        close(sock);
    }
}

int handshake(int sock) {
    int iSize = 256;
    int iMsgSize = 2;
    int iContinueProgram = QUIT;
    char *szBuffer = malloc(sizeof(char) * 256);
    char *szClientMsg = malloc(sizeof(char) * iMsgSize);
    if (szBuffer == NULL || szClientMsg == NULL) {
        printf("error on malloc\r\n");
    } else {

        readFromServer(sock, szBuffer, iSize - 1);

        do {

            if (fgets(szClientMsg, iMsgSize, stdin) != 0) {
                writeToServer(sock, szClientMsg);
                readFromServer(sock, szBuffer, iSize - 1);
            }

        } while (strncmp(szBuffer, "ERROR", 5) == 0);

        if( strncmp(szClientMsg, "Y", 1) == 0 || strncmp(szClientMsg, "y", 1) == 0) {
            iContinueProgram = OK;
        }

        free(szBuffer);
        free(szClientMsg);
    }
    return iContinueProgram;
}
void communicate(int sock) {
    int i = 0;
    char cBuffer = 0;
    int iSize = 256;
    char *szClientMsg = malloc(sizeof(char) * iSize);
    char *szBuffer = malloc(sizeof(char) * iSize);
    if (szBuffer == NULL || szClientMsg == NULL) {
        printf("malloc failed in: communicate()\r\n");
    } else {
        readFromServer(sock, szBuffer, iSize);

        getchar(); //capturing newline from previous enter

        //Gets userinput (Divided with newline)
        do {
            i = 0;
            memset(szClientMsg, 0, sizeof(char) * iSize);
            printf("enter message: ");

            while( (cBuffer = getchar()) != '\n') {
                szClientMsg[i] = cBuffer;
                i++;
            }
            writeToServer(sock, szClientMsg);
        } while( strncmp(szClientMsg, "QUIT", 4) != 0);
    }

    free(szBuffer);
    free(szClientMsg);
}

void readFromServer(int sock, char *szBuffer, int iBuffSize) {
    int i = 0;
    int n = 0;
    int iErrorCode = OK;
    char cBuff = 0;
    if(szBuffer == NULL) {
        printf("szBuffer pointing to NULL\r\n");
    } else {
        memset(szBuffer, 0, sizeof(char) * iBuffSize);

        do {
            n = read(sock, &cBuff, 1);
            if(n < 0) {
                printf("ERROR reading from sock\r\n");
                iErrorCode = ERROR;
                break;
            }
        } while(cBuff != 2);

        if(iErrorCode == OK) {
            do {
                //Reading 1 and 1 char into array
                n = read(sock, &cBuff, 1);
                if (n < 0) {
                    printf("ERROR reading from socket\r\n");
                    perror("ERROR reading from socket");
                    break;
                } else if (cBuff == 3){
                    //Reached end of text
                    break;
                } else {
                    szBuffer[i] = cBuff;
                    i++;
                }
            } while (i < iBuffSize);

            printf("read: %s \r\n", szBuffer);

        }
    }
}

void writeToServer(int sock, char *szMessage) {
    int n;
    char STX = 2;
    char ETX = 3;

    n = write(sock, &STX, 1);
    if(n < 0) {
        printf("ERROR writing to socket\r\n");
    } else {
        n = write(sock, szMessage, strlen(szMessage));
        if(n < 0) {
            printf("ERROR writing to socket\r\n");
        } else {
            n = write(sock, &ETX, 1);
            if(n < 0) {
                printf("ERROR writing to socket\r\n");
            }
        }
    }
}

int checkIfNumber(char *szText) {
    int i = 0;
    int isNumber = 1;
    while (szText[i] != '\0') {
        if (!(isdigit(szText[i]))) {
            isNumber = 0;
        }
        i++;
    }
    return isNumber;
}
