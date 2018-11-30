/**
  * @brief
  */
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <fstream>

class BGR24_to_HSV
{
    public:
        void bgr24_to_hsv(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
    private:
        uint8_t getMaxBGR_VBGR(uint8_t b, uint8_t g, uint8_t r, uint8_t *V_BGR);
        uint8_t getMinBGR(uint8_t b, uint8_t g, uint8_t r);
};
class Draw
{
	public:
		void mixColor(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3], 
		uint8_t red[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t white[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t des[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
		void horizontal_line(uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y, uint16_t x_start, uint16_t x_end);
		void vertical_line(uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t x, uint16_t y_start, uint16_t y_end);
		void dot(uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t x, uint16_t y);
};
class colorFilter
{
	public:
		colorFilter(uint8_t colorName);
		void detectColor(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
	private:
		bool inRange(uint8_t h, uint8_t s, uint8_t v);
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t HUE_MAX;
		uint8_t HUE_MIN;
		uint8_t SAT_MAX;
		uint8_t SAT_MIN;
		uint8_t VAL_MAX;
		uint8_t VAL_MIN;
};