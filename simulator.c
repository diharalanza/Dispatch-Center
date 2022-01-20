#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "simulator.h"

#include "taxi.c"
#include "display.c"
#include "dispatchCenter.c"

#include <time.h>

int main() {
  DispatchCenter     ottawaDispatch;
  pthread_t          request, display;

  srand(time(NULL));
  ottawaDispatch.numTaxis = 10;
  ottawaDispatch.numRequests = 0;

  for (int i = 0; i < ottawaDispatch.numTaxis; i++) {

    // Create a taxi in a random area of the city.
    Taxi* currTaxi = malloc(sizeof(Taxi));

    //add taxi to dispatch center
    ottawaDispatch.taxis[i] = currTaxi;

    // ---initialize this taxi---

    //currTaxi->currentArea = (rand()%NUM_CITY_AREAS);
    currTaxi->currentArea = (rand()%NUM_CITY_AREAS);
    currTaxi->x = AREA_X_LOCATIONS[currTaxi->currentArea];
    currTaxi->y = AREA_Y_LOCATIONS[currTaxi->currentArea];

    currTaxi->pickupArea = UNKNOWN_AREA;
    currTaxi->dropoffArea = UNKNOWN_AREA;

    currTaxi->plateNumber = i;

    currTaxi->status = AVAILABLE;

    //create a child process for this taxi
    int childPID = fork();
    //checks if the process running  is the child
    if (childPID == 0) {
      runTaxi(currTaxi);
      exit(0);
    }
    else{
      currTaxi->pID = childPID;
    }

  }

  // Spawn a thread to handle incoming requests
  pthread_create(&request, NULL, handleIncomingRequests, &ottawaDispatch);

  // Spawn a thread to handle display
  pthread_create(&display, NULL, showSimulation, &ottawaDispatch);

  // Wait for the dispatch center thread to complete, from a STOP command
  pthread_join(request, NULL);

  // Kill all the taxi processes
  for (int j = 0; j < ottawaDispatch.numTaxis; j++) {
    kill(ottawaDispatch.taxis[j]->pID, SIGKILL);
  }

  // Free the memory
  for (int k = 0; k < ottawaDispatch.numTaxis; k++) {
    free(ottawaDispatch.taxis[k]);
  }

  printf("Simulation complete.\n");
}
