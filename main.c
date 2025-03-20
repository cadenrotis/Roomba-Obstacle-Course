/**
 * main.c
 */

#include "timer.h"
#include "lcd.h"
#include "uart.h"
#include "movement.h"
#include "open_interface.h"
#include "adc.h"
#include <math.h>
#include "ping.h"
#include "servo.h"
#include "button.h"w

int main(void)
{
    timer_init();
    //lcd_init();
    ping_initialize();
    init_timer_interrupts();
    servo_initialize();
    button_init();

    ourBot_Scan(0); //sets the sensor to position 0

    uart_init();
    uart_interrupt_init();
    adc_init();

    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);


    // Initialize the map
    createSong();

    createMap(sensor_data);

    char toggle = 0;
    int m_count = 0;
    int h_count = 0;

    while (uart_data != 'q')
    {
        //changing the mode of robot between autonomous and manual modes
        if (uart_data == 't')
        {
            if (toggle == 0)
            {
                toggle = 1;
                uart_sendStr("In autonomous mode");
                uart_data = 0;

                uart_sendChar('\n');
                uart_sendChar('\r');
                uart_sendChar('\n');
                uart_sendChar('\r');
            }
            else
            {
                toggle = 0;
                uart_sendStr("In manual mode");
                uart_data = 0;

                uart_sendChar('\n');
                uart_sendChar('\r');
                uart_sendChar('\n');
                uart_sendChar('\r');
            }

            oi_setWheels(0, 0);
        }

        //robot is in manual mode --> to navigate to fires and explore the terrain/field
        if (toggle == 0)
        {
            h_count = 0;

            resetTotalData();


            if (uart_data == 'm')
            {
                cliffIRValues(sensor_data);
                uart_data = 0;
            }

            //robot will do a 180 degree sweep using the IR sensor
            if (uart_data == 'n')
            {
                sweepFieldIR(sensor_data);
                //findObjects();

                uart_data = 0;

                if (m_count == 3)
                {
                    resetTotalData();
                }
            }

            //robot will print the map onto Putty
            if (uart_data == 'p')
            {
                printMap();
                uart_data = 0;
            }

            //robot will use the ping sensor to scan what is in front of it
            if (uart_data == 'f')
            {
                senseForward(sensor_data, 93);

                uart_data = 0;
            }

            //robot will turn left or counterclockwise
            if (uart_data == 'a')
            {
                manualTurn(10, sensor_data, 1);

                uart_data = 0;
                resetTotalData();
            }

            //robot will turn right or clockwise
            else if (uart_data == 'd')
            {
                manualTurn(0, sensor_data, 1);

                uart_data = 0;
                resetTotalData();
            }

            //robot will move forward
            else if (uart_data == 'w')
            {
                manualMove(10, sensor_data, 1);

                uart_data = 0;
                resetTotalData();
            }

            //robot will move backward
            else if (uart_data == 's')
            {
                manualMove(0, sensor_data, 1);

                uart_data = 0;
                resetTotalData();
            }

            //robot will clear the map
            else if (uart_data == 'r')
            {
                createMap(sensor_data);
                uart_data = 0;

                uart_sendStr("Map has been reset!");

                uart_sendChar('\n');
                uart_sendChar('\r');
            }

            //Since we have a lot of keys mapped to the functionality of the robot, this will print what each key does to Putty
            else if(uart_data == 'e')
            {
                printControls();
                uart_data = 0;
            }

        }

        //robot is in autonomous mode --> robot will autonomously go around a fire to extinguish it
        //once we get the color sensor, the autonomous program should start automatically when the color sensor detects a fire
        else
        {
            extinguishFire(sensor_data);
            toggle = 0;
        }
    }

    oi_free(sensor_data);
}
