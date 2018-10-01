#include <stdint.h>
#include <unistd.h>
extern "C" {
	#include "car_lib.h"
}
class Driver
{
    public:
        void waitStartSignal();
    private:
};