#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>


// Initialize the dispatch center server by creating the server socket, setting up the server address,
// binding the server socket and setting up the server to listen for taxi and customer clients.
void initializeDispatchServer(int *serverSocket, struct sockaddr_in  *serverAddress) {

  int status;

  //create the server
  *serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (*serverSocket < 0) {
    printf("DISPATCH CENTER ERROR: could not open socket.\n");
    exit(-1);
  }

  //setup the servers address
  memset(serverAddress, 0, sizeof(*serverAddress));
  serverAddress->sin_family = AF_INET;
  serverAddress->sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddress->sin_port = htons((unsigned short) SERVER_PORT);

  //bind the server socket
  status = bind(*serverSocket, (struct sockaddr *)serverAddress, sizeof(*serverAddress));
  if (status < 0 ) {
    printf("DISPATCH CENTER ERROR: could not bind socket.\n");
    exit(-1);
  }

  //set the server to listen to this many number of connections
  status = listen(*serverSocket, MAX_TAXIS + MAX_REQUESTS);
  if (status < 0) {
    printf("DISPATCH CENTER ERROR: could not listen on socket.\n");
    exit(-1);
  }

}

// Handle client requests coming in through the server socket.  This code should run
// indefinitiely.  It should wait for a client to send a request, process it, and then
// close the client connection and wait for another client.  The requests that may be
// handled are SHUTDOWN (from stop.c), REQUEST_TAXI (from request.c) and UPDATE or
// REQUEST_CUSTOMER (from taxi.c) as follows:

//   SHUTDOWN - causes the dispatch center to go offline.

//   REQUEST_TAXI - contains 2 additional bytes which are the index of the area to be
//                  picked up in and the index of the area to be dropped off in. If
//                  the maximum number of requests has not been reached, a single YES
//                  byte should be sent back, otherwise NO should be sent back.

//   REQUEST_CUSTOMER - contains no additional bytes.  If there are no pending customer
//                      requests, then NO should be sent back.   Otherwise a YES should
//                      be sent back followed by the pickup and dropoff values for the
//                      request that has been waiting the logest in the queue.

//   UPDATE - contains additional bytes representing the taxi's x, y plate, status and
//            dropoff area.  Nothing needs to be sent back to the taxi.

void *handleIncomingRequests(void *d) {
  DispatchCenter  *dispatchCenter = d;

  int                 serverSocket, clientSocket, addressSize, bytesReceived;
  struct sockaddr_in  serverAddress, clientAddress;
  unsigned char       buffer[10], response[10];

  // Initialize the server
  initializeDispatchServer(&serverSocket, &serverAddress);
  dispatchCenter->online = 1;

  // Waiting for clients
  while (dispatchCenter->online == 1) {

    addressSize = sizeof(clientAddress);
    clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &addressSize);
    if (clientSocket < 0) {
      printf("DISPATCH CENTER ERROR: could not accept incoming client connection.\n");
      exit(-1);
    }

    //get the message from the client
    bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

    switch(buffer[0]){

      case SHUTDOWN:

        //acknowledged
        response[0] = YES;
        dispatchCenter->online = 0;
        break;
//==============================================================================

      case REQUEST_TAXI:

        //if max requests is not reached
        if (dispatchCenter->numRequests != MAX_REQUESTS) {
          response[0] = YES;

          //add the request to dc's request buffer
          dispatchCenter->requests[dispatchCenter->numRequests].pickupLocation = buffer[1];
          dispatchCenter->requests[dispatchCenter->numRequests].dropoffLocation = buffer[2];
          //increment number of requests
          dispatchCenter->numRequests++;
        }
        //else send cant do it
        else{
          response[0] = NO;
        }
        break;

//==============================================================================

      case REQUEST_CUSTOMER:

        //if there are requests made
        if (dispatchCenter->numRequests != 0) {

          //set the response
          response[0] = YES;
          response[1] = dispatchCenter->requests[0].pickupLocation;
          response[2] = dispatchCenter->requests[0].dropoffLocation;

          //restructuring requests buffer to remove oldest request
          for (int req = 0; req < dispatchCenter->numRequests-1; req++) {
             dispatchCenter->requests[req] = dispatchCenter->requests[req+1];
          }
          //decrement number of requests
          dispatchCenter->numRequests--;
        }
        //if no requests, send NO
        else{
          response[0] = NO;
        }
        break;
//==============================================================================

      case UPDATE:

        //acknowledged
        response[0] = YES;

        //go thru all taxis in dispatch center until found taxi with the right license plate
        for (int taxi = 0; taxi < dispatchCenter->numTaxis; taxi++) {
          //once found, upadte its data with received data
          if (buffer[1] == dispatchCenter->taxis[taxi]->plateNumber) {

            dispatchCenter->taxis[taxi]->x = (buffer[2] << 8) | buffer[6];
            dispatchCenter->taxis[taxi]->y = (buffer[3] << 8) | buffer[7];
            dispatchCenter->taxis[taxi]->status = buffer[4];
            dispatchCenter->taxis[taxi]->dropoffArea = buffer[5];

            break;
          }
        }
        break;
//==============================================================================

    }

    //send response
    send(clientSocket, response, sizeof(response), 0);

    //close client after doing requested command
    close(clientSocket);

  }

  //close the server if while loop was exited (server goes offline)
  close(serverSocket);

}
