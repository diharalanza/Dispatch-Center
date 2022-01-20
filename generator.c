#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "simulator.h"

void main() {
  // Set up the random seed
  srand(time(NULL));

  char command[30];

  while(1) {

    int pickuploc = (rand()%NUM_CITY_AREAS);
    int dropoffloc = (rand()%NUM_CITY_AREAS);

    sprintf(command,"./customer %d %d &",pickuploc,dropoffloc);

    system(command);

    usleep(250000);   /// do not remove this
  }
}
