#include "ping.h"
#include <driverlib/interrupt.h>

volatile enum
{
    LOW, HIGH, DONE
} state; // set by ISR
volatile unsigned int rising_time = 0; //Pulse start time: Set by ISR
volatile unsigned int falling_time = 0; //Pulse end time: Set by ISR

char professionallyNamedPulseCheck = 0;
int callPulse = 0;

//initialize the ping sensor by utilizing Port B pin 3 (PB3)
void ping_initialize(void)
{
    SYSCTL_RCGCTIMER_R |= 0b001000; //enable Timer 3
    TIMER3_CFG_R |= 0x4; //select the 16-bit timer configuration
    TIMER3_TBMR_R |= 0b000000000111; //set timer to capture mode and edge-time mode
    TIMER3_TBMR_R &= 0b111111100111; //set timer to count down
    TIMER3_CTL_R |= 0b000110100000000; //enable timer b and have it detect positive and negative edges
    TIMER3_TBPR_R |= 0x00FF; //write the prescaler value
    TIMER3_TBILR_R |= 0xFFFF; //load the start count value for the timer
    TIMER3_IMR_R |= 0b010000000000; //enable capture mode interrupt
    TIMER3_CTL_R |= 0b000000100000000; //have the timer start counting

    state = HIGH;
    callPulse = 0;
}

void send_pulse(void)
{
    timer_init();

    //state = HIGH;
    professionallyNamedPulseCheck = 0;

    SYSCTL_RCGCGPIO_R |= 0b000010; //enable Port B
    GPIO_PORTB_DIR_R |= 0b00001000; //set PB3 as output
    GPIO_PORTB_DEN_R |= 0b00001000; //enable digital functions for PB3
    GPIO_PORTB_AFSEL_R &= 0b11110111; //disable alternative functions

    GPIO_PORTB_DATA_R &= 0x00; //set PB3 to low
    timer_waitMicros(10); //wait 10 microseconds before changing PB3

    GPIO_PORTB_DATA_R |= 0xFF; //set PB3 to high
    timer_waitMicros(10); //wait 10 microseconds before changing PB3

    GPIO_PORTB_DATA_R &= 0x00; //set PB3 to low
    //timer_waitMicros(10); //wait 10 microseconds before changing PB3

    GPIO_PORTB_DIR_R &= 0b11110111; //set PB3 as input
    GPIO_PORTB_AFSEL_R |= ~0b11110111; //enable alternative functions
    GPIO_PORTB_PCTL_R |= 0x00007000; //set PB3 to alternative function T3CCP1
}

//read the echo pulse that's emitted from the sensor to determine the time (pulse width between the rising and falling edges)
float ping_read(void)
{
    lcd_init();
    float distance = 0;

    if (callPulse == 0)
    {
        TIMER3_IMR_R &= 0b101111111111; //mask capture mode interrupt

        send_pulse();

        TIMER3_ICR_R |= 0x00400;  //clear the capture event interrupt
        TIMER3_IMR_R |= 0b010000000000; //unmask capture mode interrupt

        callPulse++;
    }

    if (state == DONE)
    {
        //calculate the width of the pulse in clock cycles
        unsigned int pulse_width = rising_time - falling_time;

        int timerOverflow = 0;
        if(pulse_width < 0)
        {
            timerOverflow++;
        }

        //calculate distance in cm
        distance = (34000 * (float) pulse_width * (0.0625 * pow(10, -6))) / 2;

        //lcd_printf("PW (CC): %d\nPW (ms): %2.2f\nDis (cm): %2.2f\nTimer Overflow: %d", pulse_width, ((float)pulse_width / (float)16000), distance, timerOverflow);

        state = HIGH;

        TIMER3_IMR_R &= 0b101111111111; //mask capture mode interrupt

        send_pulse();

        TIMER3_ICR_R |= 0x00400;  //clear the capture event interrupt
        TIMER3_IMR_R |= 0b010000000000; //unmask capture mode interrupt

    }

    return distance;
}

void init_timer_interrupts(void)
{
    NVIC_EN1_R |= 0x00000010; //TIMER3B interrupts is interrupt #36, enabling the TIMER3B interrupt

    //IntMasterEnable();
    IntRegister(INT_TIMER3B, TIMER3B_Wrangler);
}

//ISR that captures rising and falling edge times of PING sensor
void TIMER3B_Wrangler(void)
{
    if (state == HIGH)
    {
        rising_time = TIMER3_TBR_R;
        state = LOW;
    }

    else if (state == LOW)
    {
        falling_time = TIMER3_TBR_R;
        state = DONE;

        professionallyNamedPulseCheck = 1;
    }

    TIMER3_ICR_R |= 0x00400; //clear interrupt
}
