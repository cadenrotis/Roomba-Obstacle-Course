/*
 * servo.h
 *
 */

#include <stdint.h>
#include <inc/tm4c123gh6pm.h>
#include "timer.h"
#include "lcd.h"
#include "math.h"

void servo_initialize();

int servo_movement(float degrees);

int calibrate_Servo(float degrees, int mode);

int setServo(char minAngle);

void ourBot_Scan(int degrees);

