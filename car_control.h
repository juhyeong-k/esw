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
        CVinfo lastCVinfo;
        void decisionMaking(CVinfo info);
        void waitRightDetect(CVinfo info);
        void waitLeftDetect(CVinfo info);
        void waitRoadClose(CVinfo info);
};