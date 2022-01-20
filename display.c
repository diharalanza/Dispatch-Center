#include <unistd.h>
#include <string.h>
#include <X11/Xlib.h>

#define  AREA_RADIUS      30
#define  VEHICLE_RADIUS   10

const int   AREA_COLORS[NUM_CITY_AREAS] = {0xDD5555, 0xFF8800, 0xFFFF00, 0x55DD33, 0x9999FF, 0xCC33FF};
const int   VEHICLE_COLORS[NUM_CITY_AREAS] = {0xFF5555, 0xFF8855, 0xFFFF88, 0x55FF55, 0x6666FF, 0xBB55FF};


//Display-related variables
Display *display;
Window   win;
GC       gc;


// Initialize and open the simulator window with size CITY_WIDTH x CITY_HEIGHT.
void initializeWindow() {
  // Open connection to X server
  display = XOpenDisplay(NULL);

  // Create a simple window, set the title and get the graphics context then
  // make is visible and get ready to draw
  win = XCreateSimpleWindow(display,  RootWindow(display, 0), 0, 0,
			    CITY_WIDTH, CITY_HEIGHT, 0, 0x000000, 0xFFFFFF);
  XStoreName(display, win, "Taxi Dispatch Simulator");
  gc = XCreateGC(display, win, 0, NULL);
  XMapWindow(display, win);
  XFlush(display);
  usleep(20000);  // sleep for 20 milliseconds.
}

// Close the display window
void closeWindow() {
  XFreeGC(display, gc);
  XUnmapWindow(display, win);
  XDestroyWindow(display, win);
  XCloseDisplay(display);
}


// Redraw all the city areas and all the taxis that are on route.
// This code should run in an infinite loop continuously drawing the city.
// Taxis are drawn as circles with radius VEHICLE_RADIUS.
void *showSimulation(void *d) {
  DispatchCenter  *dispatch = d;
  
  // Open the window
  initializeWindow();

  // Now keep redrawing until someone kills the thread
  while(1) {
    // Erase the background 
    XSetForeground(display, gc, 0xFFFFFF);
    XFillRectangle(display, win, gc, 0, 0, CITY_WIDTH, CITY_HEIGHT);

    // Draw all the city areas
    for (int i=0; i<NUM_CITY_AREAS; i++) {
      XSetForeground(display, gc, AREA_COLORS[i]);
      XFillArc(display, win, gc,
	       AREA_X_LOCATIONS[i]-AREA_RADIUS, AREA_Y_LOCATIONS[i]-AREA_RADIUS,
	       2*AREA_RADIUS, 2*AREA_RADIUS, 0, 360*64);
      XSetForeground(display, gc, 0x000000);
      XDrawArc(display, win, gc,
	       AREA_X_LOCATIONS[i]-AREA_RADIUS, AREA_Y_LOCATIONS[i]-AREA_RADIUS,
	       2*AREA_RADIUS, 2*AREA_RADIUS, 0, 360*64);
      if ((i<2)||(i==5))
	XDrawString(display, win, gc, AREA_X_LOCATIONS[i]-AREA_RADIUS+15,
		    AREA_Y_LOCATIONS[i]-AREA_RADIUS-4, AREA_NAMES[i], strlen(AREA_NAMES[i]));
      else
	XDrawString(display, win, gc, AREA_X_LOCATIONS[i]-AREA_RADIUS+15,
		    AREA_Y_LOCATIONS[i]+AREA_RADIUS+12, AREA_NAMES[i], strlen(AREA_NAMES[i]));
    }
    
    // Draw all the taxis 
    for (int i=0; i<dispatch->numTaxis; i++) {
      // Set the color to the dropoffArea color if on way there
      if (dispatch->taxis[i]->status == DROPPING_OFF)
	XSetForeground(display, gc, VEHICLE_COLORS[dispatch->taxis[i]->dropoffArea]);
      else if (dispatch->taxis[i]->status == PICKING_UP)
	XSetForeground(display, gc, 0x333333);
      else 
	XSetForeground(display, gc, 0xFFFFFF);

      XFillArc(display, win, gc,
	       dispatch->taxis[i]->x-VEHICLE_RADIUS,
	       dispatch->taxis[i]->y-VEHICLE_RADIUS,
	       2*VEHICLE_RADIUS, 2*VEHICLE_RADIUS, 0, 360*64);
      XSetForeground(display, gc, 0x000000); // draw a black border
      XDrawArc(display, win, gc,
	       dispatch->taxis[i]->x-VEHICLE_RADIUS,
	       dispatch->taxis[i]->y-VEHICLE_RADIUS,
	       2*VEHICLE_RADIUS, 2*VEHICLE_RADIUS, 0, 360*64);
      char  plate[4];
      sprintf(plate, "%03d", dispatch->taxis[i]->plateNumber);
      XDrawString(display, win, gc,  dispatch->taxis[i]->x-5 + VEHICLE_RADIUS,
		  dispatch->taxis[i]->y+1 - VEHICLE_RADIUS, plate, strlen(plate));
    }
    XFlush(display);
    usleep(2000);
  }
  
  closeWindow();
  pthread_exit(NULL);
}

