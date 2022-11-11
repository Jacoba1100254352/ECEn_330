#include "clockControl.h"
#include "clockDisplay.h"
#include "touchscreen.h"
#include <display.h>
#include <stdio.h>

#define ADC_COUNTER_MAX 1
#define AUTO_COUNTER_MAX 10
#define RATE_COUNTER_MAX 1

// States for the controller state machine.
enum clockControl_st_t {
  init_st,          // Initialize to/start with this state
  never_touched_st, // Wait til first touch
  waiting_st,       // Wait til next touch
  adc_timer_st,     // Waiting for ADC to settle
  auto_timer_st,    // Wait for the auto-update delay when the button is held
  rate_timer_running_st, // Wait for the rate timer delay
  rate_timer_expired_st, // Starts after the rate_tiemr_running_st
  increment_second_st    // add a second to the clock
};
static enum clockControl_st_t currentState;

static double tick_period;

// Initialize the clock control state machine, with a given period in seconds.
void clockControl_init(double period_seconds) {
  currentState = init_st;
  tick_period = period_seconds;
}

// Print states when changed/set for debugging
static void debugStatePrint() {
  static enum clockControl_st_t previousState;
  static bool firstPass = true;

  // Print message based off of current position and state changes
  if (previousState != currentState || firstPass) {
    firstPass = false;
    previousState = currentState; // track state change

    // Print current state when changed
    switch (currentState) {
    case init_st:
      printf("init_st\n");
      break;
    case never_touched_st:
      printf("never_touched_st\n");
      break;
    case waiting_st:
      printf("waiting for touch_st\n");
      break;
    case adc_timer_st:
      printf("ad_timer_st\n");
      break;
    case auto_timer_st:
      printf("auto_timer_st\n");
      break;
    case rate_timer_running_st:
      printf("rate_timer_running_st\n");
      break;
    case rate_timer_expired_st:
      printf("rate_timer_expired_st\n");
      break;
    }
  }
}

// Tick the clock control state machine
void clockControl_tick() {
  static uint16_t adcCounter = 0;
  static uint16_t autoCounter = 0;
  static uint16_t rateCounter = 0;
  static uint16_t secCounter = 0;

  display_point_t point = touchscreen_get_location();

  debugStatePrint();

  // Transition and Moore output
  switch (currentState) {
  case init_st:
    currentState = never_touched_st;
    break;
  case never_touched_st:
    if (display_isTouched())
      currentState = waiting_st;
    break;
  case waiting_st:
    if (display_isTouched()) {
      display_clearOldTouchData();
      secCounter = 0;
      currentState = adc_timer_st;
    }
    if (secCounter == tick_period) {
      clockDisplay_advanceTimeOneSecond();
      secCounter = 0;
    }
    break;
  case adc_timer_st:
    if (display_isTouched() && adcCounter == ADC_COUNTER_MAX) {
      currentState = auto_timer_st;
    } else if (!display_isTouched() && adcCounter == ADC_COUNTER_MAX) {
      clockDisplay_performIncDec(point);
      currentState = waiting_st;
    }
    break;
  case auto_timer_st:
    if (display_isTouched() && autoCounter == AUTO_COUNTER_MAX) {
      clockDisplay_performIncDec(point);
      currentState = rate_timer_running_st;
    } else if (!display_isTouched()) {
      clockDisplay_performIncDec(point);
      currentState = waiting_st;
    }
    break;
  case rate_timer_running_st:
    if (display_isTouched() && rateCounter == RATE_COUNTER_MAX)
      currentState = rate_timer_expired_st;
    else if (!display_isTouched())
      currentState = waiting_st;
    break;
  case rate_timer_expired_st:
    if (display_isTouched()) {
      clockDisplay_performIncDec(point);
      currentState = rate_timer_running_st;
    } else if (!display_isTouched())
      currentState = waiting_st;
    break;
  default:
    printf("Error with {currentState:%d}", currentState);
    break;
  }

  // Mealy Output
  switch (currentState) {
  case init_st:
    break;
  case never_touched_st:
    break;
  case waiting_st:
    adcCounter = 0;
    autoCounter = 0;
    rateCounter = 0;
    secCounter++;
    break;
  case adc_timer_st:
    adcCounter++;
    break;
  case auto_timer_st:
    autoCounter++;
    break;
  case rate_timer_running_st:
    rateCounter++;
    break;
  case rate_timer_expired_st:
    rateCounter = 0;
    break;
  default:
    printf("Error with {currentState:%d}", currentState);
    break;
  }
}