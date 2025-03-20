#include "movement.h"
#include "adc.h"

// MAPPING VARIABLES ==============================================================================================
const unsigned int xMap = 112;
const unsigned int yMap = 180;

/*
 struct CyBotCoordinates
 {
 int x;
 int y;
 } ourCoords;
 */
double currentAngle = 90; // The angle that we're currently at in degrees

double xPos = 56.0; // Double for the current X-Position
double yPos = 90.0; // Double for the current X-Position
// ^^ We store the double version of our position to counteract incremental rounding throwing off our position. We don't want to have a rounding error every time we update the map...

char mapArray[yMap][xMap];

short orbitingFire = 0;
// ================================================================================================================

//global variables for sensor code
float total_distance[91]; //holds the total distance at each angle
short midAngle[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
short objectCount = 0; // How many objects we have
float minLinearWidth = 180;
short minWidthAngle = 0; // The angle at which the smallest radial width lies
float smallestDistance = 180;
float distanceReading = 0; //used to get distance from ping sensor
double sumDis = 0; //keeps track of the total distance gone
short fireRawValue = 3000; //raw value for the color red
short boundaryValueL = 2790; //raw value for the white boundary line
short boundaryValueR = 2750; //raw value for the white boundary line
short cliffValue = 1000;

char allowForward = 1; // If > 1, allow the driver to move forward

double totalDistance = 0; //variable to keep track of how far we've traveled

void move_forward(oi_t *sensor, double centimeters, int track, short mode)
{
    double mm = centimeters * 10;
    int16_t current_speed = 0;

    //short overshoot = 30; // The overshoot in mm

    double sum = 0;
    if (mm > 30)
    {
        while (sum < (mm - 30))
        {
            // Gradually increase/decrease the speed
            if ((current_speed < 300) && (sum < (mm * 0.7)))
                current_speed += 10;
            else if (current_speed > 100)
                current_speed -= 50;

            checkGroundSensors(sensor, mode);

            oi_setWheels(current_speed, current_speed);

            // oi_update(sensor);
            sum += sensor->distance; //whenever you look at the struct member distance, it returns the distance after it was last checked, so you need to continuously add the
            //checked distance to some sort of counter to get the total distance

            if (track == 1)
            {
                totalDistance += (sensor->distance / 10);
            }
        }

    }
    else
    {
        while (sum < mm)
        {
            // Move forward slowly
            oi_setWheels(100, 100);

            checkGroundSensors(sensor, mode);

            // oi_update(sensor);
            sum += sensor->distance; //whenever you look at the struct member distance, it returns the distance after it was last checked, so you need to continuously add the
            //checked distance to some sort of counter to get the total distance

            if (track == 1)
            {
                totalDistance += (sensor->distance / 10);
            }

        }

    }

    oi_setWheels(0, 0); //stops the iRobot

    free(sensor);
    sensor = oi_alloc();
}

void move_backward(oi_t *sensor, double centimeters)
{
    double mm = centimeters * 10;
    int16_t current_speed = 0;

    //short overshoot = 10; // The overshoot in mm

    double sum = 0;
    //while (sum > (mm + overshoot))
    while (sum > mm)
    {

        // Gradually increase/decrease the speed
        if ((current_speed > -300) && (sum > (mm * 0.7)))
            current_speed -= 10;
        else if (current_speed < -100)
            current_speed += 50;

        oi_setWheels(current_speed, current_speed);

        // oi_setWheels(-100, -100);

        oi_update(sensor);
        sum += sensor->distance; //whenever you look at the struct member distance, it returns the distance after it was last checked, so you need to continuously add the
        //checked distance to some sort of counter to get the total distance
        totalDistance -= (sensor->distance / 10);
    }

    oi_setWheels(0, 0); //stops the iRobot

    free(sensor);
    sensor = oi_alloc();
}

void turn_clockwise(oi_t *sensor, double degrees)
{
    double sum = 0;
    double overshoot = 10; // The overshoot in deg

    while (sum > (0 - (degrees - overshoot)))
    {
        oi_update(sensor);
        sum += sensor->angle;

        // currentAngle += sum;

        oi_setWheels(-100, 100);
    }

    currentAngle -= degrees;
    oi_setWheels(0, 0);

    free(sensor);
    sensor = oi_alloc();
}

void turn_counterClockwise(oi_t *sensor, double degrees)
{
    double sum = 0;
    double overshoot = 10; // The overshoot in deg

    while (sum < (degrees - overshoot))
    {
        oi_update(sensor);
        sum += sensor->angle;

        // currentAngle += sum;

        oi_setWheels(100, -100);
    }

    currentAngle += sum;
    oi_setWheels(0, 0);

    free(sensor);
    sensor = oi_alloc();
}

void manualTurn(unsigned char direction, oi_t *sensor, short mode)
{
    int16_t sensitivity = 50;

    if (direction > 1) // Turn CCW
    {
        oi_setWheels(sensitivity, -sensitivity);

        checkGroundSensors(sensor, mode);

    }
    else // Turn CW
    {
        oi_setWheels(-sensitivity, sensitivity);

        checkGroundSensors(sensor, mode);
        // Update the current angle
        // oi_update(sensor);
    }

    timer_waitMillis(250);

    oi_setWheels(0, 0);

    // Update the current angle
    oi_update(sensor);
    double turned = sensor->angle;
    currentAngle += (turned * 1.2);
    // lcd_printf("CurrentAngle: %2.2f", currentAngle);
    //lcd_printf("CurrAngle: %2.2f", currentAngle);

    char currAng[26];
    sprintf(currAng, "Current angle: %2.2f cm\n", currentAngle);

    uart_sendStr(currAng);
    uart_sendChar('\n');
    uart_sendChar('\r');

    free(sensor);
    sensor = oi_alloc();
}

void manualMove(unsigned char direction, oi_t *sensor, short mode)
{
    int16_t speed = 0;

    double oldSumDis = sumDis;

    int i;
    for (i = 0; i < 10; i++)
    {
        // Accelerate (For more consistent/accurate movements)
        if (i < 5)
            speed += 50;
        else
            speed -= 50;

        if (speed < 0)
        {
            speed = 0;
        }

        if (direction > 1) // FW
        {
            if (allowForward > 0)
            {
                oi_setWheels(speed, speed);
            }

            checkGroundSensors(sensor, mode);

            oi_update(sensor);
            sumDis += (sensor->distance) / 10;
        }
        else // BW
        {
            oi_setWheels(-speed, -speed);

            oi_update(sensor);
            sumDis += (sensor->distance) / 10;
        }

        timer_waitMillis(1);
    }

    oi_setWheels(0, 0);

// lcd_printf("SumDis: %2.2f", sumDis - oldSumDis);
// timer_waitMillis(250);

// Update the map with the distance we've traveled
    updateMap(sumDis - oldSumDis);

//    char totalDis[16];
//    sprintf(totalDis, "Curr Dis: %2.2f cm\n", sumDis);

    free(sensor);
    sensor = oi_alloc();
}

void bumpedLeft(oi_t *sensor)
{
    uart_sendStr("Left bumper was triggered");

    uart_sendChar('\n');
    uart_sendChar('\r');
    uart_sendChar('\n');
    uart_sendChar('\r');

    oi_setWheels(0, 0);

    move_backward(sensor, -1.5);
    /* turn_clockwise(sensor, 90);

     move_forward(sensor, 25, 0);
     turn_counterClockwise(sensor, 90);*/

//totalDistance -= 30;
//move_forward(sensor, 200 - totalDistance, 1);
    mapPoint('R', 19.0);
}

void bumpedRight(oi_t *sensor)
{
    uart_sendStr("Right bumper was triggered");

    uart_sendChar('\n');
    uart_sendChar('\r');
//    uart_sendChar('\n');
//    uart_sendChar('\r');

    oi_setWheels(0, 0);

    move_backward(sensor, -1.5);
    /*turn_counterClockwise(sensor, 90);

     move_forward(sensor, 25, 0);
     turn_clockwise(sensor, 90);*/

//totalDistance -= 30;
//move_forward(sensor, 200 - totalDistance, 1);
    mapPoint('R', 19.0);

}

void cliffDetected(oi_t *sensor)
{
//    uart_sendChar('\n');
//    uart_sendChar('\r');
//    uart_sendChar('\n');
//    uart_sendChar('\r');
//
//    cliffIRValues(sensor);
//
//    uart_sendChar('\n');
//    uart_sendChar('\r');
//    uart_sendChar('\n');
//    uart_sendChar('\r');

    //move_backward(sensor, -8);

// Update the map
    mapPoint('C', 18);
    allowForward = 0;

}

void cliffIRValues(oi_t *sensor)
{
    oi_update(sensor);

//    char signalValue[28];
//    sprintf(signalValue, "Front Left Raw Value: %d",
//            sensor->cliffFrontLeftSignal);
//
//    uart_sendStr(signalValue);
//    uart_sendChar('\n');
//    uart_sendChar('\r');
//
//    sprintf(signalValue, "Front Right Raw Value: %d",
//            sensor->cliffFrontRightSignal);
//    uart_sendStr(signalValue);
//    uart_sendChar('\n');
//    uart_sendChar('\r');
//    uart_sendChar('\n');
//    uart_sendChar('\r');
//
//    sprintf(signalValue, "Right Raw Value: %d", sensor->cliffRightSignal);
//    uart_sendStr(signalValue);
//    uart_sendChar('\n');
//    uart_sendChar('\r');
//    uart_sendChar('\n');
//    uart_sendChar('\r');
//
////    sprintf(signalValue, "Left Raw Value: %d", sensor->cliffLeftSignal);
//    uart_sendStr(signalValue);
//    uart_sendChar('\n');
//    uart_sendChar('\r');
//    uart_sendChar('\n');
//    uart_sendChar('\r');

    free(sensor);
    sensor = oi_alloc();
}

void adjustPathAuto(oi_t *sensor)
{
    uart_sendStr("Crossed the fire break!");
    cliffIRValues(sensor);

    manualMove(0, sensor, 0);
    oi_setWheels(0, 0);

}

// =====================================================================================================================================
// ================================               CODE FOR SENSOR ON ROBOT                 =============================================
// =====================================================================================================================================

//this function sweeps the field and collects the distance and angle information using ping sensor
void sweepFieldPing()
{
    servo_initialize();

    int l;
    for (l = 0; l < 1; l++)
    {
        char title[] = "Degrees\t\tDistance (cm) - Ping";

        int i;
        for (i = 0; i < strlen(title); i++)
        {
            uart_sendChar(title[i]);
        }

        uart_sendChar('\n');
        uart_sendChar('\r');

        for (i = 0; i < 91; i++)
        {
            ourBot_Scan(i * 2); //sets the sensor to position i, then updates the sensor struct with whatever it reads

//            char distance[10];
//            char angle[10];
//
//            sprintf(distance, "%2.2f", ping_read());
//            sprintf(angle, "%d", i * 2);

            //prints degrees amount
//            int j;
//            for (j = 0; j < strlen(angle); j++)
//            {
//                uart_sendChar(angle[j]);
//            }

            uart_sendChar('\t');
            uart_sendChar('\t');

            //prints angle amount
//            for (j = 0; j < strlen(distance); j++)
//            {
//                uart_sendChar(distance[j]);
//            }

            uart_sendChar('\n');
            uart_sendChar('\r');
        }

        uart_sendChar('\n');
        uart_sendChar('\r');
    }
}

void pingMidpoint()
{
//timer_init();
    servo_initialize();

    char title[] = "Midpoint scan at each object";

    int i;
    int j;
    for (i = 0; i < strlen(title); i++)
    {
        uart_sendChar(title[i]);
    }

    uart_sendChar('\n');
    uart_sendChar('\r');

    short scanCount = 5;
    float total = 0;

    for (i = 0; i < objectCount; i++)
    {
        total = 0;

        for (j = 0; j < scanCount; j++)
        {
            ourBot_Scan(midAngle[i]); //point sensor at the mid-angle of each object
            total += ping_read();

            /*char distance[10];
             char angle[10];

             sprintf(distance, "%2.2f", sensor.sound_dist);
             sprintf(angle, "%d", midAngle[i]);

             //prints angle amount
             int j;
             for (j = 0; j < strlen(angle); j++)
             {
             uart_sendChar(angle[j]);
             }

             uart_sendChar('\t');
             uart_sendChar('\t');

             //prints distance amount
             for (j = 0; j < strlen(distance); j++)
             {
             uart_sendChar(distance[j]);
             }

             uart_sendChar('\n');
             uart_sendChar('\r');*/
        }

        //timer_waitMicros(2000);
//        char distance[10];
//        char angle[10];
//
//        sprintf(distance, "%2.2f", (total / scanCount) - 7);
//        sprintf(angle, "%d", midAngle[i]);

        //prints angle amount
//        int j;
//        for (j = 0; j < strlen(angle); j++)
//        {
//            uart_sendChar(angle[j]);
//        }

        uart_sendChar('\t');
        uart_sendChar('\t');

        //prints distance amount
//        for (j = 0; j < strlen(distance); j++)
//        {
//            uart_sendChar(distance[j]);
//        }

        uart_sendChar('\n');
        uart_sendChar('\r');
    }

    uart_sendChar('\n');
    uart_sendChar('\r');
}

//this function sweeps the field and collects the distance and angle information using IR sensor
void sweepFieldIR(oi_t *sensor)
{
    servo_initialize();

    float calDistance = 0; // calibrated distance

    int l;
    for (l = 0; l < 1; l++)
    {
        char title[] = "Degrees\t\tDistance (cm) - IR";

        int i;
        for (i = 0; i < strlen(title); i++)
        {
            uart_sendChar(title[i]);
        }

        uart_sendChar('\n');
        uart_sendChar('\r');

        for (i = 0; i < 91; i++)
        {
            ourBot_Scan(i * 2); //sets the sensor to position i, then updates the sensor struct with whatever it reads

            calDistance = convertToCm(adc_read());

            //since we don't know if the float values start at zero, we need to make sure they get set to the correct value at the very beginning
            total_distance[i] = calDistance;

            char distance[10];
            char angle[10];

            sprintf(distance, "%2.2f", total_distance[i]);
            sprintf(angle, "%d", i * 2);

            //prints angle amount
            int j;
            for (j = 0; j < strlen(angle); j++)
            {
                uart_sendChar(angle[j]);
            }

            uart_sendChar('\t');
            uart_sendChar('\t');

            //prints distance amount
            for (j = 0; j < strlen(distance); j++)
            {
                uart_sendChar(distance[j]);
            }

            uart_sendChar('\n');
            uart_sendChar('\r');

//            // MAPPING -----------------------------
//            if (calDistance < 70.0)
//            {
//                // Update the map with an object
//                mapSweep(i * 2.0, (double)calDistance); // [angle] = current angle of sweep (in degrees),
//
//            }
//            // -------------------------------------
        }

        uart_sendChar('\n');
        uart_sendChar('\r');
    }

    free(sensor);
    sensor = oi_alloc();
}

void senseForward(oi_t *sensor, int inputAngle)
{
    servo_initialize();

    char title[] = "Distance in front (90 degrees): ";

    int i;
    for (i = 0; i < strlen(title); i++)
    {
        uart_sendChar(title[i]);
    }

//    uart_sendChar('\n');
//    uart_sendChar('\r');

    /*for (i = 60; i <= 120; i = i + 2)
     {
     ourBot_Scan(i); //sets the sensor to position i, then updates the sensor struct with whatever it reads

     //since we don't know if the float values start at zero, we need to make sure they get set to the correct value at the very beginning
     total_distance[i] = ping_read();

     char distance[10];
     char angle[10];

     sprintf(distance, "%2.2f", total_distance[i]);
     sprintf(angle, "%d", i);

     //prints angle amount
     int j;
     for (j = 0; j < strlen(angle); j++)
     {
     uart_sendChar(angle[j]);
     }

     uart_sendChar('\t');
     uart_sendChar('\t');

     //prints distance amount
     for (j = 0; j < strlen(distance); j++)
     {
     uart_sendChar(distance[j]);
     }

     uart_sendChar('\n');
     uart_sendChar('\r');

     //            // MAPPING -----------------------------
     //            if (calDistance < 70.0)
     //            {
     //                // Update the map with an object
     //                mapSweep(i * 2.0, (double)calDistance); // [angle] = current angle of sweep (in degrees),
     //
     //            }
     //            // -------------------------------------
     }*/

    ourBot_Scan(inputAngle); //sets the sensor to position i, then updates the sensor struct with whatever it reads

    //getting distance from the ping sensor
    float forwardDistance = ping_read();
    forwardDistance = ping_read(); //sometimes the distance from when the ping sensor is first used is bad, so get the second measurement instead to help with accuracy

    char distance[10];
    char angle[10];

    sprintf(distance, "%2.2f", forwardDistance);
//    sprintf(angle, "%d", inputAngle);
//
//    //prints angle amount
//    int j;
//    for (j = 0; j < strlen(angle); j++)
//    {
//        uart_sendChar(angle[j]);
//    }
//
//    uart_sendChar('\t');
//    uart_sendChar('\t');

    int j;
    //prints distance amount
    for (j = 0; j < strlen(distance); j++)
    {
        uart_sendChar(distance[j]);
    }

    uart_sendChar('\n');
    uart_sendChar('\r');

    uart_sendChar('\n');
    uart_sendChar('\r');

//    uart_sendChar('\n');
//    uart_sendChar('\r');

    free(sensor);
    sensor = oi_alloc();
}

//the function calculates the number of objects, their radial width, and their angles. It also figures out the object with the smallest radial width, and then points the sensor at it
int calculateData(float average_distance[], float min)
{
    float threshold = 15.0;

    char onObj = 0;
    char passThresh = 0; // The amount of "overpass" in degrees before incrementing the object count
    char hitObject = 0;
    short method_tracker = 0;
    int j;

    short startAngle = 0;
    short endAngle = 0;

    char radialWidth[10];
    char linearWidth_S[10];

    short minRadWidth = 180; // The current smallest radial width

//    char title2[] = "Object#\t\tAngle\t\tDistance\t\tWidth";
//
    int k;
//    for (k = 0; k < strlen(title2); k++)
//    {
//        uart_sendChar(title2[k]);
//    }
//
//    uart_sendChar('\n');
//    uart_sendChar('\r');

    int i;
    for (i = 3; i < 91; i++)
    {
        //when the robot goes from scanning an object to scanning a gap and passing the threshold
        if (onObj == 1 && (average_distance[i] > (min + threshold))
                && passThresh >= 4)
        {
            //objectCount++;
            passThresh = 0;
            onObj = 0;
            hitObject = 0;
            method_tracker++;

            endAngle = ((i - 1) * 2) - 4;

            midAngle[objectCount - 1] = ((endAngle - startAngle) / 2)
                    + startAngle;

            //finds the center distance of the object
            float min_distance = average_distance[startAngle / 2];
            for (k = (startAngle / 2); k <= (endAngle / 2); k++)
            {
                if (average_distance[k] < min_distance)
                {
                    min_distance = average_distance[k];
                }
            }

//            char distance[10];
//            sprintf(distance, "%2.2f", min_distance);
//
//            //prints distance amount
//            for (j = 0; j < strlen(distance); j++)
//            {
//                uart_sendChar(distance[j]);
//            }
//
//            uart_sendChar('\t');
//            uart_sendChar('\t');
//            uart_sendChar('\t');

            if (min_distance < smallestDistance)
            {
                smallestDistance = min_distance;
            }

            //finding the object with the smallest radial width for the sensor to point to at the end of the program
            /*if ((endAngle - startAngle) < minRadWidth)
             {
             minWidthAngle = ((endAngle + startAngle) / 2);
             minRadWidth = (endAngle - startAngle);
             }*/

            //finding the object with the smallest linear width for the sensor to point to at the end of the program
            float a = average_distance[startAngle / 2];
            float b = average_distance[(endAngle / 2)]; //-1 here?
            float radial_width =
                    (float) ((endAngle - startAngle) * (M_PI / 180)); //convert this to radians since cos() takes in a radian value

            float linearWidth = sqrt(
                    pow(a, 2) + pow(b, 2) - (2 * a * b * cos(radial_width)));

            //finds the object with the smallest linear width
            if (linearWidth < minLinearWidth)
            {
                minLinearWidth = linearWidth;
                minWidthAngle = ((endAngle + startAngle) / 2);
            }

//            sprintf(radialWidth, "%d", endAngle - startAngle);
//            sprintf(linearWidth_S, "%2.2f", linearWidth);

            //prints radial width
            /*for (j = 0; j < strlen(radialWidth); j++)
             {
             uart_sendChar(radialWidth[j]);
             }*/

            //prints linear width
//            for (j = 0; j < strlen(linearWidth_S); j++)
//            {
//                uart_sendChar(linearWidth_S[j]);
//            }
//
//            uart_sendChar('\n');
//            uart_sendChar('\r');
            mapSweep(midAngle[objectCount - 1], min_distance, linearWidth);
        }
        else if (average_distance[i] < (min + threshold))
        {
            onObj = 1;
            passThresh = 0;

            if (hitObject == 0)
            {
                objectCount++;
                hitObject = 1;
                startAngle = i * 2;

                //char distance[10];
                //char angle[10];
                //char object_count[10];

//                sprintf(distance, "%2.2f", average_distance[i]);
//                sprintf(angle, "%d", i * 2);
//                sprintf(object_count, "%d", objectCount);
//
//                //prints object number
//                for (j = 0; j < strlen(object_count); j++)
//                {
//                    uart_sendChar(object_count[j]);
//                }
//
//                uart_sendChar('\t');
//                uart_sendChar('\t');

//                //prints angle amount
//                for (j = 0; j < strlen(angle); j++)
//                {
//                    uart_sendChar(angle[j]);
//                }
//
//                uart_sendChar('\t');
//                uart_sendChar('\t');
            }
        }
        else if (onObj == 1)
        {
            passThresh += 2;
        }
    }

//in case there is an object at the end of the sweep and the sensor stops on that object
    if (method_tracker < objectCount)
    {
        endAngle = 180;

        float min_distance = average_distance[startAngle / 2];
        for (k = (startAngle / 2); k <= (endAngle / 2); k++)
        {
            if (average_distance[k] < min_distance)
            {
                min_distance = average_distance[k];
            }
        }

//        char distance[10];
//        sprintf(distance, "%2.2f", min_distance);

        //prints degrees amount
//        for (j = 0; j < strlen(distance); j++)
//        {
//            uart_sendChar(distance[j]);
//        }

//        uart_sendChar('\t');
//        uart_sendChar('\t');
//        uart_sendChar('\t');

        //finding the object with the smallest radial width for the sensor to point to at the end of the program
        /*if ((endAngle - startAngle) < minRadWidth)
         {
         minWidthAngle = ((endAngle + startAngle) / 2);
         minRadWidth = (endAngle - startAngle);
         }*/

        //finding the object with the smallest linear width for the sensor to point to at the end of the program
        float a = average_distance[startAngle / 2];
        float b = average_distance[endAngle / 2];
        float radial_width = (float) ((endAngle - startAngle) * (M_PI / 180)); //convert this to radians since cos() takes in a radian value

        float linearWidth = sqrt(
                pow(a, 2) + pow(b, 2) - (2 * a * b * cos(radial_width)));

        //finds the object with the smallest linear width
        if (linearWidth < minLinearWidth)
        {
            minLinearWidth = linearWidth;
            minWidthAngle = ((endAngle + startAngle) / 2);
        }

//        sprintf(radialWidth, "%d", endAngle - startAngle);
//         sprintf(linearWidth_S, "%2.2f", linearWidth);

        //prints radial width
        /*for (k = 0; k < strlen(radialWidth); k++)
         {
         uart_sendChar(radialWidth[k]);
         }*/

        //prints linear width
//        for (j = 0; j < strlen(linearWidth_S); j++)
//        {
//            uart_sendChar(linearWidth_S[j]);
//        }
//
//        uart_sendChar('\n');
//        uart_sendChar('\r');
    }

    return minWidthAngle;
}

//since the IR sensor returns data that isn't in cm, we made a formula to convert that data into cm so we can work with it
float convertToCm(int input)
{
//double distanceInCm = (3 * pow(10, 7)) * pow((double) input, -1.89);
//double distanceInCm = (7 * pow(10, 6)) * pow((double) input, -1.72);
    double distanceInCm = (89094 * pow((double) input, -1.14)); //cybot 04
    return (float) distanceInCm;
}

void findObjects()
{
//finding the minimum average distance where a object is read
    int i;

    float min = total_distance[0];
    for (i = 0; i < 91; i++)
    {
        if (total_distance[i] < min)
        {
            min = total_distance[i];
        }
    }

    uart_sendChar('\n');
    uart_sendChar('\r');

    uart_sendChar('\n');
    uart_sendChar('\r');

    minWidthAngle = calculateData(total_distance, min);

    uart_sendChar('\n');
    uart_sendChar('\r');

//    char pointedAngle[30];
//    sprintf(pointedAngle, "Sensor is pointing at angle %d", minWidthAngle);

//prints where the sensor is pointing at the end
//    for (i = 0; i < strlen(pointedAngle); i++)
//    {
//        uart_sendChar(pointedAngle[i]);
//    }

    uart_sendChar('\n');
    uart_sendChar('\r');

    uart_sendChar('\n');
    uart_sendChar('\r');

    ourBot_Scan(minWidthAngle); //moves sensor to the object with the smallest radial width

}

void resetTotalData()
{
    int i;
    for (i = 0; i < 91; i++)
    {
        total_distance[i] = 0;
    }

    objectCount = 0;
}

void automomousDriving(oi_t *sensor_data)
{
    if (minWidthAngle > 90)
    {
        turn_counterClockwise(sensor_data, minWidthAngle - 90);
    }
    else if (minWidthAngle < 90)
    {
        turn_clockwise(sensor_data, 90 - minWidthAngle);
    }

//cyBOT_Scan(90, &sensor); //sets the sensor to position i, then updates the sensor struct with whatever it reads
    float distanceWeWannaGoHo = smallestDistance - 7;

//    char distance[30];
//    char minWidth[26];
//    sprintf(distance, "The distance to go: %2.2f", distanceWeWannaGoHo);
//    sprintf(minWidth, "Minimum width: %d", minWidthAngle);

//    int j;
////prints degrees amount
//    for (j = 0; j < strlen(distance); j++)
//    {
//        uart_sendChar(distance[j]);
//    }
//
//    for (j = 0; j < strlen(minWidth); j++)
//    {
//        uart_sendChar(minWidth[j]);
//    }

    uart_sendChar('\n');
    uart_sendChar('\r');
    uart_sendChar('\n');
    uart_sendChar('\r');

    move_forward(sensor_data, (double) distanceWeWannaGoHo, 0, 1);
}

void findWhereToGo()
{
    char word[30];

    if (minWidthAngle > 90)
    {
//        sprintf(word, "Turn left by %d degrees", (minWidthAngle - 90));
    }
    else if (minWidthAngle < 90)
    {
//        sprintf(word, "Turn right by %d degrees", (90 - minWidthAngle));
    }

    uart_sendStr(word);

    uart_sendChar('\n');
    uart_sendChar('\r');
    uart_sendChar('\n');
    uart_sendChar('\r');
}

void extinguishFire(oi_t *sensor)
{
    uart_sendStr("Robot will begin extinguishing the fire!");
    uart_sendChar('\n');
    uart_sendChar('\r');
    uart_sendChar('\n');
    uart_sendChar('\r');

    short totalAngle = 0;

    short totalDis = 0;

//first check that the robot is not to far or to close to the fire
    char robotIsOk = 0;

    char totAngle[40];
    short numTurns = 0;

    while (robotIsOk == 0)
    {
        oi_update(sensor);

        if (((sensor->cliffFrontRightSignal >= fireRawValue)
                && (sensor->cliffFrontRightSignal < (fireRawValue + 400)))
                || ((sensor->cliffFrontLeftSignal >= fireRawValue)
                        && (sensor->cliffFrontLeftSignal < (fireRawValue + 400))))
        {
            robotIsOk = 1;
            mapPoint('F', 0);
            orbitingFire = 1;
        }
        else
        {
            manualMove(10, sensor, 0);
            mapPoint('F', 0);
        }

        if (uart_data == 'o')
        {
            break;
        }
    }

    robotIsOk = 0;

    //set robot into correct position before moving around the fire
    while (robotIsOk == 0)
    {
        oi_update(sensor);

        if (((sensor->cliffFrontRightSignal >= fireRawValue)
                && (sensor->cliffFrontRightSignal < (fireRawValue + 400))))
        {
            turn_clockwise(sensor, 15);
        }

        else
        {
            robotIsOk = 1;
        }

        if (uart_data == 'o')
        {
            break;
        }
    }

    robotIsOk = 0;

    //this code goes around the fire
    while (robotIsOk == 0)
    {
        oi_update(sensor);

        if (sensor->cliffLeftSignal > 2700)
        {
            if ((sensor->cliffFrontRightSignal >= 2700))
            {
                turn_clockwise(sensor, 15);
                totalAngle -= 15;
                sprintf(totAngle, "Angle amount: %d", totalAngle);
                uart_sendStr(totAngle);
            }

            else
            {
                manualMove(10, sensor, 0);
                mapPoint('F', 0);
                mapPoint('F', -7.0);
            }

            if (uart_data == 'o')
            {
                break;
            }
        }

        else if ((sensor->cliffFrontRightSignal >= 2700))
        {
            turn_clockwise(sensor, 15);
            totalAngle -= 15;
            sprintf(totAngle, "Angle amount: %d", totalAngle);
            uart_sendStr(totAngle);
        }

        //auto-correcting turn when the robot reaches a corner of the fire zone
        else
        {
            oi_update(sensor);

            turn_counterClockwise(sensor, 90);
            totalAngle += 90;

            totalDis += sensor->distance;

            if (totalDis >= 30)
            {
                numTurns++;
                totalDis = 0;
            }

            sprintf(totAngle, "Angle amount: %d", totalAngle);
            uart_sendStr(totAngle);
        }

        if (numTurns == 5)
        {
            robotIsOk = 1;
        }

        if (uart_data == 'o')
        {
            break;
        }
    }

    orbitingFire = 0;

    free(sensor);
    sensor = oi_alloc();
}

void createMap(oi_t *sensor)
{
    int i, j;

    for (i = 0; i < yMap; i++)
    {
        for (j = 0; j < xMap; j++)
        {
            mapArray[i][j] = ' ';
        }
    }

    // createSong();

    oi_play_song(0);
}

void updateMap(double distance)
{
    double theta = (currentAngle * 3.14159) / 180; // The angle in radians

// Set set the old point to a "Charted" character
    if (orbitingFire == 0)
    {
        // mapArray[(int) yPos][(int) xPos] = '.';
        // "Clear" the other directions
        mapArray[(int) yPos + 1][(int) xPos] = '.';
        mapArray[(int) yPos - 1][(int) xPos] = '.';
        mapArray[(int) yPos][(int) xPos + 1] = '.';
        mapArray[(int) yPos][(int) xPos - 1] = '.';

        mapArray[(int) yPos + 1][(int) xPos + 1] = '.';
        mapArray[(int) yPos - 1][(int) xPos + 1] = '.';
        mapArray[(int) yPos + 1][(int) xPos - 1] = '.';
        mapArray[(int) yPos - 1][(int) xPos - 1] = '.';
    }

    xPos += (distance * 0.2 * cos(theta));
    yPos -= (distance * 0.2 * sin(theta));

    if (orbitingFire > 0)
    {
        mapArray[(int) yPos][(int) xPos] = 'F';
        mapArray[(int) yPos + 1][(int) xPos] = 'F';
        mapArray[(int) yPos - 1][(int) xPos] = 'F';
        mapArray[(int) yPos][(int) xPos + 1] = 'F';
        mapArray[(int) yPos][(int) xPos - 1] = 'F';

        mapArray[(int) yPos + 1][(int) xPos + 1] = 'F';
        mapArray[(int) yPos - 1][(int) xPos + 1] = 'F';
        mapArray[(int) yPos + 1][(int) xPos - 1] = 'F';
        mapArray[(int) yPos - 1][(int) xPos - 1] = 'F';
    }

}

void mapPoint(char obstacleType, double distanceFromCenter) // [X] = boundary, [O] = Obstacle/Tree, [R] = Bump/Rock, [F] = Fire, [C} = Cliff Edge
{
    double theta = (currentAngle * 3.14159) / 180; // The angle in radians

// Calculate where we need to place the obstacle
    double xTarget = xPos + (distanceFromCenter * 0.2 * cos(theta));
    double yTarget = yPos - (distanceFromCenter * 0.2 * sin(theta));

// Place the character at the target point on the map
    mapArray[(int) yTarget][(int) xTarget] = obstacleType;

    if (obstacleType == 'F')
    {
        mapArray[(int) yPos + 1][(int) xPos] = 'F';
        mapArray[(int) yPos - 1][(int) xPos] = 'F';
        mapArray[(int) yPos][(int) xPos + 1] = 'F';
        mapArray[(int) yPos][(int) xPos - 1] = 'F';

        mapArray[(int) yPos + 1][(int) xPos + 1] = 'F';
        mapArray[(int) yPos - 1][(int) xPos + 1] = 'F';
        mapArray[(int) yPos + 1][(int) xPos - 1] = 'F';
        mapArray[(int) yPos - 1][(int) xPos - 1] = 'F';
    }
}

void printMap()
{
// Set our initial position in the map
    mapArray[(int) yPos][(int) xPos] = '@';

    if (currentAngle > 360)
    {
        currentAngle -= 360;
    }
    else if (currentAngle < -360)
    {
        currentAngle += 360;
    }

    // Check the orientation
    if ((int) yPos > 1
            && ((currentAngle > 45 && currentAngle < 135) || currentAngle < -315))
    {
        // "Clear" the other directions
        mapArray[(int) yPos + 1][(int) xPos] = '@';
        mapArray[(int) yPos][(int) xPos - 1] = '@';
        mapArray[(int) yPos][(int) xPos + 1] = '@';

        // Place an orientation indicator
        mapArray[(int) yPos - 1][(int) xPos] = '^';
    }
    else if ((int) xPos > 1
            && ((currentAngle > 135 && currentAngle < 225)
                    || (currentAngle < -135 && currentAngle > -315)))
    {
        // "Clear" the other directions
        mapArray[(int) yPos + 1][(int) xPos] = '@';
        mapArray[(int) yPos - 1][(int) xPos] = '@';
        mapArray[(int) yPos][(int) xPos + 1] = '@';

        // Place an orientation indicator
        mapArray[(int) yPos][(int) xPos - 1] = '<';
    }
    else if (((int) yPos) < yMap
            && ((currentAngle > 225 && currentAngle < 315)
                    || (currentAngle < -45 && currentAngle > -135)))
    {
        // "Clear" the other directions
        mapArray[(int) yPos][(int) xPos - 1] = '@';
        mapArray[(int) yPos - 1][(int) xPos - 1] = '@';
        mapArray[(int) yPos][(int) xPos + 1] = '@';

        // Place an orientation indicator
        mapArray[(int) yPos + 1][(int) xPos] = 'V';
    }
    else if ((int) xPos < xMap)
    {
        // "Clear" the other directions
        mapArray[(int) yPos + 1][(int) xPos] = '@';
        mapArray[(int) yPos - 1][(int) xPos] = '@';
        mapArray[(int) yPos][(int) xPos - 1] = '@';

        // Place an orientation indicator
        mapArray[(int) yPos][(int) xPos + 1] = '>';
    }

// Print the map
    int i, j;
    for (i = 0; i < yMap; i++)
    {
        for (j = 0; j < xMap; j++)
        {
            uart_sendChar(mapArray[i][j]);

            // timer_waitMicros(10);
        }

        uart_sendChar('\n');
        uart_sendChar('\r');
    }
}

void mapSweep(double angle, double obstacleDistance, double linWidth) // [angle] = current angle of sweep (in degrees),
// [obstacleDistance] = The distance at which we detect an obstacle (-1 if we don't have an object)
{
// Check if we have an obstacle
    if (obstacleDistance > 0)
    {
        // Calculate the angle we need to place points
        double newAngle = (angle - 90) + currentAngle;
        double theta = (newAngle * 3.14159) / 180; // The angle in radians

        // Calculate where we need to place the obstacle
        double xTarget = xPos
                + (((linWidth * 0.5) + obstacleDistance) * 0.2 * cos(theta)); // Adding 13.97cm because this is the approximate distance from the center of the bot to the sensor
        double yTarget = yPos
                - (((linWidth * 0.5) + obstacleDistance) * 0.2 * sin(theta));

        // Place the character at the target point on the map
        mapArray[(int) yTarget][(int) xTarget] = 'O';
    }
}

void printControls()
{
    uart_sendStr(
            "----------------------------------------------------------------------------------------------");
    uart_sendChar('\n');
    uart_sendChar('\r');

    uart_sendStr("[W] --> drive forward");
    uart_sendChar('\n');
    uart_sendChar('\r');

    uart_sendStr("[S] --> drive backward");
    uart_sendChar('\n');
    uart_sendChar('\r');

    uart_sendStr("[A] --> turn counter clockwise");
    uart_sendChar('\n');
    uart_sendChar('\r');

    uart_sendStr("[D] --> turn clockwise");
    uart_sendChar('\n');
    uart_sendChar('\r');

    uart_sendStr("[F] --> 90 degree forward scan");
    uart_sendChar('\n');
    uart_sendChar('\r');

    uart_sendStr("[N] --> 180 degree sweep");
    uart_sendChar('\n');
    uart_sendChar('\r');

    uart_sendStr("[M] --> print cliff IR values");
    uart_sendChar('\n');
    uart_sendChar('\r');

    uart_sendStr("[P] --> print map");
    uart_sendChar('\n');
    uart_sendChar('\r');

    uart_sendStr("[R] --> reset map");
    uart_sendChar('\n');
    uart_sendChar('\r');

    uart_sendStr("[T] --> start autonomous");
    uart_sendChar('\n');
    uart_sendChar('\r');

    uart_sendStr("[O] --> cancel out of autonomous");
    uart_sendChar('\n');
    uart_sendChar('\r');

    uart_sendStr("[Q] --> stop the entire program/Cybot");
    uart_sendChar('\n');
    uart_sendChar('\r');

    uart_sendStr(
            "----------------------------------------------------------------------------------------------");
    uart_sendChar('\n');
    uart_sendChar('\r');
}

void checkGroundSensors(oi_t *sensor, short mode)
{
    oi_update(sensor);

    if (sensor->bumpLeft)
    {
        bumpedLeft(sensor);
        return;
    }
    else if (sensor->bumpRight)
    {
        bumpedRight(sensor);
        return;
    }

    //for detecting a cliff
    if (sensor->cliffFrontLeftSignal < cliffValue)
    {
        uart_sendStr("Near a cliff edge on front left sensor! (can't move forward, move carefully)");
        cliffDetected(sensor);

        move_backward(sensor, -1);

        while (uart_data == 'w')
        {
            oi_setWheels(0, 0);
            allowForward = 0;
        }

        // Tell the map that we have a boundary!
        mapPoint('X', 18.0);
        return;
    }

    //for detecting a cliff
    if (sensor->cliffFrontRightSignal < cliffValue)
    {
        uart_sendStr("Near a cliff edge on front right sensor! (can't move forward, move carefully)");
        cliffDetected(sensor);

        move_backward(sensor, -1);

        while (uart_data == 'w')
        {
            oi_setWheels(0, 0);
            allowForward = 0;
        }

        // Tell the map that we have a boundary!
        mapPoint('X', 18.0);
        return;
    }

    //for detecting a cliff
    if (sensor->cliffLeftSignal < cliffValue)
    {
        uart_sendStr("Near a cliff edge on left sensor! (can't move forward, move carefully)");
        cliffDetected(sensor);

        move_backward(sensor, -1);

        while (uart_data == 'w')
        {
            oi_setWheels(0, 0);
            allowForward = 0;
        }

        // Tell the map that we have a boundary!
        mapPoint('X', 18.0);
        return;
    }

    //for detecting a cliff
    if (sensor->cliffRightSignal < 300)
    {
        uart_sendStr("Near a cliff edge on right sensor! (can't move forward, move carefully)");
        cliffDetected(sensor);

        move_backward(sensor, -1);

        while (uart_data == 'w')
        {
            oi_setWheels(0, 0);
            allowForward = 0;
        }

        // Tell the map that we have a boundary!
        mapPoint('X', 18.0);
        return;
    }

    if (mode == 1)
    {
        //for detecting red in autonomous part
        if ((sensor->cliffFrontLeftSignal >= fireRawValue)
                && (sensor->cliffFrontLeftSignal < (fireRawValue + 400)))
        {
            adjustPathAuto(sensor);
            mapPoint('F', 18.0);
        }

        //for detecting red in autonomous part
        if ((sensor->cliffFrontRightSignal >= fireRawValue)
                && (sensor->cliffFrontRightSignal < (fireRawValue + 400)))
        {
            adjustPathAuto(sensor);
            mapPoint('F', 18.0);
        }

        //for detecting the white boundary
        else if ((sensor->cliffFrontLeftSignal > (boundaryValueL - 20))
                && (sensor->cliffFrontLeftSignal < (boundaryValueL + 20)))
        {
            uart_sendStr(
                    "Near the border edge on front left sensor! (can't move forward, move carefully)");
//            allowForward = 0;

            move_backward(sensor, -1);

            while (uart_data == 'w')
            {
                oi_setWheels(0, 0);
                allowForward = 0;
            }

            // Tell the map that we have a boundary!
            mapPoint('X', 18.0);
            return;
        }

        //for detecting the white boundary
        else if ((sensor->cliffFrontRightSignal > (boundaryValueR - 20))
                && (sensor->cliffFrontRightSignal < (boundaryValueR + 20)))
        {
            uart_sendStr(
                    "Near the border edge on front right sensor! (can't move forward, move carefully)");
//            allowForward = 0;

            move_backward(sensor, -1);

            while (uart_data == 'w')
            {
                oi_setWheels(0, 0);
                allowForward = 0;
            }

            // Tell the map that we have a boundary!
            mapPoint('X', 18.0);
            return;
        }

        //for detecting the white boundary
        else if ((sensor->cliffFrontLeftSignal > (boundaryValueL - 20))
                && (sensor->cliffFrontLeftSignal < (boundaryValueL + 20))
                && (sensor->cliffFrontRightSignal > (boundaryValueR - 20))
                && (sensor->cliffFrontRightSignal < (boundaryValueR + 20)))
        {
            uart_sendStr(
                    "Near the border edge on front left sensor and front right sensor! (can't move forward, move carefully)");
//            allowForward = 0;

            move_backward(sensor, -1);

            while (uart_data == 'w')
            {
                oi_setWheels(0, 0);
                allowForward = 0;
            }

            // Tell the map that we have a boundary!
            mapPoint('X', 18.0);
            return;
        }

        //for detecting the white boundary
        else if ((sensor->cliffRightSignal >= 2700)
                && (sensor->cliffRightSignal <= 2800))
        {
            uart_sendStr(
                    "Near the border edge on right sensor! (can't move forward, move carefully)");
            //            allowForward = 0;

            move_backward(sensor, -1);

            while (uart_data == 'w')
            {
                oi_setWheels(0, 0);
                allowForward = 0;
            }

            // Tell the map that we have a boundary!
            mapPoint('X', 18.0);
            return;
        }

        //for detecting the white boundary
        else if ((sensor->cliffLeftSignal >= 2700)
                && (sensor->cliffLeftSignal <= 2800))
        {
            uart_sendStr(
                    "Near the border edge on left sensor! (can't move forward, move carefully)");
            //            allowForward = 0;

            move_backward(sensor, -1);

            while (uart_data == 'w')
            {
                oi_setWheels(0, 0);
                allowForward = 0;
            }

            // Tell the map that we have a boundary!
            mapPoint('X', 18.0);
            return;
        }
    }

    allowForward = 1;

    free(sensor);
    sensor = oi_alloc();
}

void createSong()
{
    int songI = 0;
    int numNotes = 4;
    unsigned char notes[] = { 1, 3, 3, 11 };
    unsigned char noteDur[] = { 1, 2, 3, 6 };

    oi_loadSong(songI, numNotes, notes, noteDur);
}
