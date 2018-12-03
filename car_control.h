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
        void drive(struct thr_data *data, CVinfo cvInfo, SensorInfo sensorInfo);
        void pass(struct thr_data *data, CVinfo cvInfo, SensorInfo sensorInfo);
        void horizonPark(struct thr_data *data, SensorInfo sensorInfo);
        void verticalPark(struct thr_data *data, SensorInfo sensorInfo);
        void goTunnel();
        void waitStartSignal();
    private:
        struct DriveState {
        	bool isGoing;
        	bool isTurningRight;
        	bool isTurningLeft;
        	bool isEnteringCurve;

            bool isWhiteLineDetected;
        };
        struct ParkingState {
            uint8_t stage[4];
            bool horizontalDetected;
            bool verticalDetected;
            struct timeval startTime;
            struct timeval endTime;
        };
        DriveState driveState;
        ParkingState parkingState;

        float I_term;
        float prev_error;
        uint8_t emergencyTimeout;
        uint16_t globalDelay;
        uint8_t horizonParkingStage;
        uint8_t verticalParkingStage;
        uint8_t passStage;
        uint8_t greenLightDirection;

        bool TurnDetected(CVinfo cvInfo);
        bool LineDetected(CVinfo cvInfo);

        bool isWhiteLineDetected(SensorInfo sensorInfo);
        void updateParkingState(struct thr_data *data, SensorInfo sensorInfo, ParkingState *parkingState);
        void resetParkingState(ParkingState *parkingState);
        void requestHorizonParking(struct thr_data *data);
        void requestVerticalParking(struct thr_data *data);

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