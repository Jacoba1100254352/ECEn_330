#include "gameControl.h"
#include "config.h"
#include "display.h"
#include "missile.h"
#include "touchscreen.h"
#include <stdbool.h>

// Missile initializations
missile_t missiles[CONFIG_MAX_TOTAL_MISSILES];
missile_t *enemy_missiles = &(missiles[0]);
missile_t *player_missiles = &(missiles[CONFIG_MAX_ENEMY_MISSILES]);
missile_t *plane_missiles = &(missiles[CONFIG_MAX_PLANE_MISSILES]);

// Boolean to determine which missiles to tick
bool missile_half;

// Initialize the game control logic
// This function will initialize all missiles, stats, plane, etc.
void gameControl_init() {
  display_fillScreen(DISPLAY_BLACK);

  // Initialize missiles
  for (uint16_t i = 0; i < CONFIG_MAX_TOTAL_MISSILES; i++)
    missile_init_dead(&missiles[i]);

  missile_half = false;
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
        break;
      }

    touchscreen_ack_touch();
  }

  // Loop over only half of the missiles, missileIndex initializes to starting
  // Index
  uint8_t missileIndex = (missile_half) ? 0 : CONFIG_MAX_TOTAL_MISSILES / 2;
  uint8_t endMissileRange = (missile_half) ? CONFIG_MAX_TOTAL_MISSILES / 2 : CONFIG_MAX_TOTAL_MISSILES;
  for (; missileIndex < endMissileRange; missileIndex++)
    missile_tick(&missiles[missileIndex]);
  missile_half = !missile_half;

  // Check if missile i should explode, caused by an exploding missile j
  for (uint8_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES; i++)
    for (uint8_t j = 0; j < CONFIG_MAX_PLAYER_MISSILES; j++) {

      // Skip if the missile is already dead (not flying) or there is no
      // explosion from the player
      if (!missile_is_flying(&enemy_missiles[i]) || !missile_is_exploding(&player_missiles[j]))
        continue;

      // Determine if an enemy is within the radius of the player missile
      if (abs(enemy_missiles[i].x_current - player_missiles[j].x_current) <= player_missiles[j].radius &&
          abs(enemy_missiles[i].y_current - player_missiles[j].y_current) <= player_missiles[j].radius)
        missile_trigger_explosion(&enemy_missiles[i]);
    }
}