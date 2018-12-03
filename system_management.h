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

#define DRIVE_SPEED		100
/**
  * @Brief
  */

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
	uint16_t direction;
	bool isLeftTurnDetected;
	bool isRightTurnDetected;
	bool isLeftDetected;
	bool isRightDetected;
	bool isPathStraight;
	bool isPathLeft;
	bool isPathRight;
	bool isRoadClose;

	bool isTunnelDetected;
	bool isSafezoneDetected;

	bool isDepartedLeft;
	bool isDepartedRight;
	bool isLeftReinstation;
	bool isRightReinstation;
	bool isEmergency;
	bool isForwadPathExist;

	uint8_t greenLightReply;
};
struct SensorInfo {
	uint8_t line;
	int distance[7];
};
struct thr_data {
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

    bool horizonParkingRequest;
};
/**
  * @Brief
  */
#define CAMERA_X_SERVO     1500
#define CAMERA_Y_SERVO     1650

#define FRONT_UP			79
#define FRONT_DOWN		148
#define SIDE_UP			149
#define SIDE_DOWN			179

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

/**
  * @Brief
  */
#define YELLOW 0
#define yellow_HUE_MAX    45
#define yellow_HUE_MIN    7
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
#define green_HUE_MAX    80
#define green_HUE_MIN    45
#define green_SAT_MAX    255
#define green_SAT_MIN    100
#define green_VAL_MAX    255
#define green_VAL_MIN    10

#define WHITE 3
#define white_HUE_MAX    255
#define white_HUE_MIN    100
#define white_SAT_MAX    50
#define white_SAT_MIN    0
#define white_VAL_MAX    255
#define white_VAL_MIN    150
/**
  * @Brief
  */
#define lineDectectTHRESHOLD 7
#define colorDetectTHRESHOLD 700

/* CV.cpp */
#define ROAD_END_DETCTED_THRESHOLD		3
#define GET_DIRECTION_THRESHOLD		90 //90%

#define SIDE_DIRECTION_THRESHOLD		8 //90%
#define SIDE_DIRECTION_SIDE_UP			149 //90%
#define SIDE_DIRECTION_SIDE_DOWN		179 //90%

#define ISROADCLOSE_DISTANCE			35
#define ISROADCLOSE_THRESHOLD			5

#define FOWARD_PATH_EXIST_DISTANCE		80
#define FOWARD_PATH_EXIST_THRESHOLD	ISROADCLOSE_THRESHOLD

#define IS_SAFEZONE_CLOSE_THRESHOLD	3
#define SAFEZONE_CLOSE_UPLINE			129

#define REINSTATION_WIDTH				80

#define EMERGENCY_THRESHOLD				300

#define WHITELINE_DETECT_THRESHOLD		3 // Low -> sensetive

/**
 *  Traffic Lights
 */
#define GREENLIGHT_WIDTH_THRESHOLD		30
#define GREENLIGHT_DETECTED_THRESHOLD		10
#define GREENLIGHT_LEFT_THRESHOLD		    20

/**
 *  Tunnel
 */
#define TUNNEL_DETECT_THRESHOLD			3500