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
    verticalParkingStage = 0;
    passStage = 0;
    gettimeofday(&parkingState.startTime, NULL);
}
void Driver::drive(struct thr_data *data, CVinfo cvInfo, SensorInfo sensorInfo)
{
    printf("Going %d Left %d Right %d EnteringCurve %d\r\n", driveState.isGoing, driveState.isTurningLeft, driveState.isTurningRight, driveState.isEnteringCurve);
    
    /**
     *  Required speed determination function.
     *  Structure required to indicate current state.
     */

    /**
     *  Emergency
     */
    if(cvInfo.isEmergency) {
        DesireSpeed_Write(0);
        emergencyTimeout = 100;
        return;
    }
    else if(emergencyTimeout) {
        emergencyTimeout--;
        return;
    }
    /**
     *  White Line detect handling
     */
    if( isWhiteLineDetected(sensorInfo) ) {

    }
    /**
     *  Parking
     */
    // Safety equipment required -> Once the parking is complete, skip it.
    if(parkingState.stage[3]) {
        resetParkingState(&parkingState);
        if(parkingState.horizontalDetected)     {
            requestHorizonParking(data);
        }
        else if(parkingState.verticalDetected)  {
            requestVerticalParking(data);
        }
    }
    updateParkingState(data, sensorInfo, &parkingState);
    /**
     *  Passing
     */
    if(cvInfo.isCarinFront_CV & (sensorInfo.distance[1] > 700 ))
        data->passRequest = true;
    /**
     *  Tunnel
     */
    if(cvInfo.isTunnelDetected) {
        //goTunnel();
        //return;
    }
    else {
        I_term = 0;
        prev_error = 0;
    }

    /**
     *  Normal Driving
     */
    // Code Cleanup Required.
    /*
    if( (sensorInfo.distance[1] > 2500) | (sensorInfo.distance[6] > 2500) ) {
        DesireSpeed_Write(0);
    }
    else */if(cvInfo.isPathStraight) DesireSpeed_Write(NORMAL_SPEED);
    else DesireSpeed_Write(SLOW_SPEED);

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
    uint8_t i;
    gettimeofday(&parkingState->endTime, NULL);
    uint32_t optime = ((parkingState->endTime.tv_sec - parkingState->startTime.tv_sec)*1000) 
                + ((int)parkingState->endTime.tv_usec/1000 - (int)parkingState->startTime.tv_usec/1000);
    printf("Parking timeout : %dms\r\n", optime);
    if(optime > PARKING_DETECT_TIMEOUT) {
        for(i = 0; i < 4; i++) parkingState->stage[i] = 0;
    }
    //R front detected
    if(sensorInfo.distance[2] > 650) {
        DesireSpeed_Write(SLOW_SPEED);
        parkingState->stage[0] = 1;
        gettimeofday(&parkingState->startTime, NULL);
    }
    //R back only detected
    if( parkingState->stage[0] ) {
        if( sensorInfo.distance[3] > 650 ) {
            if( sensorInfo.distance[2] > 650 ) {
                parkingState->horizontalDetected = true;
                parkingState->verticalDetected = false;
            }
            else {
                parkingState->horizontalDetected = false;
                parkingState->verticalDetected = true;
            }
            parkingState->stage[0] = 0;
            parkingState->stage[1] = 1;
        }
    }
    //R front only detected
    if( parkingState->stage[1] ) {
        if( (sensorInfo.distance[2] > 650) && (sensorInfo.distance[3] < 650) ) {
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
void Driver::pass(struct thr_data *data, CVinfo cvInfo, SensorInfo sensorInfo)
{
    switch(passStage)
    {
        case 0 :
            passStage++;
            Steering_Write(1000);
            DesireSpeed_Write(100);
            break;
        case 1 :
            if(cvInfo.isRoadClose) {
                passStage++;
            }
            break;
        case 2 :
            Steering_Write(1500);
            DesireSpeed_Write(0);
            break;
        case 3 :
            break;
    }
    printf("\r\n\r\npassStage %d\r\n\r\n", passStage);
    //passStage = 0;
    //data->passRequest = false;
}
void Driver::requestHorizonParking(struct thr_data *data)
{
    data->horizonParkingRequest = true;
}
void Driver::requestVerticalParking(struct thr_data *data)
{
    data->verticalParkingRequest = true;
}
void Driver::resetParkingState(ParkingState *parkingState)
{
    int i;
    for(i=0; i<4; i++) parkingState->stage[i] = 0;
}
void Driver::horizonPark(struct thr_data *data, SensorInfo sensorInfo)
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
            if(sensorInfo.distance[4] > 2200) {
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
            break;
        case 7 :
            if(sensorInfo.distance[4] > 2500) {
                horizonParkingStage++;
                Steering_Write(2000);
                DesireSpeed_Write(100);
            }
            break;
        /**
         *
         */
        case 8 :
            if(sensorInfo.distance[2] < 300) {
                horizonParkingStage++;
                Steering_Write(1000);
                DesireSpeed_Write(100);
            }
            break;
        case 9 :
            horizonParkingStage = 0;
            data->horizonParkingRequest = false;
            break;
    }
}
void Driver::verticalPark(struct thr_data *data, SensorInfo sensorInfo)
{
    switch(verticalParkingStage)
    {
        case 0 :
            verticalParkingStage++;
            Steering_Write(2000);
            DesireSpeed_Write(100);
            break;
        case 1 :
            if( sensorInfo.distance[3] < 300 ) {
                verticalParkingStage++;
                Steering_Write(1500);
                DesireSpeed_Write(-100);
            }
            break;
        case 2 :
            if( sensorInfo.distance[3] > 750 ) {
                verticalParkingStage++;
                Steering_Write(1000);
                DesireSpeed_Write(-100);
            }
            break;
        case 3 :
            if( sensorInfo.distance[3] > 2000 ) {
                verticalParkingStage++;
                Steering_Write(1500);
                DesireSpeed_Write(-100);
            }
            break;
        case 4 :
            if( sensorInfo.distance[4] > 2500 ) {
                verticalParkingStage++;
                Steering_Write(1500);
                DesireSpeed_Write(100);
            }
            break;
        case 5 : //escape
            if( sensorInfo.distance[2] < 300 ) {
                verticalParkingStage++;
                Steering_Write(1150);
                DesireSpeed_Write(100);
            }
            break;
        case 6 :
            if( sensorInfo.distance[5] < 1000 ) {
                verticalParkingStage = 0;
                data->verticalParkingRequest = false;
            }
            break;
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
