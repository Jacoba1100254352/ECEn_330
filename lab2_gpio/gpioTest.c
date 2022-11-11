#include "gpioTest.h"
#include "buttons.h"
#include "display.h"
#include "leds.h"
#include "stdbool.h"
#include "stdio.h"
#include "switches.h"

#define ALL_SWITCHES_HIGH 0xF
#define ALL_BUTTONS_HIGH 0xF

#define X_ORIGIN 0
#define Y_ORIGIN 0
#define BOX_WIDTH DISPLAY_WIDTH / 4
#define BOX_HEIGHT DISPLAY_HEIGHT / 2
#define TEXT_Y BOX_HEIGHT / 2
#define TEXT_X BOX_WIDTH / 5
#define TEXT_SIZE 2
#define RESET_AS_ZERO 0

#define BTN0 "BTN0"
#define BTN1 "BTN1"
#define BTN2 "BTN2"
#define BTN3 "BTN3"

// Runs a test of the buttons. As you push the buttons, graphics and messages
// will be written to the LCD panel. The test will until all 4 pushbuttons are
// simultaneously pressed.
void gpioTest_buttons() {

  // Initialize buttons and display
  buttons_init();

  display_init();
  display_fillRect(X_ORIGIN, Y_ORIGIN, DISPLAY_WIDTH, DISPLAY_HEIGHT,
                   DISPLAY_BLACK);
  display_setTextSize(TEXT_SIZE);

  // Get button status
  uint8_t reading = buttons_read();

  // Keep track of screen display status
  bool display_0_on = false;
  bool display_1_on = false;
  bool display_2_on = false;
  bool display_3_on = false;

  // Get and update buttons status and display until all are lit
  while (reading != ALL_BUTTONS_HIGH) {
    reading = buttons_read();

    // Determine button press status
    bool button_0_pressed = reading & BUTTONS_BTN0_MASK;
    bool button_1_pressed = reading & BUTTONS_BTN1_MASK;
    bool button_2_pressed = reading & BUTTONS_BTN2_MASK;
    bool button_3_pressed = reading & BUTTONS_BTN3_MASK;

    // BTN 0 Draw block for button press or clear for button de-press
    if (!display_0_on && button_0_pressed) {
      // Draw
      display_fillRect(BOX_WIDTH + BOX_WIDTH + BOX_WIDTH, Y_ORIGIN, BOX_WIDTH,
                       BOX_HEIGHT, DISPLAY_YELLOW);
      display_setCursor(BOX_WIDTH + BOX_WIDTH + BOX_WIDTH + TEXT_X, TEXT_Y);
      display_setTextColor(DISPLAY_BLACK);
      display_println(BTN0);
      display_0_on = true;
    } else if (display_0_on && !button_0_pressed) {
      // Erase
      display_fillRect(BOX_WIDTH + BOX_WIDTH + BOX_WIDTH, Y_ORIGIN, BOX_WIDTH,
                       BOX_HEIGHT, DISPLAY_BLACK);
      display_0_on = false;
    }

    // BTN 1 Draw block for button press or clear for button de-press
    if (!display_1_on && button_1_pressed) {
      // Draw
      display_fillRect(BOX_WIDTH + BOX_WIDTH, Y_ORIGIN, BOX_WIDTH, BOX_HEIGHT,
                       DISPLAY_GREEN);
      display_setCursor(BOX_WIDTH + BOX_WIDTH + TEXT_X, TEXT_Y);
      display_setTextColor(DISPLAY_BLACK);
      display_println(BTN1);
      display_1_on = true;
    } else if (display_1_on && !button_1_pressed) {
      // Erase
      display_fillRect(BOX_WIDTH + BOX_WIDTH, Y_ORIGIN, BOX_WIDTH, BOX_HEIGHT,
                       DISPLAY_BLACK);
      display_1_on = false;
    }

    // BTN 2 Draw block for button press or clear for button de-press
    if (!display_2_on && button_2_pressed) {
      // Draw
      display_fillRect(BOX_WIDTH, Y_ORIGIN, BOX_WIDTH, BOX_HEIGHT, DISPLAY_RED);
      display_setCursor(BOX_WIDTH + TEXT_X, TEXT_Y);
      display_setTextColor(DISPLAY_WHITE);
      display_println(BTN2);
      display_2_on = true;
    } else if (display_2_on && !button_2_pressed) {
      // Erase
      display_fillRect(BOX_WIDTH, Y_ORIGIN, BOX_WIDTH, BOX_HEIGHT,
                       DISPLAY_BLACK);
      display_2_on = false;
    }

    // BTN 3 Draw block for button press or clear for button de-press
    if (!display_3_on && button_3_pressed) {
      // Draw
      display_fillRect(X_ORIGIN, Y_ORIGIN, BOX_WIDTH, BOX_HEIGHT, DISPLAY_BLUE);
      display_setCursor(TEXT_X, TEXT_Y);
      display_setTextColor(DISPLAY_WHITE);
      display_println(BTN3);
      display_3_on = true;
    } else if (display_3_on && !button_3_pressed) {
      // Erase
      display_fillRect(X_ORIGIN, Y_ORIGIN, BOX_WIDTH, BOX_HEIGHT,
                       DISPLAY_BLACK);
      display_3_on = false;
    }
  }

  // Clear the whole screen
  display_fillScreen(DISPLAY_BLACK);
}

// Runs a test of the switches. As you slide the switches, LEDs directly above
// the switches will illuminate. The test will run until all switches are slid
// upwards. When all 4 slide switches are slid upward, this function will
// return.
void gpioTest_switches() {
  // Initialize Switches and LEDS
  switches_init();

  leds_init(RESET_AS_ZERO);

  // Get and display switches status until all are lit
  while (switches_read() != ALL_SWITCHES_HIGH)
    leds_write(switches_read());

  // Reset all LEDs to 0
  leds_write(RESET_AS_ZERO);
}