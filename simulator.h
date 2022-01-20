#define CITY_WIDTH            720   // The width and height of the city window
#define CITY_HEIGHT           600

#define NUM_CITY_AREAS          6   // Number of main areas of the city
#define MAX_TAXIS             100   // Maximum number of Taxis in the system
#define MAX_REQUESTS          100   // Maximum number of customer requests at any time

#define SERVER_IP     "127.0.0.1"   // IP address of simulator server
#define SERVER_PORT          6000   // PORT of the simulator server

#define UNKNOWN_AREA      -1

const char *AREA_NAMES[NUM_CITY_AREAS] = {"North", "East", "Airport", "South", "Center", "West"};
const short AREA_X_LOCATIONS[6] = {360, 670, 670, 260, 400,  50};
const short AREA_Y_LOCATIONS[6] = { 60, 150, 380, 520, 280, 240};


const char  TIME_ESTIMATES[NUM_CITY_AREAS][NUM_CITY_AREAS] =
  {{10, 20, 40, 40, 20, 20},
   {20, 10, 20, 20, 20, 40},
   {40, 20, 10, 40, 40, 60},
   {40, 20, 40, 10, 20, 20},
   {20, 20, 40, 20, 10, 20},
   {20, 40, 60, 20, 20, 10}};


typedef struct {
  char   pickupLocation;	 // City area to be picked up in (0 to NUM_CTY_AREAS)
  char   dropoffLocation;        // City area to be dropped off in (0 to NUM_CTY_AREAS)
} Request;



typedef struct {
  int      pID;                  // process ID for this taxi
  int      x;                    // x location of the taxi
  int      y;                    // y location of the taxi
  short    plateNumber;          // unique plate id for taxi
  char     status;               // AVAILABLE, PICKING_UP, or DROPPING_OFF
  char     currentArea;          // index of city area currently in
  char     pickupArea;           // index of city area to pick up person in
  char     dropoffArea;          // index of city area to drop off person in
  short    eta;                  // time left until reaching destination
} Taxi;



typedef struct {
  char        online;                 // 0 = no, 1 = yes
  Taxi       *taxis[MAX_TAXIS];       // the taxis belonging to this dispatch center
  int         numTaxis;               // the number of taxis in the dispatch center
  Request     requests[MAX_REQUESTS]; // customer requests
  int         numRequests;            // number of customer requests in the system
} DispatchCenter;


// Command codes that server responds to
#define SHUTDOWN           1   // used to shut down the server
#define REQUEST_TAXI       2   // used when customer wants to see if there is a taxi available
#define REQUEST_CUSTOMER   3   // used when taxi wants to see if there is a customer available
#define UPDATE             4   // used when taxi wants to send a position update

// Command codes sent from server to client
#define YES                5
#define NO                 6

// Status of a taxi
#define AVAILABLE          0
#define PICKING_UP         1
#define DROPPING_OFF       2
