#include "adc.h"

//initalize ADC module 0 to use AIN10
void adc_init(void)
{
    //ADC registers to enable ADC0 with SSO
    SYSCTL_RCGCADC_R |= 0b01; //enable ADC0 with clock

    //GPIO registers to enable AIN10 on PB4
    SYSCTL_RCGCGPIO_R |= 0b00000010; //enable GPIO port B clock
    GPIO_PORTB_AFSEL_R |= 0b00010000; //set PB4 to peripheral signal
    GPIO_PORTB_DEN_R &= 0b11101111; //disable digital functions for PB4
    GPIO_PORTB_DIR_R &= 0b11101111; //set direction of PB4 to input
    GPIO_PORTB_AMSEL_R |= 0b00010000; //enable analog functions (AIN10) for PB4

    ADC0_ACTSS_R &= 0xFFFFFFFE; //disable SS0
    ADC0_SSMUX0_R |= 0xA; //AIN10 is the first and only sample to be converted
    ADC0_SSCTL0_R |= 0x6; //AIN10 is the only sample to convert, so stop after the first sample is converted
    ADC0_CC_R = 0x0; //set clock source for ADC
    ADC0_ACTSS_R |= 0x1; //enable SS0
}

int adc_read(void)
{
    int data;

    ADC0_PSSI_R |= 0x1; //start sampling on SS0
    ADC0_SAC_R |= 0x4; //16x hardware averaging

    //wait for the samples to be done with their conversion
    while ((~ADC0_RIS_R & 0x1))
    {

    }

    ADC0_ISC_R |= 0x1; //clear SS0 interrupt

    data = ADC0_SSFIFO0_R & 0xFFF; //store converted data into results variable

    return data;
}
