#include "gameControl.h"
#include "config.h"
#include "display.h"
#include "missile.h"
#include "plane.h"
#include "touchscreen.h"
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

// Definitions
#define HALF 2

// Function Declarations
static void writeShotsStat();
static void writeImpactedStat();

// Missile initializations
missile_t missiles[CONFIG_MAX_TOTAL_MISSILES];
missile_t *enemy_missiles = &(missiles[0]);
missile_t *player_missiles = &(missiles[CONFIG_MAX_ENEMY_MISSILES]);
missile_t *plane_missiles = &(missiles[CONFIG_MAX_PLANE_MISSILES]);

// Stats
uint16_t impacted;
uint16_t shot;

// Boolean to determine which missiles to tick
bool missile_half;

// Initialize the game control logic
// This function will initialize all missiles, stats, plane, etc.
void gameControl_init() {
  // Randomize
  srand(time(0));

  // Clear the screen
  display_fillScreen(DISPLAY_BLACK);

  // Initialize the plane
  plane_init(&plane_missiles[0]);

  // Initialize missiles
  for (uint16_t i = 0; i < CONFIG_MAX_TOTAL_MISSILES; i++)
    missile_init_dead(&missiles[i]);

  missile_half = false;

  // Initialize stats text on the screen
  writeShotsStat();
  writeImpactedStat();

  // Initialize the stats
  impacted = 0;
  shot = 0;
}

// Tick the game control logic
//
// This function should tick the missiles, handle screen touches, collisions,
// and updating statistics.
void gameControl_tick() {

  // Check for dead enemy missiles and re-initialize
  for (uint16_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES; i++)
    if (missile_is_dead(&enemy_missiles[i]))
      missile_init_enemy(&enemy_missiles[i]);

  // Check for dead plane missiles and re-initialize
  if (missile_is_dead(&plane_missiles[0]))
    missile_init_plane(&plane_missiles[0], rand() % DISPLAY_WIDTH, rand() % DISPLAY_HEIGHT);

  // Initialize Player missiles and shoot if the screen is pressed
  if (touchscreen_get_status() == TOUCHSCREEN_RELEASED) {
    display_point_t coor = touchscreen_get_location();
    for (uint16_t i = 0; i < CONFIG_MAX_PLAYER_MISSILES; i++)
      if (missile_is_dead(&player_missiles[i])) {
        missile_init_player(&player_missiles[i], coor.x, coor.y);
        shot += 1; // Update the shot stat
        break;
      }

    touchscreen_ack_touch();
  }

  // Loop over only half of the missiles, missileIndex initializes to starting
  // Index
  uint8_t missileIndex = (missile_half) ? 0 : CONFIG_MAX_TOTAL_MISSILES / HALF;
  uint8_t endMissileRange = missileIndex + CONFIG_MAX_TOTAL_MISSILES / HALF;
  for (; missileIndex < endMissileRange; missileIndex++)
    missile_tick(&missiles[missileIndex]);
  missile_half = !missile_half;

  // Tick the plane
  plane_tick();

  // Check if missile i should explode, caused by an exploding missile j
  for (uint8_t i = 0; i < CONFIG_MAX_TOTAL_MISSILES; i++) {
    for (uint8_t j = 0; j < CONFIG_MAX_ENEMY_MISSILES; j++) {

      // Skip if the missile is already dead (not flying) or there is no
      // explosion from the player
      if (!missile_is_flying(&missiles[j]) || !missile_is_exploding(&missiles[i]))
        continue;

      // Determine if an enemy is within the radius of the player missile
      if (abs(missiles[i].x_current - missiles[j].x_current) <= missiles[i].radius &&
          abs(missiles[i].y_current - missiles[j].y_current) <= missiles[i].radius)
        missile_trigger_explosion(&missiles[j]);
    }

    // Explode plane if it is within radius
    if (missiles[i].type == MISSILE_TYPE_PLAYER) {
      display_point_t plane_location = plane_getXY();
      if (abs(missiles[i].x_current - plane_location.x) <= missiles[i].radius &&
          abs(missiles[i].y_current - plane_location.y) <= missiles[i].radius)
        plane_explode();
    } else if (missiles[i].impacted) { // Update the impacted stat
      missiles[i].impacted = false;
      impacted += 1;
    }
  }

  ///   Display the stats   ///

  // Shots
  static uint16_t previousShot = 0;
  if (previousShot < shot) {
    writeShotsStat();
    previousShot = shot;
  }

  // Impacted
  static uint16_t previousImpacted = 0;
  if (previousImpacted < impacted) {
    writeImpactedStat();
    previousImpacted = impacted;
  }
}

static void writeShotsStat() {
  static char shotsOutput[20];

  // Set text location
  display_setCursor(10, 10);

  // Clear previous drawing
  display_setTextColor(DISPLAY_BLACK);
  display_println(shotsOutput);

  // Update text
  sprintf(shotsOutput, "Shots: %d", shot);

  // Reset text location
  display_setCursor(10, 10);

  // Write new text
  display_setTextColor(DISPLAY_WHITE);
  display_println(shotsOutput);
}

static void writeImpactedStat() {
  static char impactedOutput[20];

  // Set text location
  display_setCursor(DISPLAY_WIDTH / 2, 10);

  // Clear previous drawing
  display_setTextColor(DISPLAY_BLACK);
  display_println(impactedOutput);

  // Update text
  sprintf(impactedOutput, "Impacted: %d", impacted);

  // Reset text location
  display_setCursor(DISPLAY_WIDTH / 2, 10);

  // Write new text
  display_setTextColor(DISPLAY_WHITE);
  display_println(impactedOutput);
}