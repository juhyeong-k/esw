#include "system_management.h"
#include <sys/time.h>
#include "car_control.h"

Driver::Driver()
{
    int i;
    for(i=0; i<4; i++) parkingState.stage[i] = 0;
    driveState = {1,0,};
    I_term = 0;
    prev_error = 0;
    emergencyTimeout = 0;
    horizonParkingStage = 0;
}
void Driver::drive(struct thr_data *data, CVinfo cvInfo, SensorInfo sensorInfo)
{
    printf("Going %d Left %d Right %d EnteringCurve %d\r\n", driveState.isGoing, driveState.isTurningLeft, driveState.isTurningRight, driveState.isEnteringCurve);

    // Emergency
    if(cvInfo.isEmergency) {
        DesireSpeed_Write(0);
        emergencyTimeout = 100;
        return;
    }
    else if(emergencyTimeout) {
        emergencyTimeout--;
        if(emergencyTimeout == 0) DesireSpeed_Write(DRIVE_SPEED);
        return;
    }
    // White Line detect handling
    if( isWhiteLineDetected(sensorInfo) ) {
        //request
    }

    if(parkingState.stage[3]) {
        resetParkingState(&parkingState);
        requestHorizonParking(data);
    }
    updateParkingState(data, sensorInfo, &parkingState);
    // Tunnel
    if(cvInfo.isTunnelDetected) {
        //goTunnel();
        //return;
    }
    else {
        I_term = 0;
        prev_error = 0;
    }

    // Normal Driving
    if(cvInfo.isDepartedLeft) {
        driveState.isGoing = false;
        driveState.isTurningRight = true;
        Steering_Write(1000);
        return;
    }
    else if(cvInfo.isDepartedRight) {
        driveState.isGoing = false;
        driveState.isTurningLeft = true;
        Steering_Write(2000);
        return;
    }

    if(driveState.isGoing) {
        if(TurnDetected(cvInfo)) {
            StateisEnteringCurve(&driveState);
            return;
        }
        else if( !Turning(driveState) ) {
            Steering_Write(cvInfo.direction);
            return;
        }
    }
    if(driveState.isTurningRight) {
        if( !LineDetected(cvInfo) ) {
            StateisGoing(&driveState);
            return;
        }
        else return;
    }
    if(driveState.isTurningLeft) {
        if( !LineDetected(cvInfo) ) {
            StateisGoing(&driveState);
            return;
        }
        else return;
    }
    if(driveState.isEnteringCurve) {
        if(cvInfo.isRoadClose) {
            if(cvInfo.direction < 1500) {
                driveState.isEnteringCurve = false;
                driveState.isTurningRight = true;
                Steering_Write(1000);
                return;
            }
            else if(cvInfo.direction > 1500) {
                driveState.isEnteringCurve = false;
                driveState.isTurningLeft = true;
                Steering_Write(2000);
                return;
            }
            else {
                Steering_Write(cvInfo.direction);
                return;
            }
        }
        else {
            Steering_Write(1500);
            return;
        }
    }
}
void Driver::updateParkingState(struct thr_data *data, SensorInfo sensorInfo, ParkingState *parkingState)
{
    gettimeofday(&parkingState->endTime, NULL);
    uint32_t optime = ((parkingState->endTime.tv_sec - parkingState->startTime.tv_sec)*1000) 
                + ((int)parkingState->endTime.tv_usec/1000 - (int)parkingState->startTime.tv_usec/1000);
    //R front detected
    if(sensorInfo.distance[2] > 750) {
        parkingState->stage[0] = 1;
        gettimeofday(&parkingState->startTime, NULL);
    }
    //R back only detected
    if( parkingState->stage[0] ) {
        if( (sensorInfo.distance[2] < 400) && (sensorInfo.distance[3] > 750) ) {
            parkingState->stage[0] = 0;
            parkingState->stage[1] = 1;
        }
    }
    //R front only detected
    if( parkingState->stage[1] ) {
        if( (sensorInfo.distance[2] > 750) && (sensorInfo.distance[3] < 750) ) {
            parkingState->stage[1] = 0;
            parkingState->stage[2] = 1;
        }
    }
    //R front not detected
    if( parkingState->stage[2]) {
        if( sensorInfo.distance[2] < 400 ) {
            parkingState->stage[2] = 0;
            parkingState->stage[3] = 1;
        }
    }
}
void Driver::requestHorizonParking(struct thr_data *data)
{
    data->horizonParkingRequest = true;
}
void Driver::resetParkingState(ParkingState *parkingState)
{
    int i;
    for(i=0; i<4; i++) parkingState->stage[i] = 0;
}
void Driver::horizonPark(SensorInfo sensorInfo)
{
    switch(horizonParkingStage)
    {
        case 0 :
            Steering_Write(2000);
            DesireSpeed_Write(100);
            horizonParkingStage++;
            break;
        case 1 :
            if(sensorInfo.distance[3] < 200) {
                horizonParkingStage++;
                Steering_Write(1500);
                DesireSpeed_Write(-100);
            }
            break;
        case 2 :
            if(sensorInfo.distance[2] > 500) {
                horizonParkingStage++;
                Steering_Write(2000);
                DesireSpeed_Write(100);
            }
            break;
        case 3 :
            if(sensorInfo.distance[3] < 200) {
                horizonParkingStage++;
                Steering_Write(1500);
                DesireSpeed_Write(-100);
            }
            break;
        case 4 : 
            if(sensorInfo.distance[2] > 1500) {
                horizonParkingStage++;
                Steering_Write(2000);
                DesireSpeed_Write(-100);
            }
        break;
        case 5 : 
            if(sensorInfo.distance[4] > 2000) {
                horizonParkingStage++;
                Steering_Write(1000);
                DesireSpeed_Write(100);
            }
        break;
        case 6 :
            if(sensorInfo.distance[1] > 2500) {
                horizonParkingStage++;
                Steering_Write(1500);
                DesireSpeed_Write(-100);
            }
        case 7 :
            if(sensorInfo.distance[4] > 2500) {
                horizonParkingStage++;
                DesireSpeed_Write(0);
                Alarm_Write(ON);
                CarLight_Write(ALL_ON);
            }
        /**
         *
         */
        /*
        case 7 :
        */ 
        case 8 : break;
    }
}
bool Driver::isWhiteLineDetected(SensorInfo sensorInfo)
{
    uint8_t i,temp;
    temp = 0;
    for(i = 1; i < 6; i++) {
        if( ~sensorInfo.line & (1 << i) ) temp++;
    }
    if(temp > 3) return true;
    else return false;
}
bool Driver::TurnDetected(CVinfo cvInfo)
{
    if(cvInfo.isForwadPathExist) return false;
    if(cvInfo.isRightTurnDetected | cvInfo.isLeftTurnDetected) return true;
    else return false;
}
bool Driver::LineDetected(CVinfo cvInfo)
{
    if(cvInfo.isLeftDetected | cvInfo.isRightDetected) return true;
    else return false;
}
bool Driver::Turning(DriveState driveState)
{
    if(driveState.isTurningRight | driveState.isTurningLeft) return true;
    else return false;
}
void Driver::StateisEnteringCurve(struct DriveState *driveState)
{
    driveState->isGoing = false;
    driveState->isEnteringCurve = true;
}
void Driver::StateisGoing(struct DriveState *driveState)
{
    driveState->isTurningLeft = false;
    driveState->isTurningRight = false;
    driveState->isGoing = true;
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

    Steering_Write(direction);
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
