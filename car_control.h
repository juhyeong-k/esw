#include <stdint.h>
#include <unistd.h>

extern "C" {
	#include "car_lib.h"
}
class Driver
{
    public:
    	Driver();
        void drive(CVinfo info);
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