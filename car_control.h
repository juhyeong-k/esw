#include <stdint.h>
#include <unistd.h>

#define Kp  0.3
#define Ki  0
#define Kd  0.2
#define dT  0.1

extern "C" {
	#include "car_lib.h"
}
class Driver
{
    public:
    	Driver();
        void drive(CVinfo cvInfo, SensorInfo sensorInfo);
        void goTunnel();
        void waitStartSignal();
    private:
        struct DriveState {
        	bool isGoing;
        	bool isTurningRight;
        	bool isTurningLeft;
        	bool isEnteringCurve;
        };
        DriveState driveState;
        CVinfo lastCVinfo;

        float I_term;
        float prev_error;
        uint8_t emergencyTimeout;

        bool TurnDetected(CVinfo cvInfo);
        bool LineDetected(CVinfo cvInfo);

        bool Turning(DriveState driveState);
        void StateisEnteringCurve(struct DriveState *driveState);
        void StateisGoing(struct DriveState *driveState);
};
class Sensor
{
    public:
        Sensor();
        SensorInfo getInfo();
    private:
        SensorInfo sensorInfo;
};