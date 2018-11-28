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
	bool isRightTurnDetected;
	bool isLeftTurnDetected;
	bool isRightDetected;
	bool isLeftDetected;
	bool isPathStraight;
	bool isPathRight;
	bool isPathLeft;
	bool isRoadClose;
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
#define yellow_HUE_MAX    40
#define yellow_HUE_MIN    8
#define yellow_SAT_MAX    255
#define yellow_SAT_MIN    50
#define yellow_VAL_MAX    255
#define yellow_VAL_MIN    30

#define RED 1
#define red_HUE_MAX    180
#define red_HUE_MIN    140
#define red_SAT_MAX    255
#define red_SAT_MIN    150
#define red_VAL_MAX    255
#define red_VAL_MIN    50


#define GREEN 2
#define green_HUE_MAX    70
#define green_HUE_MIN    50
#define green_SAT_MAX    255
#define green_SAT_MIN    50
#define green_VAL_MAX    255
#define green_VAL_MIN    50

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

#define ISROADCLOSE_DISTANCE			30
#define ISROADCLOSE_THRESHOLD			5