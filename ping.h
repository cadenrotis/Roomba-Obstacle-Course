/*
 * ping.h
 *
 */

#include <stdint.h>
#include <inc/tm4c123gh6pm.h>
#include "timer.h"
#include "lcd.h"
#include "math.h"

void ping_initialize(void);

void send_pulse(void);

float ping_read(void);

void init_timer_interrupts(void);

void TIMER3B_Wrangler(void);
