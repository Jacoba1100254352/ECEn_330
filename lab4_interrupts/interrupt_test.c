#include "interrupts.h"
#include "intervalTimer.h"
#include "leds.h"

#define LED_0 0x01
#define LED_1 0x02
#define LED_2 0x04

#define HZ_10 10
#define HZ_1 1
#define HZ_0_1 0.1

#define CLEAR 0
#define ON 1

// Purpose: Acknowledge and blink LED 0
static void myIsr_0() {
  // Acknowledge timer interrupt
  intervalTimer_ackInterrupt(INTERVAL_TIMER_0);

  // Blink
  leds_write(leds_read() ^ LED_0);
}

// Purpose: Acknowledge and blink LED 1
static void myIsr_1() {
  // Acknowledge timer interrupt
  intervalTimer_ackInterrupt(INTERVAL_TIMER_1);

  // Blink
  leds_write(leds_read() ^ LED_1);
}

// Purpose: Acknowledge and blink LED 2
static void myIsr_2() {
  // Acknowledge timer interrupt
  intervalTimer_ackInterrupt(INTERVAL_TIMER_2);

  // Blink
  leds_write(leds_read() ^ LED_2);
}

/*
This function is a small test application of your interrupt controller.  The
goal is to use the three AXI Interval Timers to generate interrupts at different
rates (10Hz, 1Hz, 0.1Hz), and create interrupt handler functions that change the
LEDs at this rate.  For example, the 1Hz interrupt will flip an LED value each
second, resulting in LED that turns on for 1 second, off for 1 second,
repeatedly.

For each interval timer:
    1. Initialize it as a count down timer with appropriate period (1s, 0.5s,
0.25s)
    2. Enable the timer's interrupt output
    3. Enable the associated interrupt input on the interrupt controller.
    4. Register an appropriate interrupt handler function (isr_timer0,
isr_timer1, isr_timer2)
    5. Start the timer

Make sure you call `interrupts_init()` first!
*/
void interrupt_test_run() {
  // Initialize interrupts and leds
  interrupts_init();
  leds_init(CLEAR);

  // Timer 0
  interrupts_register(INTERVAL_TIMER_0_INTERRUPT_IRQ, myIsr_0);
  interrupts_irq_enable(INTERVAL_TIMER_0_INTERRUPT_IRQ);
  intervalTimer_initCountDown(INTERVAL_TIMER_0, HZ_0_1);
  intervalTimer_enableInterrupt(INTERVAL_TIMER_0);
  intervalTimer_start(INTERVAL_TIMER_0);

  // Timer 1
  interrupts_register(INTERVAL_TIMER_1_INTERRUPT_IRQ, myIsr_1);
  interrupts_irq_enable(INTERVAL_TIMER_1_INTERRUPT_IRQ);
  intervalTimer_initCountDown(INTERVAL_TIMER_1, HZ_1);
  intervalTimer_enableInterrupt(INTERVAL_TIMER_1);
  intervalTimer_start(INTERVAL_TIMER_1);

  // Timer 2
  interrupts_register(INTERVAL_TIMER_2_INTERRUPT_IRQ, myIsr_2);
  interrupts_irq_enable(INTERVAL_TIMER_2_INTERRUPT_IRQ);
  intervalTimer_initCountDown(INTERVAL_TIMER_2, HZ_10);
  intervalTimer_enableInterrupt(INTERVAL_TIMER_2);
  intervalTimer_start(INTERVAL_TIMER_2);

  while (ON)
    ;
}