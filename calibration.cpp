#include <string.h>
#include <assert.h>
#include "system_management.h"
#include "calibration.h"
#include "image_processing.h"
#include <iostream>
std::ofstream Calibration_result("calibration_result");
bool Calibrator::isReady(const char* str, uint8_t *mono_src)
{
	if (strcmp (str, "vertical") == 0)
		return isVertical_Ready(mono_src);
	if (strcmp (str, "horizontal") == 0)
		return isHorizontal_Ready(mono_src);
}
bool Calibrator::isVertical_Ready(uint8_t *mono_src)
{
	#ifdef bgr24
    	int x,i,j,k;
    	uint8_t stop_flag = 0;
    	uint16_t height[8] = {0,};

		for( i = 1; i < 8; i++) {
			x = 40 * i;
	        for(j = VPE_OUTPUT_H - 1; j > 0; j--) {
	            k = x + 3 * VPE_OUTPUT_W * j;
	            if(mono_src[k]) {
	            	height[i] = j;
	            	stop_flag++;
	            	break;
	            }
	        }
		}
		if( stop_flag == 7) {
			std::cout << height[1] << "\t" << height[2] << "\t" << height[3] << "\t" << height[4] 
			<< "\t" << height[5] << "\t" << height[6] << "\t" << height[7] << "\n";
			if ( height[1] == height[7] )
				if(height[2] == height[6])
					if(height[3] == height[5])
						if(height[4] == 179);
			return true;
		}
		else
			return false;
	#endif
}
bool Calibrator::isHorizontal_Ready(uint8_t *mono_src)
{
	while(1)
	{

	}
}
void Calibrator::vertical(uint8_t *mono_src)
{
	int i,j;
	if( !(VPE_OUTPUT_W == 320 & VPE_OUTPUT_H == 180) ) return;

	for(i = 0; i < 32; i++)
	{

	}
}
void Calibrator::draw_dot(uint8_t *des, uint16_t x, uint16_t y)
{
    #ifdef bgr24
        uint32_t point = y * VPE_OUTPUT_W * 3 + x * 3 + 1;
        des[point] = 255;
        des[point+1] = des[point+2] = 0;
    #endif
}