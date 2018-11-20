#include "system_management.h"
#include "car_control.h"

Driver::Driver()
{
    lastCVinfo = {1500, 0,};
}
void Driver::drive(CVinfo info)
{
    decisionMaking(info);
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