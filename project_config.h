/*
 * @File   project_config.h
 * @Brief  Configurations for main.c
 *
 */

#define CAPTURE_IMG_W       1280
#define CAPTURE_IMG_H       720
#define CAPTURE_IMG_SIZE    (CAPTURE_IMG_W*CAPTURE_IMG_H*2) // YUYU : 16bpp
#define CAPTURE_IMG_FORMAT  "uyvy"
#define VPE_OUTPUT_W            320
#define VPE_OUTPUT_H            180

// display output & dump  format: rgb24, w:320, h:180
#define VPE_OUTPUT_IMG_SIZE    (VPE_OUTPUT_W*VPE_OUTPUT_H*3) // rgb24 : 24bpp
#define VPE_OUTPUT_RESOLUTION  VPE_OUTPUT_W*VPE_OUTPUT_H
#define VPE_OUTPUT_FORMAT       "bgr24"

// display output & dump  format: NV12, w:320, h:180
//#define VPE_OUTPUT_IMG_SIZE    (VPE_OUTPUT_W*VPE_OUTPUT_H*3/2) // NV12 : 12bpp
//#define VPE_OUTPUT_FORMAT       "nv12"

// display output & dump  format: yuyv, w:320, h:180
//#define VPE_OUTPUT_IMG_SIZE    (VPE_OUTPUT_W*VPE_OUTPUT_H*2)
//#define VPE_OUTPUT_FORMAT       "yuyv"

/**
  * @Brief  Values for detect_Yellow_color()
  */
#define maxHue    45
#define minHue    25
#define maxSat    250
#define minSat    30
#define maxVal    255
#define minVal    50
