#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


// The dispatch center server connection ... when made
int                 clientSocket;  // client socket id
struct sockaddr_in  clientAddress; // client address
struct sockaddr_in  serverAddress; // client address
int                 status, bytesReceived;
char                buffer[10];    // buffer to send


// Set up a client socket and connect to the dispatch center server.  Return -1 if there was an error.
int connectToDispatchCenter(int *sock,  struct sockaddr_in *address) {

  //create client socket
  *sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  //exits if error
  if (*sock < 0) {
    printf("CLIENT ERROR: could not open socket\n");
    exit(-1);
  }

  memset(address, 0, sizeof(*address));
  address->sin_family = AF_INET;
  address->sin_addr.s_addr = inet_addr(SERVER_IP);
  address->sin_port = htons((unsigned short) SERVER_PORT);

  status = connect(*sock, (struct sockaddr *)address, sizeof(*address));

  //if connection was succesful return 0, else return -1
  if (status < 0) {
    return -1;
  }
  else{
    return 0;
  }
}

//helper function to move taxi
void moveTaxi(Taxi* taxi){

  short deltaX, deltaY;

  //if taxi is pcking up a customer
  if (taxi->status == PICKING_UP) {

    deltaX = (AREA_X_LOCATIONS[taxi->pickupArea] - AREA_X_LOCATIONS[taxi->currentArea]) / TIME_ESTIMATES[taxi->currentArea][taxi->pickupArea];;
    deltaY = (AREA_Y_LOCATIONS[taxi->pickupArea] - AREA_Y_LOCATIONS[taxi->currentArea]) / TIME_ESTIMATES[taxi->currentArea][taxi->pickupArea];;

  }
  //if taxi is dropping off a customer
  else{

    deltaX = (AREA_X_LOCATIONS[taxi->dropoffArea] - AREA_X_LOCATIONS[taxi->pickupArea]) / TIME_ESTIMATES[taxi->pickupArea][taxi->dropoffArea];
    deltaY = (AREA_Y_LOCATIONS[taxi->dropoffArea] - AREA_Y_LOCATIONS[taxi->pickupArea]) / TIME_ESTIMATES[taxi->pickupArea][taxi->dropoffArea];

  }

  //increment the coordinates
  taxi->x = taxi->x + deltaX;
  taxi->y = taxi->y + deltaY;

  //decrement the eta by 1 second
  taxi->eta--;

}

// This code runs the taxi forever ... until the process is killed
void runTaxi(Taxi *taxi) {
  // Copy the data over from this Taxi to a local copy
  Taxi   thisTaxi;
  thisTaxi.plateNumber = taxi->plateNumber;
  thisTaxi.currentArea = taxi->currentArea;
  thisTaxi.x = taxi->x;
  thisTaxi.y = taxi->y;
  thisTaxi.status = AVAILABLE;
  thisTaxi.pickupArea = UNKNOWN_AREA;
  thisTaxi.dropoffArea = UNKNOWN_AREA;
  thisTaxi.eta = 0;

  // Go into an infinite loop to request customers from dispatch center when this taxi is available
  // as well as sending location updates to the dispatch center when this taxi is picking up or dropping off.
  while(1) {

    if (thisTaxi.status == AVAILABLE) {

      //connect to dispatch server to request a customer
      if (connectToDispatchCenter(&clientSocket, &serverAddress) == 0){
        //send command to server requesting a customer
        buffer[0] = REQUEST_CUSTOMER;

        send(clientSocket, buffer, sizeof(buffer), 0);

        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        //close client
        close(clientSocket);

        //if server respnds with yes, get this customers info and populate taxis attributes
        if (buffer[0] == YES) {

          //gets pickup and dropoff area
          thisTaxi.pickupArea = buffer[1];
          thisTaxi.dropoffArea = buffer[2];

          //if taxi is in the area of the customer, go straight to dropping them off
          if (buffer[1] == thisTaxi.currentArea) {
            thisTaxi.status = DROPPING_OFF;
            thisTaxi.eta = TIME_ESTIMATES[thisTaxi.pickupArea][thisTaxi.dropoffArea];
          }
          //else, first go pick them up
          else{
            thisTaxi.status = PICKING_UP;
            thisTaxi.eta = TIME_ESTIMATES[thisTaxi.currentArea][thisTaxi.pickupArea];
          }
        }
      }
    }

    //once gotten a request and becoming unavailable, will start sending updates to server
    else{

      //attempt to connect to server again
      if (connectToDispatchCenter(&clientSocket, &serverAddress) == 0){

        //taxi will begin to move and update its attributes
        moveTaxi(&thisTaxi);

        //if the taxi has reached the drop off location
        if (thisTaxi.status == DROPPING_OFF && thisTaxi.eta == 0) {

          //setting coordinates
          thisTaxi.x = AREA_X_LOCATIONS[thisTaxi.dropoffArea];
          thisTaxi.y = AREA_Y_LOCATIONS[thisTaxi.dropoffArea];

          //taxi will become available again
          thisTaxi.status = AVAILABLE;

          //taxis current area will be changed to the area it last dropped off a customer
          thisTaxi.currentArea = thisTaxi.dropoffArea;
          thisTaxi.dropoffArea = UNKNOWN_AREA;
          thisTaxi.pickupArea = UNKNOWN_AREA;

        }
        //if taxi has reached the pick up location
        else if(thisTaxi.status == PICKING_UP && thisTaxi.eta == 0){

          //setting coordinates
          thisTaxi.x = AREA_X_LOCATIONS[thisTaxi.pickupArea];
          thisTaxi.y = AREA_Y_LOCATIONS[thisTaxi.pickupArea];

          //after picking up customer, taxi will go to dropoff mode
          thisTaxi.status = DROPPING_OFF;

          //dispath center wont know the area when taxi is travelling
          thisTaxi.currentArea = UNKNOWN_AREA;

          //change taxis eta to time taken from pickup to dropoff
          thisTaxi.eta = TIME_ESTIMATES[thisTaxi.pickupArea][thisTaxi.dropoffArea];

        }

        //send this info to server to update servers taxis (taxi array in the struct)
        buffer[0] = UPDATE;
        buffer[1] = thisTaxi.plateNumber;
        buffer[2] = thisTaxi.x >> 8;
        buffer[3] = thisTaxi.y >> 8;
        buffer[4] = thisTaxi.status;
        buffer[5] = thisTaxi.dropoffArea;
        buffer[6] = thisTaxi.x & 0xff;
        buffer[7] = thisTaxi.y & 0xff;

        //sending to server
        send(clientSocket, buffer, sizeof(buffer), 0);

        //will receive aknowledgemt msg
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        //close client
        close(clientSocket);
      }
    }
    usleep(50000);  // A delay to slow things down a little
  }
}
