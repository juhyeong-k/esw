#include "system_management.h"
#include "car_control.h"

Driver::Driver()
{
    lastCVinfo = {1500, 0,};
    driveState = {1,0,};
}
void Driver::drive(CVinfo info)
{
    if(driveState.isGoingStraight) {
        if(info.isRightTurnDetected | info.isLeftTurnDetected) {
            driveState.isGoingStraight = false;
            driveState.isEnteringCurve = true;
            return;
        }
        else if( !(driveState.isTurningRight | driveState.isTurningLeft) ) {
            SteeringServoControl_Write(info.direction);
            return;
        }
    }
    if(driveState.isTurningRight) {
        if( !(info.isLeftDetected | info.isRightDetected) ) {
            driveState.isTurningRight = false;
            driveState.isGoingStraight = true;
            return;
        }
        else return;
    }
    if(driveState.isTurningLeft) {
        if( !(info.isLeftDetected | info.isRightDetected) ) {
            driveState.isTurningLeft = false;
            driveState.isGoingStraight = true;
            return;
        }
        else return;
    }
    if(driveState.isEnteringCurve) {
        if(info.isRoadClose) {
            if(info.direction < 1500) {
                driveState.isEnteringCurve = false;
                driveState.isTurningRight = true;
                SteeringServoControl_Write(1000);
                return;
            }
            else if(info.direction > 1500) {
                driveState.isEnteringCurve = false;
                driveState.isTurningLeft = true;
                SteeringServoControl_Write(2000);
                printf("SteeringServoControl_Write\r\n");
                return;
            }
            else {
                SteeringServoControl_Write(info.direction);
                return;
            }
        }
        else {
            SteeringServoControl_Write(1500);
            return;
        }
    }
}
void Driver::decisionMaking(CVinfo info)
{
    
}
void Driver::waitRightDetect(CVinfo info)
{

}
void Driver::waitLeftDetect(CVinfo info)
{

}
void Driver::waitRoadClose(CVinfo info)
{

}
void Driver::waitStartSignal()
{
	uint16_t fowardDistance;
    while(1)
    {
    	fowardDistance = DistanceSensor(1);
    	if(fowardDistance > 4000) break;
    	usleep(100000);
    }
    CarLight_Write(ALL_ON);
    Alarm_Write(ON);
    usleep(500000);
    CarLight_Write(ALL_OFF);
    Alarm_Write(OFF);
}
void Driver::goTunnel(Task* currentTask) {

    uint16_t leftSensor, rightSensor;
    float I_term = 0;
    float prev_error = 0;
    uint16_t direction = SteeringServoControl_Read();
    while(currentTask->tunnel)
    {
        rightSensor = DistanceSensor(2);
        leftSensor = DistanceSensor(6) + 100;
        printf("leftSensor %d  rightSensor %d\r\n", leftSensor, rightSensor);
        float error = rightSensor - leftSensor;
        float P_term = error * Kp;
                  I_term += Ki*error*dT;
        float D_term = Kd * (error - prev_error)/dT;

        float PID = (P_term + I_term + D_term)/100;
        direction = (int)(direction + PID);
        if(direction > 2000) direction = 2000;
        else if (direction < 1000) direction = 1000;
        printf("PID  %f\r\n", PID);
        printf("direction %d\r\n", direction);

        SteeringServoControl_Write(direction);
    }
}
