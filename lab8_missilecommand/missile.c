#include "missile.h"
#include "config.h"
#include "display.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Definitions
#define DECISION_TIME 3
#define PRINT_DEBUG_STATEMENTS false

#define LAUNCH_CUTOFF_ONE DISPLAY_WIDTH / 3
#define LAUNCH_CUTOFF_TWO DISPLAY_WIDTH *(2 / 3)

#define LAUNCH_POINT_ONE DISPLAY_WIDTH / 4
#define LAUNCH_POINT_TWO DISPLAY_WIDTH / 2
#define LAUNCH_POINT_THREE (DISPLAY_WIDTH / 4) * 3

#define NUMBER_OF_LOCATIONS 3
#define DOUBLED 2

// Struct for X and Y coordinates
typedef struct {
  uint16_t x;
  uint16_t y;
} missile_location_t;

// States
typedef enum { INACTIVE, MOVING, GROWING, SHRINKING, DEAD } command_t;

// Static Function Declarations
static missile_location_t enemyOrigin();
static missile_location_t enemyDestination();
static uint16_t getMissileColor(missile_t *);
static uint16_t closestFiringLocation(missile_t *);
static double distance(missile_location_t, missile_location_t);
static void init_missiles(missile_t *, missile_location_t, missile_location_t);
static void updateLength(missile_t *);
static void drawMissile(missile_t *);
static void eraseMissile(missile_t *);
static void increaseRadius(missile_t *);
static void decreaseRadius(missile_t *);
static void drawCircle(missile_t *);
static void eraseCircle(missile_t *);

// Print the given state passed in by the state variable
static void printStateString(command_t currentState) {
  // Print the given state
  switch (currentState) {
  case INACTIVE:
    printf("INACTIVE");
    break;
  case MOVING:
    printf("MOVING");
    break;
  case GROWING:
    printf("GROWING");
    break;
  case SHRINKING:
    printf("SHRINKING");
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
static void debug(command_t currentState) {
  static command_t previousState = INACTIVE;
  // Print state change when a change occurs
  if (previousState != currentState) {
    printf("\nDEBUG: ");
    printStateString(previousState);
    printf(" -> ");
    printStateString(currentState);

    previousState = currentState;
  }
}

////////// State Machine INIT Functions //////////
// Unlike most state machines that have a single `init` function, our missile
// will have different initializers depending on the missile type.

// Initialize the missile as a dead missile.  This is useful at the start of the
// game to ensure that player and plane missiles aren't moving before they
// should.
void missile_init_dead(missile_t *missile) { missile->currentState = DEAD; }

// Initialize the missile as an enemy missile.  This will randomly choose the
// origin and destination of the missile.  The origin should be somewhere near
// the top of the screen, and the destination should be the very bottom of the
// screen.
void missile_init_enemy(missile_t *missile) {
  missile->type = MISSILE_TYPE_ENEMY;

  missile_location_t origin = enemyOrigin();
  missile->x_origin = origin.x;
  missile->y_origin = origin.y;

  missile_location_t dest = enemyDestination();
  missile->x_dest = dest.x;
  missile->y_dest = dest.y;

  init_missiles(missile, origin, dest);
}

// Initialize the missile as a player missile.  This function takes an (x, y)
// destination of the missile (where the user touched on the touchscreen).  The
// origin should be the closest "firing location" to the destination (there are
// three firing locations evenly spaced along the bottom of the screen).
void missile_init_player(missile_t *missile, uint16_t x_dest, uint16_t y_dest) {
  missile->type = MISSILE_TYPE_PLAYER;

  missile->x_dest = x_dest;
  missile->y_dest = y_dest;

  missile->x_origin = (x_dest <= LAUNCH_CUTOFF_ONE)   ? LAUNCH_POINT_ONE
                      : (x_dest <= LAUNCH_CUTOFF_TWO) ? LAUNCH_POINT_TWO
                                                      : LAUNCH_POINT_THREE;
  missile->y_origin = DISPLAY_HEIGHT;

  missile_location_t dest = {missile->x_dest, missile->y_dest};
  missile_location_t origin = {missile->x_origin, missile->y_origin};

  init_missiles(missile, origin, dest);
}

// Helper function to lower code lines and perform common missile
// initializations
static void init_missiles(missile_t *missile, missile_location_t origin,
                          missile_location_t dest) {
  missile->currentState = INACTIVE;

  missile->total_length = distance(origin, dest);

  missile->explode_me = false;
  missile->impacted = false;
  missile->length = 0;
  missile->radius = 0;

  missile->x_current = origin.x;
  missile->y_current = origin.y;
}

// Initialize the missile as a plane missile.  This function takes an (x, y)
// location of the plane which will be used as the origin.  The destination can
// be randomly chosen along the bottom of the screen.
void missile_init_plane(missile_t *missile, int16_t plane_x, int16_t plane_y) {
  missile->type = MISSILE_TYPE_PLANE;

  missile->x_origin = plane_x;
  missile->y_origin = plane_y;

  missile->x_dest = rand() * DISPLAY_WIDTH;
  missile->y_dest = DISPLAY_HEIGHT;

  missile_location_t dest = {missile->x_dest, missile->y_dest};
  missile_location_t origin = {missile->x_origin, missile->y_origin};

  init_missiles(missile, origin, dest);
}

////////// State Machine TICK Function //////////
void missile_tick(missile_t *missile) {
  // Toggle debug function
  if (PRINT_DEBUG_STATEMENTS)
    debug(missile->currentState);

  // Perform state update first.
  switch (missile->currentState) {

  case INACTIVE:
    missile->currentState = MOVING;
    break;

  case MOVING:
    if (missile->explode_me) {
      missile->currentState = GROWING;
      missile->explode_me = false;
      eraseMissile(missile);
    }

    // Erase and set missile as inactive when finished moving
    else if (missile->length >= missile->total_length) {
      if (missile->type == MISSILE_TYPE_PLAYER) {
        missile->currentState = GROWING;
        // missile_trigger_explosion(missile);
      } else
        missile->currentState = DEAD;
      eraseMissile(missile);
    }
    break;

  case GROWING:
    // Change state to shrinking when max size of explosion is reached
    if (missile->radius >= CONFIG_EXPLOSION_MAX_RADIUS)
      missile->currentState = SHRINKING;
    break;

  case SHRINKING:
    // Change state to Inactive when done shrinking
    if (missile->radius <= 0) {
      missile->currentState = DEAD;
      eraseCircle(missile);
      eraseMissile(missile);
    }
    break;

  default:
    break;
  }

  // State action switch statement
  switch (missile->currentState) {

  case MOVING:
    eraseMissile(missile);
    updateLength(missile);
    drawMissile(missile);
    break;

  case GROWING:
    eraseMissile(missile);
    increaseRadius(missile);
    drawCircle(missile);
    break;

  case SHRINKING:
    eraseCircle(missile);
    decreaseRadius(missile);
    drawCircle(missile);
    break;

  default:
    break;
  }
}

// Return whether the given missile is dead.
bool missile_is_dead(missile_t *missile) {
  return (missile->currentState == DEAD);
}

// Return whether the given missile is exploding.  This is needed when detecting
// whether a missile hits another exploding missile.
bool missile_is_exploding(missile_t *missile) {
  return (missile->currentState == GROWING ||
          missile->currentState == SHRINKING);
}

// Return whether the given missile is flying.
bool missile_is_flying(missile_t *missile) {
  return (missile->currentState == MOVING);
}

// Used to indicate that a flying missile should be detonated.  This occurs when
// an enemy or plane missile is located within an explosion zone.
void missile_trigger_explosion(missile_t *missile) {
  missile->explode_me = true;
}

// Calculate a random enemy Origin
static missile_location_t enemyOrigin() {
  missile_location_t returnLocation;

  returnLocation.x = rand() % DISPLAY_WIDTH;
  returnLocation.y = 0; // rand() % DISPLAY_HEIGHT;

  return returnLocation;
}

// Calculate a random enemy Destination
static missile_location_t enemyDestination() {
  missile_location_t returnLocation;

  returnLocation.x = rand() % DISPLAY_WIDTH;
  returnLocation.y = DISPLAY_HEIGHT; // rand() % DISPLAY_HEIGHT;

  return returnLocation;
}

// Find and return the length/distance between two points
static double distance(missile_location_t point1, missile_location_t point2) {
  // Calculate the distance between two points
  return sqrt(pow(abs(point1.x - point2.x), 2) +
              pow(abs(point1.y - point2.y), 2));
}

// Draw the missile
static void drawMissile(missile_t *missile) {
  uint32_t color = getMissileColor(missile);

  display_drawLine(missile->x_origin, missile->y_origin, missile->x_current,
                   missile->y_current, color);
}

// Erase the mimssile
static void eraseMissile(missile_t *missile) {
  display_drawLine(missile->x_origin, missile->y_origin, missile->x_current,
                   missile->y_current, DISPLAY_BLACK);
}

// Update the missile length and increase it depending on type
static void updateLength(missile_t *missile) {

  switch (missile->type) {
  case MISSILE_TYPE_ENEMY:
    missile->length += CONFIG_ENEMY_MISSILE_DISTANCE_PER_TICK * DOUBLED;
    break;
  case MISSILE_TYPE_PLANE:
    missile->length += CONFIG_PLANE_DISTANCE_PER_TICK * DOUBLED;
    break;
  case MISSILE_TYPE_PLAYER:
    missile->length += CONFIG_PLAYER_MISSILE_DISTANCE_PER_TICK * DOUBLED;
    break;
  }

  missile->x_current =
      missile->x_origin + ((double)missile->length / missile->total_length) *
                              (missile->x_dest - missile->x_origin);
  missile->y_current =
      missile->y_origin + ((double)missile->length / missile->total_length) *
                              (missile->y_dest - missile->y_origin);
  missile_location_t pointOrigin = {missile->x_origin, missile->y_origin};
  missile_location_t pointCurrent = {missile->x_current, missile->y_current};
  missile->length = (double)distance(pointOrigin, pointCurrent);
}

// Select missile color
static uint16_t getMissileColor(missile_t *missile) {
  switch (missile->type) {
  case MISSILE_TYPE_ENEMY:
    return CONFIG_COLOR_ENEMY;
    break;
  case MISSILE_TYPE_PLAYER:
    return CONFIG_COLOR_PLAYER;
    break;
  case MISSILE_TYPE_PLANE:
    return CONFIG_COLOR_PLANE;
    break;
  }
}

// Erase the circle for the explosions
static void eraseCircle(missile_t *missile) {
  display_fillCircle(missile->x_current, missile->y_current, missile->radius,
                     DISPLAY_BLACK);
}

// Increase the missile's radius
static void increaseRadius(missile_t *missile) {
  missile->radius += CONFIG_EXPLOSION_RADIUS_CHANGE_PER_TICK * DOUBLED;
}

// Decrease the missile's radius
static void decreaseRadius(missile_t *missile) {
  missile->radius -= CONFIG_EXPLOSION_RADIUS_CHANGE_PER_TICK * DOUBLED;
}

// Draw the circle
static void drawCircle(missile_t *missile) {
  uint32_t color = getMissileColor(missile);

  display_fillCircle(missile->x_current, missile->y_current, missile->radius,
                     color);
}