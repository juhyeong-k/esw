#include "system_management.h"
#include <sys/time.h>
#include "car_control.h"

Driver::Driver()
{
    driveState = {1,0,};
    I_term = 0;
    prev_error = 0;
    emergencyTimeout = 0;
    globalDelay = 0;

    parkingStage = 0;
    horizonParkingStage = 0;
    verticalParkingStage = 0;
    passEnteringStage = 0;
    passStage = 0;
    roundaboutStage = 0;

    greenLightDirection = 0;

    gettimeofday(&parkingState.startTime, NULL);
    gettimeofday(&roundaboutState.startTime, NULL);
    gettimeofday(&passState.startTime, NULL);
}
void Driver::drive(struct thr_data *data, CVinfo cvInfo, SensorInfo sensorInfo)
{
    printf("\r\n--- Drive state\r\n");
    printf("\tGoing %d Left %d Right %d EnteringCurve %d\r\n", driveState.isGoing, driveState.isTurningLeft, driveState.isTurningRight, driveState.isEnteringCurve);
    
    /**
     *  Required speed determination function.
     *  Structure required to indicate current state.
     */

    /**
     *  Emergency
     */
    if (cvInfo.isEmergency && !data->mission.isEmergencyEnd) {
        DesireSpeed_Write(0);
        emergencyTimeout = 50;
        parkingStage = 0;
        return;
    }
    else if(emergencyTimeout == 1) {
        if(!data->mission.isEmergencyEnd) DesireSpeed_Write(NORMAL_SPEED);
        data->mission.isEmergencyEnd = true;
        parkingStage = 0;
        emergencyTimeout = 0;
        return;
    }
    else if(emergencyTimeout) {
        emergencyTimeout--;
        return;
    }
    /**
     *  Tunnel
     */
    if( (1200 < sensorInfo.distance[2]) && (1200 < sensorInfo.distance[6]) )
        CarLight_Write(ALL_ON);
    else
        CarLight_Write(ALL_OFF);
    if(cvInfo.isTunnelDetected) {
        //goTunnel();
        //return;
    }
    else {
        I_term = 0;
        prev_error = 0;
    }
    /**
     *  White Line detect handling
     */
    if( isWhiteLineDetected(sensorInfo) ) {

    }
    /**
     *  Roundabout
     */
    if( cvInfo.isWhiteLineDetected_CV ) {
        gettimeofday(&roundaboutState.startTime, NULL);
    }
    if( isWhiteLineDetected(sensorInfo) && data->mission.isEmergencyEnd ) {
        gettimeofday(&roundaboutState.endTime, NULL);
        uint32_t optime = getOptime(roundaboutState.startTime, roundaboutState.endTime);
        if(optime < 7000) {
            if(!cvInfo.isLeftDetected && !cvInfo.isRightDetected) {
                gettimeofday(&passState.startTime, NULL);
                data->roundaboutRequest = true;
            }
        }
    }
    /**
     *  Parking
     */
    // Safety equipment required -> Once the parking is complete, skip it.
    if(parkingStage == 6) {
        resetParkingState(&parkingState);
        if(parkingState.horizontalDetected)     {
            requestHorizonParking(data);
        }
        else if(parkingState.verticalDetected)  {
            requestVerticalParking(data);
        }
    }
    updateParkingState(data, cvInfo, sensorInfo, &parkingState);
    /**
     *  Passing
     */
    if(cvInfo.isCarinFront_CV) {
        if( data->mission.isRoundaboutEnd && data->mission.isHorizontalEnd
            && data->mission.isVerticalEnd && data->mission.isRoundaboutEnd) {
                gettimeofday(&passState.endTime, NULL);
                uint32_t passOptime = getOptime(passState.startTime, passState.endTime);
                if(passOptime > 35000) data->passRequest = true;
            }
    }
    /**
     */
    // Code Cleanup Required.
    if(!parkingStage) {
        if(cvInfo.isPathStraight) DesireSpeed_Write(NORMAL_SPEED);
        else DesireSpeed_Write(SLOW_SPEED);
    }
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
        if( !LineDetected(cvInfo) | (sensorInfo.distance[2] > 2775) ) {
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
                Steering_Write(MIN_DIRECTION);
                return;
            }
            else if(cvInfo.direction > 1500) {
                driveState.isEnteringCurve = false;
                driveState.isTurningLeft = true;
                Steering_Write(MAX_DIRECTION);
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
uint32_t Driver::getOptime(struct timeval startTime, struct timeval endTime)
{
    uint32_t optime = ((endTime.tv_sec - startTime.tv_sec)*1000) 
                + ((int)endTime.tv_usec/1000 - (int)startTime.tv_usec/1000);
    return optime;
}
void Driver::updateParkingState(struct thr_data *data, CVinfo cvInfo, SensorInfo sensorInfo, ParkingState *parkingState)
{
    gettimeofday(&parkingState->endTime, NULL);
    uint32_t optime = ((parkingState->endTime.tv_sec - parkingState->startTime.tv_sec)*1000) 
                + ((int)parkingState->endTime.tv_usec/1000 - (int)parkingState->startTime.tv_usec/1000);
    if((parkingStage < 3 ) && !cvInfo.isPathStraight) parkingStage = 0;
    //printf("Parking timeout : %dms\r\n", optime);
    if(optime > PARKING_DETECT_TIMEOUT) {
        parkingStage = 0;
    }
    //R front detected
    if(sensorInfo.distance[2] > 600) {
        gettimeofday(&parkingState->startTime, NULL);
        if(!data->mission.isHorizontalEnd && !data->mission.isVerticalEnd)
            DesireSpeed_Write(PARKING_SPEED);
    }
    if(parkingStage == 0) {
        if(sensorInfo.distance[2] > 600) {
            parkingStage = 1;
        }
    }
    //R back only detected
    if( parkingStage == 1 ) {
        if( sensorInfo.distance[3] > 600 ) {
            parkingStage = 2;
        }
    }
    if( parkingStage == 2 ) {
        if( (sensorInfo.distance[2] < 650) && (sensorInfo.distance[3] > 650) ) {
            parkingStage = 3;
        }
    }  /*
    
    */
    //R front only detected
    if( parkingStage == 3 ) {
        if( (sensorInfo.distance[2] > 650) && (sensorInfo.distance[3] < 650) ) {
            parkingStage = 4;
        }
    }
    if( parkingStage == 4 ) {
        if( sensorInfo.distance[3] > 600 ) {
            if( sensorInfo.distance[2] > 600 ) {
                parkingState->horizontalDetected = true;
                parkingState->verticalDetected = false;
            }
            else {
                parkingState->horizontalDetected = false;
                parkingState->verticalDetected = true;
            }
            parkingStage = 5;
        }
    }
    //R front not detected
    if( parkingStage == 5 ) {
        if( sensorInfo.distance[2] < 400 ) {
            parkingStage = 6;
        }
    }
    printf("\r\nparkingStage : %d\r\n", parkingStage);
}
void Driver::roundabout(struct thr_data *data, CVinfo cvInfo, SensorInfo sensorInfo)
{
    switch(roundaboutStage)
    {
        case 0 :
            DesireSpeed_Write(80);
            Steering_Write(2000);
            if( msDelay(500) ) roundaboutStage++;
            break;
        case 1 :
            DesireSpeed_Write(0);
            globalDelay = 0;
            if( sensorInfo.distance[1] > 800 ) {
                roundaboutStage++;
            }
            break;
        case 2 :
            if( msDelay(10000) ) roundaboutStage++;
            break;
        case 3 :
            drive(data, cvInfo, sensorInfo);
            if(cvInfo.isLeftTurnDetected | ( sensorInfo.distance[1] > 2500 )) {
                roundaboutStage++;
            }
            break;
        case 4 :
            Steering_Write(1000);
            if( msDelay(200) ) roundaboutStage++;
            break;
        case 5 :
            Steering_Write(2000);
            DesireSpeed_Write(0);
            Steering_Write(cvInfo.direction);
            if( sensorInfo.distance[4] > 700 ) {
                roundaboutStage++;
            }
            break;
        case 6 :
            if(cvInfo.direction < 1700) 
                Steering_Write(1700);
            else 
                Steering_Write(cvInfo.direction);
            DesireSpeed_Write(140);
            if(cvInfo.isWhiteRightDetected) {
                roundaboutStage++;
            }
            break;
        case 7 :
            Steering_Write(cvInfo.direction);
            DesireSpeed_Write(120);
            if(!cvInfo.isWhiteRightDetected) {
                roundaboutStage++;
            }
            break;
        case 8 :
            data->mission.isRoundaboutEnd = true;
            data->roundaboutRequest = false;
            break;
    }
}
void Driver::pass(struct thr_data *data, CVinfo cvInfo, SensorInfo sensorInfo)
{
    switch(passEnteringStage)
    {
        case 0 :
            Steering_Write(2000);
            if( msDelay(300) ) passEnteringStage++;
            break;
        case 1 :
            Steering_Write(1500);
            DesireSpeed_Write(0);
            passEnteringStage++;
            break;
        case 2 :
            Steering_Write(1500);
            DesireSpeed_Write(80);
            if( msDelay(700) )passEnteringStage++;
            break;
        case 3 :
            DesireSpeed_Write(0);
            CameraXServoControl_Write(1300);
            if( msDelay(3000) ) passEnteringStage++;
            break;
        case 4 :
            passState.leftNumber = cvInfo.passNumber;
            CameraXServoControl_Write(1700);
            if( msDelay(3000) ) passEnteringStage++;
            break;
        case 5 :
            passState.rightNumber = cvInfo.passNumber;
            CameraXServoControl_Write(1500);
            if( msDelay(2000) ) passEnteringStage++;
            break;
        case 6 :
            if(passState.leftNumber > passState.rightNumber)    passLeft(data, cvInfo, sensorInfo);
            else                                                       passRight(data, cvInfo, sensorInfo);
            break;
    }
}
void Driver::passLeft(struct thr_data *data, CVinfo cvInfo, SensorInfo sensorInfo)
{
    switch(passStage)
    {
        case 0 : // Front Left.
            Steering_Write(2000);
            DesireSpeed_Write(100);
            if(msDelay(1500)) passStage++;
            break;
        case 1 : // When SideRoad is close, go Front Right.
            Steering_Write(1500);
            DesireSpeed_Write(70);
            if(cvInfo.isSideRoadClose) passStage++;
            break;
        case 2 : // When road direction < 1500, Hand over.
            Steering_Write(1100);
            DesireSpeed_Write(70);
            if(cvInfo.direction >= 1500) passStage++;
            break;
        case 3 :
            Steering_Write(1500);
            DesireSpeed_Write(0);
            if(msDelay(500)) passStage++;
            break;
        case 4 :
            Steering_Write(cvInfo.direction);
            DesireSpeed_Write(70);
            if(cvInfo.isSideRoadClose2) passStage++;
            break;
        case 5 :
            Steering_Write(1000);
            if( msDelay(2500) ) passStage++;
            break;
        case 6 :
            Steering_Write(1500);
            if( msDelay(500) ) passStage++;
            break;
        case 7 :
            Steering_Write(1500);
            DesireSpeed_Write(70);
            if(cvInfo.isSideRoadClose) passStage++;
            break;
        case 8 :
            Steering_Write(2000);
            if( msDelay(2500) ) passStage++;
            break;
        case 9 :
            Steering_Write(cvInfo.direction);
            passStage++;
            break;
        case 10 :
            DesireSpeed_Write(0);
            Alarm_Write(ON);
            CameraYServoControl_Write(1550);
            if( msDelay(1000) ) passStage++;
            break;
        case 11 :
            Alarm_Write(OFF);
            if( cvInfo.isTrafficLightsRed ) passStage++;
            break;
        case 12 :
            if( cvInfo.isTrafficLightsGreen )   passStage++;
            break;
        case 13 :
            if( msDelay(1500) ) passStage++;
            break;
        case 14 :
            greenLightDirection = cvInfo.greenLightReply;
            if(greenLightDirection == 1) { // Left
                CameraYServoControl_Write(1650);
                passStage++;
            }
            else if(greenLightDirection == 2) { // Right
                CameraYServoControl_Write(1650);
                passStage++;
            }
            break;
        case 15 :
            if( msDelay(2000) ) passStage++;
            break;
        case 16 : // When green light road is close, turn. 
            Steering_Write(1500);
            DesireSpeed_Write(80);
            if(cvInfo.isGreenLightRoadClose) {
                if(greenLightDirection == 1)        Steering_Write(2000);
                else if(greenLightDirection == 2)  Steering_Write(1000);
                passStage++;
            }
            break;
        case 17 :
            if( msDelay(4500) ) passStage++;
            break;
        case 18 : // Go foward
            Steering_Write(1500);
            if( msDelay(2000) ) passStage++;
            break;
        case 19 :
            DesireSpeed_Write(0);
            break;
    }
}
void Driver::Driver::passRight(struct thr_data *data, CVinfo cvInfo, SensorInfo sensorInfo)
{
    switch(passStage)
    {
        case 0 : // Front Left.
            Steering_Write(1000);
            DesireSpeed_Write(100);
            if(msDelay(1500)) passStage++;
            break;
        case 1 : // When SideRoad is close, go Front Right.
            Steering_Write(1500);
            DesireSpeed_Write(70);
            if(cvInfo.isSideRoadClose) passStage++;
            break;
        case 2 : // When road direction < 1500, Hand over.
            Steering_Write(1900);
            DesireSpeed_Write(70);
            if(cvInfo.direction <= 1500) passStage++;
            break;
        case 3 :
            Steering_Write(1500);
            DesireSpeed_Write(0);
            if(msDelay(500)) passStage++;
            break;
        case 4 :
            Steering_Write(cvInfo.direction);
            DesireSpeed_Write(70);
            if(cvInfo.isSideRoadClose2) passStage++;
            break;
        case 5 :
            Steering_Write(2000);
            if( msDelay(2500) ) passStage++;
            break;
        case 6 :
            Steering_Write(1500);
            if( msDelay(500) ) passStage++;
            break;
        case 7 :
            Steering_Write(1500);
            DesireSpeed_Write(70);
            if(cvInfo.isSideRoadClose) passStage++;
            break;
        case 8 :
            Steering_Write(1000);
            if( msDelay(2500) ) passStage++;
            break;
        case 9 :
            Steering_Write(cvInfo.direction);
            passStage++;
            break;
        case 10 :
            DesireSpeed_Write(0);
            Alarm_Write(ON);
            CameraYServoControl_Write(1550);
            if( msDelay(1000) ) passStage++;
            break;
        case 11 :
            Alarm_Write(OFF);
            if( cvInfo.isTrafficLightsRed ) passStage++;
            break;
        case 12 :
            if( cvInfo.isTrafficLightsGreen )   passStage++;
            break;
        case 13 :
            if( msDelay(1500) ) passStage++;
            break;
        case 14 :
            greenLightDirection = cvInfo.greenLightReply;
            if(greenLightDirection == 1) { // Left
                CameraYServoControl_Write(1650);
                passStage++;
            }
            else if(greenLightDirection == 2) { // Right
                CameraYServoControl_Write(1650);
                passStage++;
            }
            break;
        case 15 :
            if( msDelay(2000) ) passStage++;
            break;
        case 16 : // When green light road is close, turn. 
            Steering_Write(1500);
            DesireSpeed_Write(80);
            if(cvInfo.isGreenLightRoadClose) {
                if(greenLightDirection == 1)        Steering_Write(2000);
                else if(greenLightDirection == 2)  Steering_Write(1000);
                passStage++;
            }
            break;
        case 17 :
            if( msDelay(4500) ) passStage++;
            break;
        case 18 : // Go foward
            Steering_Write(1500);
            if( msDelay(2000) ) passStage++;
            break;
        case 19 :
            DesireSpeed_Write(0);
            break;
    }
}
void Driver::requestHorizonParking(struct thr_data *data) {
    data->horizonParkingRequest = true;
}
void Driver::requestVerticalParking(struct thr_data *data) {
    data->verticalParkingRequest = true;
}
void Driver::resetParkingState(ParkingState *parkingState)
{
    if(parkingStage != 4)
        parkingStage = 0;
}
void Driver::horizonPark(struct thr_data *data, SensorInfo sensorInfo)
{
    switch(horizonParkingStage)
    {
        case 0 :
            Steering_Write(1500);
            DesireSpeed_Write(-90);
            if( msDelay(1400) ) horizonParkingStage++;
            break;
        case 1 : // Foward Left
            Steering_Write(2000);
            DesireSpeed_Write(90);
            if(sensorInfo.distance[3] < 400) horizonParkingStage++;
            break;
        case 2 : // Backward
            Steering_Write(1300);
            DesireSpeed_Write(-90);
            if(sensorInfo.distance[2] > 700) horizonParkingStage++;
            break;
        case 3 : // Foward Left
            Steering_Write(2000);
            DesireSpeed_Write(90);
            if(sensorInfo.distance[3] < 200) horizonParkingStage++;
            break;
        case 4 : // Backward
            Steering_Write(1300);
            DesireSpeed_Write(-90);
            if(msDelay(2000)) horizonParkingStage++;
            break;
        case 5 : // Backward Left
            Steering_Write(2000);
            DesireSpeed_Write(-90);
            if(sensorInfo.distance[4] > 2200) horizonParkingStage++;
            break;
        case 6 : // Foward Right
            Steering_Write(1500);
            DesireSpeed_Write(90);
            if(sensorInfo.distance[1] > 2200) horizonParkingStage++;
            break;
        case 7 : // Backward
            Steering_Write(1500);
            DesireSpeed_Write(-90);
            if(sensorInfo.distance[4] > 2200) horizonParkingStage++;
            break;
        case 8 :
            Alarm_Write(ON);
            DesireSpeed_Write(0);
            if( msDelay(2000) ) {
                Alarm_Write(OFF);
                horizonParkingStage++;
            }
            break;
        case 9 : // Foward Left
            Steering_Write(2000);
            DesireSpeed_Write(90);
            if( msDelay(500) ) horizonParkingStage++;
            break;
        case 10 :
            Steering_Write(1500);
            DesireSpeed_Write(-90);
            if( msDelay(500) ) horizonParkingStage++;
            break;
        case 11 : // Foward Left
            Steering_Write(2000);
            DesireSpeed_Write(90);
            if( sensorInfo.distance[2] < 400 ) horizonParkingStage++;
            break;
        case 12 : // Foward Right
            Steering_Write(1000);
            DesireSpeed_Write(90);
            if( msDelay(2500) ) horizonParkingStage++;
            break;
        case 13 :
            /****************************************************/
            data->mission.isHorizontalEnd = true;
            data->horizonParkingRequest = false;
            break;
    }
}
void Driver::verticalPark(struct thr_data *data, SensorInfo sensorInfo)
{
    switch(verticalParkingStage)
    {
        case 0 :
            Steering_Write(1500);
            DesireSpeed_Write(90);
            if( msDelay(100) )  verticalParkingStage++;
            break;
        case 1 : // Backward Right
            Steering_Write(1000);
            DesireSpeed_Write(-90);
            if( msDelay(3500) )  verticalParkingStage++;
            break;
        case 2 : // Backward
            Steering_Write(1500);
            DesireSpeed_Write(-90);
            if( sensorInfo.distance[4] > 2000 ) verticalParkingStage++;
            break;
        case 3 :
            DesireSpeed_Write(0);
            Alarm_Write(ON);
            if( msDelay(2000) ) {
                Alarm_Write(OFF);
                verticalParkingStage++;
            }
            break;
        case 4 : // Foward
            Steering_Write(1500);
            DesireSpeed_Write(90);
            if( sensorInfo.distance[2] < 300 ) verticalParkingStage++;
            break;
        case 5 : // Foward Right
            if( msDelay(400) )  verticalParkingStage++;
            break;
        case 6 : // Foward Right
            Steering_Write(1000);
            DesireSpeed_Write(90);
            if( msDelay(3500) )  verticalParkingStage++;
            break;
        case 7 : // Untill LB not detected
            data->mission.isVerticalEnd = true;
            data->verticalParkingRequest = false;
            break;
    }
}
bool Driver::isWhiteLineDetected(SensorInfo sensorInfo)
{
    uint8_t i,temp;
    temp = 0;
    for(i = 0; i < 7; i++) {
        if( ~sensorInfo.line & (1 << i) ) temp++;
    }
    if(temp > WHITELINE_DETECT_THRESHOLD) return true;
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
    while(1) {
    	fowardDistance = DistanceSensor(1);
    	if(fowardDistance > 4000) break;
    	usleep(100000);
    }
    CarLight_Write(ALL_ON);
    Alarm_Write(ON);
    usleep(500000);
    CarLight_Write(ALL_OFF);
    Alarm_Write(OFF);
    usleep(500000);
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
bool Driver::msDelay(uint16_t mstime)
{
    globalDelay++;
    if(globalDelay == mstime*3/100) {
        globalDelay = 0;
        return true;
    }
    else return false;
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
