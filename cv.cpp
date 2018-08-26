/**
  * @brief
  */
#include <stdint.h>
#include <stdio.h>
#include "system_management.h"
#include "cv.h"
Navigator::Navigator(uint8_t THRESHOLD)
{
	threshold = THRESHOLD;
    direction = 1500;
	detected_flag = 0;
	UpperStartPosition = UPPER_LINE * VPE_OUTPUT_W * 3 + VPE_OUTPUT_W * 3 / 2;
	LowerStartPosition = LOWER_LINE * VPE_OUTPUT_W * 3 + VPE_OUTPUT_W * 3 / 2;
}
uint16_t Navigator::getDirection(uint8_t *src)
{
    getUpperRightPosition(src);
    getLowerRightPosition(src);
    getUpperLeftPosition(src);
    getLowerLeftPosition(src);

    calculateDirection();

	detected_flag = 0;
    printf("direction : %d\n", direction);
	return direction;
}
void Navigator::getUpperRightPosition(uint8_t *src)
{
	// detect direction from Right UPPER_LINE
	temp = 0;
    for(i = 0; i < VPE_OUTPUT_W / 2; i++)
    {
        j = 3 * i;
        if( src[UpperStartPosition + j] )
        {
            for(k = 1; k < 10; k++) {
                if( src[UpperStartPosition + j + 3*k] )    temp++;
            }
            if(temp > threshold) {
                right_up = (( UpperStartPosition + j ) % ( VPE_OUTPUT_W * 3 )) / 3;
                UpperRightDetected();
                break;
            }
        }
    }
}
void Navigator::getLowerRightPosition(uint8_t *src)
{
	// detect direction from Right LOWER_LINE
	temp = 0;
	for(i = 0; i < VPE_OUTPUT_W / 2; i++)
    {
        j = 3 * i;
        if( src[LowerStartPosition + j] )
        {
            for(k = 1; k < 10; k++) {
            	if(src[LowerStartPosition + j + 3*k])    temp++;
            }
            if(temp > threshold) {
                right_low = (( LowerStartPosition + j ) % ( VPE_OUTPUT_W * 3 )) / 3;
                LowerRightDetected();
                break;
            }
        }
    }
}
void Navigator::getUpperLeftPosition(uint8_t *src)
{
	// detect direction from Left UPPER_LINE
	temp = 0;
	for(i = 0; i < VPE_OUTPUT_W / 2; i++)
	{
	    j = 3 * i;
	    if( src[UpperStartPosition - j] )
	    {
	        for(k = 1; k < 10; k++) {
	        	if(src[UpperStartPosition - j - 3*k])    temp++;
	        }
	        if(temp > threshold)
	        {
	            left_up = (( UpperStartPosition - j ) % (VPE_OUTPUT_W * 3)) / 3;
	            UpperLeftDetected();
	            break;
	        }
	    }
	}
}
void Navigator::getLowerLeftPosition(uint8_t *src)
{
	temp = 0;
	for(i = 0; i < VPE_OUTPUT_W / 2; i++)
    {
        j = 3 * i;
        if( src[LowerStartPosition - j] )
        {
            for(k = 1; k < 10; k++) {
            	if(src[LowerStartPosition - j - 3*k])    temp++;
            }
            if(temp > threshold) 
            {
                left_low = (( LowerStartPosition - j ) % (VPE_OUTPUT_W * 3)) / 3;
                LowerLeftDetected();
                break;
            }
        }
    }
}
void Navigator::UpperRightDetected() { detected_flag += 1; }
void Navigator::LowerRightDetected() { detected_flag += 2; }
void Navigator::UpperLeftDetected()  { detected_flag += 4; }
void Navigator::LowerLeftDetected()  { detected_flag += 8; }
bool Navigator::isRightDetected() {
	if((detected_flag & 1) && (detected_flag & 2)) return true;
	else                                               return false;
}
bool Navigator::isLeftDetected() {
    if((detected_flag & 4) && (detected_flag & 8)) return true;
    else                                               return false;
}
void Navigator::calculateDirection()
{
    float vector;
    if( isRightDetected() & isLeftDetected() )
        vector = (float)(right_up - right_low) / (LOWER_LINE - UPPER_LINE) + (float)(left_up - left_low) / (LOWER_LINE - UPPER_LINE);
    else if( isRightDetected() )
        vector = (float)(right_up - right_low) / (LOWER_LINE - UPPER_LINE);
    else if( isLeftDetected() ) {
        vector = (float)(left_up - left_low) / (LOWER_LINE - UPPER_LINE);
    }
    else return;

    if(vector > 1.11)         direction = 1000;
    else if (vector < -1.11) direction = 2000;
    else                       direction = (uint16_t)(1500 - 450 * vector);
}
