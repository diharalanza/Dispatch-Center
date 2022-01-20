#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simulator.h"


void main() {

  int                 clientSocket;  // client socket id
  struct sockaddr_in  clientAddress; // client address
  struct sockaddr_in  serverAddress; // client address
  int                 status;
  char                buffer[10];    // buffer to send

  //command to shutdown
  buffer[0] = SHUTDOWN;

  //creates client
  clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (clientSocket < 0) {
    printf("CLIENT ERROR: could not open socket\n");
    exit(-1);
  }

  //setup the servers address
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
  serverAddress.sin_port = htons((unsigned short) SERVER_PORT);

  //conect to server
  status = connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

  if (status < 0) {
    printf("CLIENT ERROR: could not connect to server (stop)\n");
    exit(-1);
  }

  //send shutdown command
  send(clientSocket, buffer, sizeof(buffer), 0);

  //close client
  close(clientSocket);

}
