#include "servo.h"

int totalVal = 341504;

//initialize the GPIO and Timer registers needed for the servo
void servo_initialize()
{
    //GPIO registers
    SYSCTL_RCGCGPIO_R |= 0b000010; //enable Port B
    GPIO_PORTB_DIR_R |= 0b00100000; //set PB5 as output
    GPIO_PORTB_DEN_R |= 0b00100000; //enable digital functions for PB5
    GPIO_PORTB_AFSEL_R |= 0b00100000; //enable peripheral signals for PB5
    GPIO_PORTB_PCTL_R |= 0x00700000; //set PB5 to use T1CCP1

    //Timer registers
    SYSCTL_RCGCTIMER_R |= 0b000010; //enable Timer 1
    TIMER1_CTL_R &= 0xFEFF; //disable Timer 1B
    TIMER1_CFG_R |= 0x4; //set Timer 1 to a 16-bit timer
    TIMER1_TBMR_R |= 0x00A; //enable PWM mode and set Timer 1B to periodic mode
    TIMER1_TBMR_R &= 0xFEB; //set Timer 1B to edge-count mode and count down mode
    TIMER1_CTL_R &= 0xBFFF; //have PWM output be unaffected or not inverted
    //TIMER1_TBPR_R |= 0x00FF; //set an 8-bit prescaler
    TIMER1_CTL_R |= 0x0100; //enable Timer 1B

    //Move the servo to 90 degrees
    TIMER1_TBILR_R = 0x3600;
    TIMER1_TBPR_R = 0x05;
    TIMER1_TBMATCHR_R = 0xE200;
    TIMER1_TBPMR_R = 0x04;
}

void ourBot_Scan(int degrees)
{
    timer_init();

    totalVal = 326924 + (degrees * 157);

    TIMER1_TBILR_R = totalVal & 0xFFFF;
    TIMER1_TBPR_R = (totalVal >> 16) & 0xFF;

    TIMER1_TBMATCHR_R = 0xE200;
    TIMER1_TBPMR_R = 0x04;

    timer_waitMillis(250);
}

//move the servo by an inputted amount of degrees
int servo_movement(float degrees)
{
    // 0
//    TIMER1_TBILR_R = 0x1900;
//    TIMER1_TBPR_R = 0x05;
//      TIMER1_TBILR_R = 0xFD0C;
//      TIMER1_TBPR_R = 0x04;
//    TIMER1_TBMATCHR_R = 0xE200;
//    TIMER1_TBPMR_R = 0x04;

    // 180
//    TIMER1_TBILR_R = 0x6D32;
//    TIMER1_TBPR_R = 0x05;
//    TIMER1_TBILR_R = 0x6B70;
//    TIMER1_TBPR_R = 0x05;
//    TIMER1_TBMATCHR_R = 0xE200;
//    TIMER1_TBPMR_R = 0x04;

//    int opVal = degrees * 157;
//    int totalVal = 0;
//
//    if (mode == 0) // Rotate CCW
//    {
//        totalVal += opVal;
//    }
//    else // Rotate CW
//    {
//        totalVal -= opVal;
//    }
//
//    TIMER1_TBILR_R = totalVal & 0xFFFF;
//    TIMER1_TBILR_R = (totalVal >> 16) & 0xFF;
//
//    TIMER1_TBMATCHR_R = 0xE200;
//    TIMER1_TBPMR_R = 0x04;
//
//    return 0;
}

int calibrate_Servo(float degrees, int mode)
{
    timer_init();
    timer_waitMillis(500);

    int opVal = degrees * 157;

    if (mode == 0) // Rotate CCW
    {
        totalVal += opVal;
    }
    else // Rotate CW
    {
        totalVal -= opVal;
    }

    TIMER1_TBILR_R = totalVal & 0xFFFF;
    TIMER1_TBPR_R = (totalVal >> 16) & 0xFF;

    TIMER1_TBMATCHR_R = 0xE200;
    TIMER1_TBPMR_R = 0x04;

    return totalVal;
}

int setServo(char minAngle)
{
    if (minAngle == 0) // Set to 0 degrees
    {
        TIMER1_TBILR_R = 0xFD0C;
        TIMER1_TBPR_R = 0x04;
        TIMER1_TBMATCHR_R = 0xE200;
        TIMER1_TBPMR_R = 0x04;

        totalVal = 326924;
    }
    else  // Set to 180 degrees
    {
        TIMER1_TBILR_R = 0x6B70;
        TIMER1_TBPR_R = 0x05;
        TIMER1_TBMATCHR_R = 0xE200;
        TIMER1_TBPMR_R = 0x04;

        totalVal = 355184;
    }

    return totalVal;
}
