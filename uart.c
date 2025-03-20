/*
 *
 *   uart_extra_help.c
 * Description: This is file is meant for those that would like a little
 *              extra help with formatting their code, and followig the Datasheet.
 */

#include "uart.h"
#include "timer.h"
#define REPLACE_ME 0x00

// Global varible
volatile char uart_data;
volatile char flag = 0;

void uart_init(void)
{
    SYSCTL_RCGCGPIO_R |= 0b00000010;      // enable clock GPIOB (page 340)
    SYSCTL_RCGCUART_R |= 0b00000010;      // enable clock UART1 (page 344)
    GPIO_PORTB_AFSEL_R = 0b00000011; // sets PB0 and PB1 as peripherals (page 671)
    GPIO_PORTB_PCTL_R = 0x00000011; // pmc0 and pmc1       (page 688)  also refer to page 650
    GPIO_PORTB_DEN_R = 0b00000011;        // enables pb0 and pb1
    GPIO_PORTB_DIR_R = 0b00000001;        // sets pb0 as output, pb1 as input

    //compute baud values [UART clock= 16 MHz] 
    double fbrd;
    int ibrd;

    //baud rate is 115,200
    fbrd = 44; // page 903
    ibrd = 8;
    //fbrd = REPLACE_ME;

    UART1_CTL_R &= 0xFFFFFFFE;      // disable UART1 (page 918)
    UART1_IBRD_R = 0x0008;        // write integer portion of BRD to IBRD
    UART1_FBRD_R = 0b00101100;   // write fractional portion of BRD to FBRD
    UART1_LCRH_R = 0b01100000; // write serial communication parameters (page 916) * 8bit and no parity
    UART1_CC_R = 0x0;          // use system clock as clock source (page 939)
    UART1_CTL_R |= 0x00000001;        // enable UART1

}

//to send a single character to putty
void uart_sendChar(char data)
{
    //stay in this loop as long as the FIFO is full
    while (UART1_FR_R & 0x20)
    {

    }

    //send data after leaving the loop
    UART1_DR_R = data;
}

//to get character from putty
char uart_receive()
{
    char data = 0;

    //mask the 4 error bits and grab only 8 data bits
    data = (char) (UART1_DR_R & 0xFF);
/*
    if(returnType == 1)
    {
        return '\n';
    }

    else
    {
    */
        return data;
    //}
}

//to send multiple characters to putty
void uart_sendStr(const char *data)
{
    int i;
    for (i = 0; i < strlen(data); i++)
    {
        uart_sendChar(data[i]);
    }

    uart_sendChar('\n');
    uart_sendChar('\r');
    uart_sendChar('\n');
    uart_sendChar('\r');
}

// _PART3

void uart_interrupt_init()
{
    //turn off UART1 while its being set up
    UART1_CTL_R &= 0xFFFFFFFE;

    //clear interrupt flags
    UART1_ICR_R |= 0x010;

    // Enable interrupts for receiving bytes through UART1
    UART1_IM_R |= 0b10000; //enable interrupt on receive - page 924

    // Find the NVIC enable register and bit responsible for UART1 in table 2-9
    // Note: NVIC register descriptions are found in chapter 3.4
    NVIC_EN0_R |= 0x000000040; //enable uart1 interrupts - page 104

    // Find the vector number of UART1 in table 2-9 ! UART1 is 22 from vector number page 104
    IntRegister(INT_UART1, uart_interrupt_handler); //give the microcontroller the address of our interrupt handler - page 104 22 is the vector number

    //re-enable UART1
    UART1_CTL_R |= 0x00000001;
}

void uart_interrupt_handler()
{
// STEP1: Check the Masked Interrupt Status

//STEP2:  Copy the data 

//STEP3:  Clear the interrupt

    if (UART1_MIS_R & 0b0000000010000)
    {
        uart_data = uart_receive();
        flag = 1;

        UART1_ICR_R |= 0b0000000010000;
    }
    else
    {
        flag = 0;
    }

}
