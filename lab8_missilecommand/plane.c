#include "plane.h"
#include "config.h"
#include "display.h"
#include "missile.h"
#include "stdbool.h"
#include <math.h>
#include <stdio.h>

// Definitions
#define PRINT_DEBUG_STATEMENTS false
#define X_TIP_OFFSET 16.3  // (21.25/sin(127.5))*sin(37.5)
#define X_BASE_OFFSET 4.22 // (5.5/sin(52.5))*sin(37.5) // 5.5 = 11/2
#define Y_BASE_OFFSET 5.5

#define DOUBLED 2

// Keep track of the plane's center, initialized in plane_init()
uint16_t plane_x_tip;
uint16_t plane_y_tip;
uint16_t plane_x_base;
uint16_t plane_y_bottom;
uint16_t plane_y_top;

missile_t plane;
missile_t *plane_missile;

// Function Declarations
static void updatePosition();
static void updatePlaneCorners();
static void drawPlane();
static void erasePlane();
static display_point_t planeOrigin();
static display_point_t planeDestination();
static double distance(display_point_t origin, display_point_t dest);

// States
typedef enum { INACTIVE, MOVING, DEAD } plane_t;
plane_t currentState;

// Print the given state passed in by the state variable
static void printStateString() {
  // Print the given state
  switch (currentState) {
  case INACTIVE:
    printf("INACTIVE");
    break;
  case MOVING:
    printf("MOVING");
    break;
  case DEAD:
    printf("DEAD");
    break;
  default:
    printf("UNKNOWN_ST");
    break;
  }
}

// Debug function for printing state changes
static void debug() {
  static plane_t previousState = INACTIVE;
  // Print state change when a change occurs
  if (previousState != currentState) {
    printf("\nDEBUG: ");
    printStateString(previousState);
    printf(" -> ");
    printStateString(currentState);

    previousState = currentState;
  }
}

// Initialize the plane state machine
// Pass in a pointer to the missile struct (the plane will only have one missile)
void plane_init(missile_t *_plane_missile) {
  currentState = INACTIVE;

  plane_missile = _plane_missile;

  // Start plane at the edge of the screen
  plane.x_origin = DISPLAY_WIDTH;
  plane.y_origin = rand() % DISPLAY_HEIGHT;

  // Initialize the plane's current possition to that of its starting
  plane.x_current = plane.x_origin;
  plane.y_current = plane.y_origin;
  display_point_t plane_origin = {plane.x_origin, plane.y_origin};

  // Set the plane's destination
  plane.x_dest = 0;
  plane.y_dest = rand() % DISPLAY_HEIGHT;
  display_point_t plane_dest = {plane.x_dest, plane.y_dest};

  plane.total_length = distance(plane_origin, plane_dest);

  plane.explode_me = false;
  plane.impacted = false;
  plane.length = 0;

  missile_init_plane(plane_missile, plane.x_current, plane.y_current);
}

// State machine tick function
void plane_tick() {
  static uint32_t timer = 0;
  // Toggle debug function
  if (PRINT_DEBUG_STATEMENTS)
    debug(currentState);

  // Perform state update first.
  switch (currentState) {

  case INACTIVE:
    currentState = MOVING;
    break;

  case MOVING:
    if (plane.explode_me || plane.x_current <= plane.x_dest) {
      currentState = DEAD;
      erasePlane();
      plane.explode_me = false;
    }
    break;

  case DEAD:
    if (timer >= 100) {
      timer = 0;
      currentState = INACTIVE;
      plane_init(&plane_missile[0]);
    }
   break;

      default:
    break;
  }

  // State action switch statement
  switch (currentState) {

  case MOVING:
    erasePlane();
    updatePosition();
    drawPlane();
    break;

  case DEAD:
    timer += 1;
    break;

  default:
    break;
  }
}

// Trigger the plane to explode
void plane_explode() { plane.explode_me = true; }

// Get the XY location of the plane
display_point_t plane_getXY() { return (display_point_t){plane.x_current, plane.y_current}; }

static void updatePosition() {
  plane.length += CONFIG_PLANE_DISTANCE_PER_TICK * DOUBLED;

  plane.x_current = plane.x_origin + ((double)plane.length / plane.total_length) * (plane.x_dest - plane.x_origin);
  plane.y_current = plane.y_origin + ((double)plane.length / plane.total_length) * (plane.y_dest - plane.y_origin);

  plane.length = (double)distance((display_point_t){plane.x_origin, plane.y_origin},
                                  (display_point_t){plane.x_current, plane.y_current});
  updatePlaneCorners();
}

static void updatePlaneCorners() {
  plane_x_tip = plane.x_current - X_TIP_OFFSET;
  plane_y_tip = plane.y_current;
  plane_x_base = plane.x_current - X_BASE_OFFSET;
  plane_y_bottom = plane.y_current - Y_BASE_OFFSET;
  plane_y_top = plane.y_current + Y_BASE_OFFSET;
}

static void drawPlane() {
  display_fillTriangle(plane_x_tip, plane_y_tip, plane_x_base, plane_y_bottom, plane_x_base, plane_y_top,
                       DISPLAY_WHITE);
}

static void erasePlane() {
  display_fillTriangle(plane_x_tip, plane_y_tip, plane_x_base, plane_y_bottom, plane_x_base, plane_y_top,
                       DISPLAY_BLACK);
}

// Find and return the length/distance between two points
static double distance(display_point_t point1, display_point_t point2) {
  // Calculate the distance between two points
  return sqrt(pow(abs(point1.x - point2.x), 2) + pow(abs(point1.y - point2.y), 2));
}