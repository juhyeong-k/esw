#include "system_management.h"
#include "car_control.h"

Driver::Driver()
{
    lastCVinfo = {1500, 0,};
    driveState = {1,0,};
    I_term = 0;
    prev_error = 0;
}
void Driver::drive(CVinfo info)
{
    if(info.isTunnelDetected) {
        //goTunnel();
        //return;
    }
    else {
        I_term = 0;
        prev_error = 0;
    }

    if(info.isDepartedLeft) {
        SteeringServoControl_Write(1000);
        return;
    }
    else if(info.isDepartedRight) {
        SteeringServoControl_Write(2000);
        return;
    }

    if(driveState.isGoing) {
        if(info.isRightTurnDetected | info.isLeftTurnDetected) {
            driveState.isGoing = false;
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
            driveState.isGoing = true;
            return;
        }
        else return;
    }
    if(driveState.isTurningLeft) {
        if( !(info.isLeftDetected | info.isRightDetected) ) {
            driveState.isTurningLeft = false;
            driveState.isGoing = true;
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
void Driver::goTunnel() {
    uint16_t leftSensor, rightSensor;
    uint16_t direction = SteeringServoControl_Read();
    rightSensor = DistanceSensor(2);
    leftSensor = DistanceSensor(6) + 100;
    
    float error = rightSensor - leftSensor;
    float P_term = error * Kp;
            I_term += Ki*error*dT;
    float D_term = Kd * (error - prev_error)/dT;

    float PID = (P_term + I_term + D_term)/100;

    direction = (int)(direction + PID);
    if(direction > 2000) direction = 2000;
    else if (direction < 1000) direction = 1000;

    SteeringServoControl_Write(direction);
}


/**
 *
 */
Sensor::Sensor()
{
    sensorInfo.distance[0] = 0;
}
SensorInfo Sensor::getInfo()
{
    int i;
    sensorInfo.line = LineSensor_Read();
    for(i=1; i < 7; i++) {
        sensorInfo.distance[i] = DistanceSensor(i);
    }
    return sensorInfo;
}
