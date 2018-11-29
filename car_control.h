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
        void drive(CVinfo info);
        void goTunnel(Task* currentTask);
        void waitStartSignal();
    private:
        struct DriveState {
        	bool isGoingStraight;
        	bool isTurningRight;
        	bool isTurningLeft;

        	bool isEnteringCurve;
        };
        DriveState driveState;
        CVinfo lastCVinfo;

        void decisionMaking(CVinfo info);
        void waitRightDetect(CVinfo info);
        void waitLeftDetect(CVinfo info);
        void waitRoadClose(CVinfo info);
};