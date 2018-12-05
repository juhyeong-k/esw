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
        void waitStartSignal();
        void drive(struct thr_data *data, CVinfo cvInfo, SensorInfo sensorInfo);
        void roundabout(struct thr_data *data, CVinfo cvInfo, SensorInfo sensorInfo);
        void pass(struct thr_data *data, CVinfo cvInfo, SensorInfo sensorInfo);
        void passLeft(struct thr_data *data, CVinfo cvInfo, SensorInfo sensorInfo);
        void passRight(struct thr_data *data, CVinfo cvInfo, SensorInfo sensorInfo);
        void horizonPark(struct thr_data *data, SensorInfo sensorInfo);
        void verticalPark(struct thr_data *data, SensorInfo sensorInfo);
        void goTunnel();
    private:
        struct DriveState {
        	bool isGoing;
        	bool isTurningRight;
        	bool isTurningLeft;
        	bool isEnteringCurve;

            bool isWhiteLineDetected;
        };
        struct ParkingState {
            bool horizontalDetected;
            bool verticalDetected;
            struct timeval startTime;
            struct timeval endTime;
        };
        struct RoundaboutState {
            struct timeval startTime;
            struct timeval endTime;
        };
        struct PassState {
            struct timeval startTime;
            struct timeval endTime;
            uint16_t leftNumber;
            uint16_t rightNumber;
            bool direction;
        };
        DriveState driveState;
        ParkingState parkingState;
        RoundaboutState roundaboutState;
        PassState passState;
        // Tunnel PID
        float I_term;
        float prev_error;
        // Emergency delay
        uint8_t emergencyTimeout;
        // global delay
        uint16_t globalDelay;
        // Mission Stage
        uint8_t parkingStage;
        uint8_t horizonParkingStage;
        uint8_t verticalParkingStage;
        uint8_t passStage;
        uint8_t passEnteringStage;
        uint8_t roundaboutStage;
        // Green Light -> 1 : Left, 2 : Right
        uint8_t greenLightDirection;

        bool isWhiteLineDetected(SensorInfo sensorInfo);
        // Parking
        void updateParkingState(struct thr_data *data, CVinfo cvInfo, SensorInfo sensorInfo, ParkingState *parkingState);
        void resetParkingState(ParkingState *parkingState);
        void requestHorizonParking(struct thr_data *data);
        void requestVerticalParking(struct thr_data *data);

        // Normal Drive
        bool Turning(DriveState driveState);
        void StateisEnteringCurve(struct DriveState *driveState);
        void StateisGoing(struct DriveState *driveState);
        bool TurnDetected(CVinfo cvInfo);
        bool LineDetected(CVinfo cvInfo);

        uint32_t getOptime(struct timeval startTime, struct timeval endTime);

        bool msDelay(uint16_t mstime);
};
class Sensor
{
    public:
        Sensor();
        SensorInfo getInfo();
    private:
        SensorInfo sensorInfo;
};