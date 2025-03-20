#include "open_interface.h"
#include "uart.h"
#include "timer.h"
#include "servo.h"
#include "ping.h"

//function to move the iRobot forward
void move_forward(oi_t *sensor, double centimeters, int track, short mode);

//function to move the iRobot backwards
void move_backward(oi_t *sensor, double centimeters);

//function to turn the iRobot clockwise/to the right
void turn_clockwise(oi_t *sensor, double degrees);

//function to turn the iRobot counterclockwise/to the left
void turn_counterClockwise(oi_t *sensor, double degrees);

//function to manually turn the iRobot clockwise via Putty
void manualTurn(unsigned char direction, oi_t *sensor, short mode);

//function to manually move the iRobot forward via Putty
void manualMove(unsigned char direction, oi_t *sensor, short mode);

//function to deal with iRobot collided with its left bumper
void bumpedLeft(oi_t *sensor);

//function to deal with iRobot collided with its right bumper
void bumpedRight(oi_t *sensor);

void cliffDetected(oi_t *sensor);

void cliffIRValues(oi_t *sensor);

void adjustPathAuto(oi_t *sensor);

void sweepFieldPing();

void pingMidpoint();

void sweepFieldIR(oi_t *sensor);

void senseForward(oi_t *sensor, int angle);

int calculateData(float average_distance[], float min);

float convertToCm(int input);

void findObjects();

void resetTotalData();

void automomousDriving(oi_t *sensor_data);

void findWhereToGo();

void extinguishFire(oi_t *sensor);

void createMap(oi_t *sensor);

void updateMap(double distance);

void mapPoint(char obstacleType, double distanceFromCenter);

void printMap();

void mapSweep(double angle, double obstacleDistance, double linWidth); // [angle] = current angle of sweep (in degrees)

void printControls();

void checkGroundSensors(oi_t *sensor, short mode);

void createSong();


