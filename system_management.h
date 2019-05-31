/*
 * @File   project_config.h
 * @Brief  Configurations for main.c
 *
 */
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/sysinfo.h>

#define NORMAL_SPEED		130 // 130
#define SLOW_SPEED		105 // 100
#define PARKING_SPEED	90 // 100

/**
 *  Normal Drive
 */
#define FRONT_UP			79
#define FRONT_DOWN		148
#define SIDE_UP			149
#define SIDE_DOWN			179

#define MAX_DIRECTION 1997
#define MIN_DIRECTION 1003

#define SIDE_DIRECTION_THRESHOLD		8	//90%

#define GET_DIRECTION_THRESHOLD			90 	//90%
#define LINE_DETECT_THRESHOLD 			7
#define SLOPE_DIVIDE_FACTOR	  			2

#define ISROADCLOSE_DISTANCE			40
#define ISROADCLOSE_THRESHOLD			5

#define FOWARD_PATH_EXIST_DISTANCE		80
#define FOWARD_PATH_EXIST_THRESHOLD	ISROADCLOSE_THRESHOLD

#define REINSTATION_WIDTH				80

#define WHITELINE_DETECT_THRESHOLD_CV	10
#define WHITELINE_DETECT_THRESHOLD		6 // Low -> sensetive
/**
 *  Emergency
 */
#define EMERGENCY_THRESHOLD				6000
/**
 *  Traffic Lights
 */
#define GREENLIGHT_WIDTH_THRESHOLD			10 // High -> sensitive
#define GREENLIGHT_DETECTED_THRESHOLD		5 // Low -> sensitive
#define GREEN_DETECT_PIXEL_THRESHOLD      80
#define RED_DETECT_PIXEL_THRESHOLD      80
#define IS_GREENLIGHT_ROADCLOSE_DISTANCE	60 // Bigger than ISROADCLOSE_THRESHOLD
/**
 *  Tunnel
 */
#define TUNNEL_DETECT_THRESHOLD			3500
/**
 *  Parking
 */
#define PARKING_DETECT_TIMEOUT			30000
/**
 *  Roundabout
 */
#define ROUNDABOUT_TIMES					20
/**
 *	Passing
 */
#define PASSING_WHITE_DETECT_HEIGHT		79    // FRONT_UP
#define PASSING_WHITE_DETECT_THRESHOLD	200    // FRONT_UP
#define FIND_EXIT_FRONT_UP					78
#define FIND_EXIT_FRONT_DOWN				179
#define IS_SIDE_ROADCLOSE_DISTANCE		40	   // Bigger than ISROADCLOSE_THRESHOLD, for Passing
#define IS_SIDE_ROADCLOSE_DISTANCE2	80

#define isCarinFront_CV_UP				90
#define isCarinFront_CV_DOWN			179
#define isCarinFront_CV_THRESHOLD		3
/**
 *  Safe Zone
 */
#define IS_SAFEZONE_CLOSE_THRESHOLD	10
#define SAFEZONE_UPLINE					79
#define SAFEZONE_DOWNLINE				169
/**
 *  Down Hill
 */
#define DOWNHILL_CHECK_UP			49
#define DOWNHILL_CHECK_DOWN			179
#define DOWNHILL_DITECT_THRESHOLD	125
/**
*  HSV
*/
#define YELLOW 0
#define yellow_HUE_MAX    45
#define yellow_HUE_MIN    5
#define yellow_SAT_MAX    255
#define yellow_SAT_MIN    30
#define yellow_VAL_MAX    255
#define yellow_VAL_MIN    50

#define RED 1
#define red_HUE_MAX    180
#define red_HUE_MIN    140
#define red_SAT_MAX    255
#define red_SAT_MIN    150
#define red_VAL_MAX    255
#define red_VAL_MIN    50

#define GREEN 2
#define green_HUE_MAX    90
#define green_HUE_MIN    45
#define green_SAT_MAX    255
#define green_SAT_MIN    60
#define green_VAL_MAX    255
#define green_VAL_MIN    10

#define WHITE 3
#define white_HUE_MAX    255
#define white_HUE_MIN    90
#define white_SAT_MAX    70
#define white_SAT_MIN    0
#define white_VAL_MAX    255
#define white_VAL_MIN    190

/**
  * @Brief
  */
#define CAMERA_X_SERVO     1500
#define CAMERA_Y_SERVO     1650

#define CAPTURE_IMG_W       1280
#define CAPTURE_IMG_H       720
#define CAPTURE_IMG_SIZE    (CAPTURE_IMG_W*CAPTURE_IMG_H*2) // YUYU : 16bpp
#define CAPTURE_IMG_FORMAT  "uyvy"
#define VPE_OUTPUT_W            320
#define VPE_OUTPUT_H            180

// display output & dump  format: bgr24, w:320, h:180
#define bgr24
#define VPE_OUTPUT_IMG_SIZE    (VPE_OUTPUT_W*VPE_OUTPUT_H*3) // bgr24 : 24bpp
#define VPE_OUTPUT_FORMAT       "bgr24"
#define VPE_OUTPUT_RESOLUTION  VPE_OUTPUT_W*VPE_OUTPUT_H

// display output & dump  format: NV12, w:320, h:180
//#define VPE_OUTPUT_IMG_SIZE    (VPE_OUTPUT_W*VPE_OUTPUT_H*3/2) // NV12 : 12bpp
//#define VPE_OUTPUT_FORMAT       "nv12"

// display output & dump  format: yuyv, w:320, h:180
//#define VPE_OUTPUT_IMG_SIZE    (VPE_OUTPUT_W*VPE_OUTPUT_H*2)
//#define VPE_OUTPUT_FORMAT       "yuyv"

class System_resource
{
	public:
		System_resource();
		uint64_t getVirtualMemUsed();
		uint64_t getTotalVirtualMem();
		uint64_t getTotalPhysMem();
		uint64_t getPhysMemUsed();
	private:
		struct sysinfo memInfo;
		long long totalVirtualMem;
		long long virtualMemUsed;
		long long totalPhysMem;
		long long physMemUsed;
};
struct CVinfo {
	/**
	 *  Normal Drive
	 */
	uint16_t direction;
	bool isLeftTurnDetected;
	bool isRightTurnDetected;
	bool isLeftDetected;
	bool isRightDetected;
	bool isPathStraight;
	bool isPathLeft;
	bool isPathRight;
	bool isRoadClose;
	bool isDepartedLeft;
	bool isDepartedRight;
	bool isLeftReinstation;
	bool isRightReinstation;
	bool isForwadPathExist;
	bool isWhiteLineDetected_CV;
	/**
	 *  Emergency
	 */
	bool isEmergency;
	/**
	 *  Passing
	 */
	bool isCarinFront_CV;
	bool isSideRoadClose;
	bool isSideRoadClose2;
	uint16_t exitDirection;
	uint16_t passNumber;
	/**
	 *  Traffic Lights
	 */
	bool isTrafficLightsGreen;
	bool isTrafficLightsRed;
	uint8_t greenLightReply;
	bool isGreenLightRoadClose;
    /**
     *  Tunnel
     */
	bool isTunnelDetected;
	bool isUpperSafezoneDetected;
	bool isLowerSafezoneDetected;
	/**
	 *  Roundabout
	 */
	bool isWhiteRightDetected;
	/**
	 *  Down Hill
	 */
	bool isDownHillDetected;
};
struct Mission {
	bool isEmergencyEnd;
	bool isRoundaboutEnd;
	bool isHorizontalEnd;
	bool isVerticalEnd;
	bool isPassEnd;
	bool isDownHillEnd;
};
struct SensorInfo {
	uint8_t line;
	int distance[7];
};
struct thr_data {
    struct timeval systemTime;

    struct display *disp;
    struct v4l2 *v4l2;
    struct vpe *vpe;
    struct buffer **input_bufs;
    struct buffer *capt;
    int index;

    int msgq_id;
    bool bfull_screen; // true : 480x272 disp 화면에 맞게 scale 그렇지 않을 경우 false.
    bool bstream_start; // camera stream start 여부
    pthread_t threads[4];

    CVinfo cvResult;
    SensorInfo sensorInfo;
    Mission mission;

    bool horizonParkingRequest;
    bool verticalParkingRequest;
    bool passRequest;
    bool roundaboutRequest;
};
