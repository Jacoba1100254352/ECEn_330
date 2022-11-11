#include <stdio.h>

#include "display.h"

#define TEXT_SIZE 2
#define RIGHT_X 0
#define TOP_Y 0
#define CENTER_X (DISPLAY_WIDTH / 2)
#define CENTER_Y (DISPLAY_HEIGHT / 2)
#define CIRCLE_X (CENTER_X / 2)
#define FILLED_CIRCLE_X (CENTER_X + DISPLAY_WIDTH / 4)
#define TRIANGLE_TIP_Y (DISPLAY_HEIGHT / 2 + 25)
#define FILLED_TRIANGLE_TIP_Y (DISPLAY_HEIGHT / 2 - 25)
#define TRIANGLES_LEFT_X (DISPLAY_WIDTH / 2 - 25)
#define TRIANGLES_RIGHT_X (DISPLAY_WIDTH / 2 + 25)
#define TRIANGLE_BOTTOM_Y (CENTER_Y + DISPLAY_HEIGHT / 3)
#define FILLED_TRIANGLE_BOTTOM_Y (CENTER_Y - DISPLAY_HEIGHT / 3)
#define RADIUS 30

// Print out "hello world" on both the console and the LCD screen.
int main() {

  // Initialize display driver, and fill scren with black
  display_init();
  display_fillScreen(DISPLAY_BLACK); // Blank the screen.

  // Draw the two green lines crossing the screen (as an X)
  display_drawLine(RIGHT_X, TOP_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT,
                   DISPLAY_GREEN);
  display_drawLine(DISPLAY_WIDTH, TOP_Y, RIGHT_X, DISPLAY_HEIGHT,
                   DISPLAY_GREEN);

  // Draw the triangles on either side vertically of the middle
  display_fillTriangle(TRIANGLES_LEFT_X, FILLED_TRIANGLE_BOTTOM_Y,
                       TRIANGLES_RIGHT_X, FILLED_TRIANGLE_BOTTOM_Y, CENTER_X,
                       FILLED_TRIANGLE_TIP_Y, DISPLAY_WHITE);
  display_drawTriangle(TRIANGLES_LEFT_X, TRIANGLE_BOTTOM_Y, TRIANGLES_RIGHT_X,
                       TRIANGLE_BOTTOM_Y, CENTER_X, TRIANGLE_TIP_Y,
                       DISPLAY_YELLOW);

  // Draw the circles on either side horizontally of the middle
  display_drawCircle(CIRCLE_X, CENTER_Y, RADIUS, DISPLAY_RED);
  display_fillCircle(FILLED_CIRCLE_X, CENTER_Y, RADIUS, DISPLAY_RED);

  return 0;
}
