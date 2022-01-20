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

// This represents a customer request.  There should be two command line arguments representing the
// pickup location and dropoff location.  Each should be a number from 0 to the NUM_CITY_AREAS.
void main(int argc, char *argv[]) {
  int                 clientSocket;  // client socket id
  struct sockaddr_in  clientAddress; // client address
  struct sockaddr_in  serverAddress; // client address
  int                 status, bytesReceived;
  char                buffer[10];    // buffer to send

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

  buffer[0] = REQUEST_TAXI;
  buffer[1] = atoi(argv[1]);
  buffer[2] = atoi(argv[2]);

  //send shutdown command
  send(clientSocket, buffer, sizeof(buffer), 0);

  bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

  //close client
  close(clientSocket);

  //print no taxi to client if such
  if (buffer[0] == NO) {
    printf("Sorry, no available taxis.\n");
  }

}
