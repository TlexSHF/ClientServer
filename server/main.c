#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <ctype.h>
#include "main.h"

int main(int argc, char **argv) {

    INITDATA *initData = initialize(argc, argv);

    if (initData == NULL) {
        printf("program could not continue...\r\n");
    } else {
        if (initData->iPort == -1 || initData->szID == 0) {
            //Program will stop
            printf("Missing port or ID. Use flags: -port [portNr] and -ID [id] to execute\r\n");
        } else {
            openToClient(initData);
        }
    }
    return 0;
}

/* VALIDATES FLAGS & INITIALIZES */
INITDATA *initialize(int argc, char **argv) {
    int i;
    int iPort = -1; //Default value to determine if port has been set
    char *szID = 0;
    INITDATA *initData = malloc(sizeof(INITDATA));
    if (initData == NULL) {
        printf("malloc failed\r\n");
    } else {
        memset(initData, 0, sizeof(INITDATA));

        //Starting at 1 to ignore first arg filename
        for (i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-port") == 0) {
                //The flag entered was -port
                if (iPort != -1) {
                    //Port already set
                    printf("Cannot set port number twice. Using the first argument\r\n");
                } else {
                    //Flag is valid and sets port, using the argument next to the flag
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
            } else if (strcmp(argv[i], "-ID") == 0) {
                //The flag entered was -ID
                if (szID != 0) {
                    //ID already set
                    printf("Cannot set ID twice. Using the first argument\r\n");
                } else {
                    //Flag is valid and sets ID
                    //Using the argument next to the flag
                    i++;

                    szID = argv[i];
                    printf("setting ID to %s...\r\n", szID);
                }
            } else {
                //Flag is invalid
                printf("Unknown flag. Please check your spelling.\r\n");
                break;
            }
        }
        initData->iPort = iPort;
        initData->szID = szID;
    }

    return initData;
}
/* MAIN LOGIC */
void openToClient(INITDATA *initData) {

    int sock = 0;
    int binder = 0;
    int newsock = 0;
    struct sockaddr_in serverAddr = {0};
    int addrLen = sizeof(serverAddr);

    /* CREATE SOCKET */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("ERROR opening socket");
        perror("socket");
    } else {

        /* BINDING SOCKET */
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(initData->iPort);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        binder = bind(sock, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
        if (binder < 0) {
            printf("ERROR on binding");
            perror("bind");
        } else {

            /* LISTEN ON SOCKET */
            if (listen(sock, 5) < 0) {
                printf("ERROR on listening");
                perror("listen");
            } else {

                /* ACCEPT THE CLIENTS CONNECTED CALL */
                newsock = accept(sock, (struct sockaddr *) &serverAddr, (socklen_t * ) & addrLen);
                if (newsock < 0) {
                    printf("ERROR on accept");
                    perror("accept");
                } else {

                    /* READ AND WRITE TO SERVER */
                    if (handshake(newsock, initData->szID) == QUIT) {
                        printf("Closing connection\r\n");
                    } else {
                        printf("Connection stays open\r\n");
                        communicate(newsock);
                    }
                    close(newsock);
                }
            }
        }
        close(sock);
    }
}
int handshake(int sock, char *szID) {
    int iSize = 256;
    int iErrorCode = OK;
    int iContinueProgram = QUIT;
    char *szMsgPart1 = "You are trying to connect to server [ ";
    char *szMsgPart2 = " ]. Do you wish to continue? [Y/N]\r\n";
    char *szMsg = malloc(sizeof(char) * iSize);
    char *szBuffer = malloc(sizeof(char) * iSize);
    if (szBuffer == NULL || szMsg == NULL) {
        printf("malloc failed in: handshake()\r\n");
        iErrorCode = ERROR;
    } else {

        strcat(szMsg, szMsgPart1);
        strcat(szMsg, szID);
        strcat(szMsg, szMsgPart2);

        do {
            writeToClient(sock, szMsg);
            readFromClient(sock, szBuffer, iSize - 1);

            if (strncmp(szBuffer, "Y", 1) == 0 || strncmp(szBuffer, "y", 1) == 0) {
                printf("client wants to continue \r\n");
                szMsg = "OK\r\n";
                writeToClient(sock, szMsg);
                iErrorCode = OK;
                iContinueProgram = OK;
            } else if (strncmp(szBuffer, "N", 1) == 0 || strncmp(szBuffer, "n", 1) == 0) {
                printf("client wants to quit\r\n");
                writeToClient(sock, "OK");
                iErrorCode = OK;
            } else {
                printf("user input unexpected\r\n");
                writeToClient(sock, "ERROR");
                iErrorCode = ERROR;
            }
        } while (iErrorCode == ERROR);
        free(szBuffer);
    }

    return iContinueProgram;
}
void communicate(int sock) {

    int readSize;
    int iSize = 256;
    char *szInitMsg = "You can now write me as many lovely messages as you like. \r\n"
                      "When you do not wish to write to me anymore, please write me these letters: QUIT\r\n"
                      "And i will know...";
    char *szBuffer = malloc(sizeof(char) * iSize);
    if(szBuffer == NULL) {
        printf("malloc failed in: communicate()\r\n");
    } else {
        writeToClient(sock, szInitMsg);

        while(strncmp(szBuffer, "QUIT", 4) != 0) {
            readFromClient(sock, szBuffer, iSize - 1);
        }

    }

    printf("program ended\r\n");
    free(szBuffer);
}
void readFromClient(int sock, char *szBuffer, int iBuffSize) {
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
void writeToClient(int sock, char *szMessage) {
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
    if(szText == NULL) {
        isNumber == 0;
    } else {
        while(szText[i] != '\0') {
            if(!(isdigit(szText[i]))) {
                isNumber = 0;
            }
            i++;
        }
    }
    return isNumber;
}