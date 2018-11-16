/**
  * @brief
  */
#include <stdint.h>
#include <stdio.h>
#include "system_management.h"
#include "cv.h"
Navigator::Navigator()
{
    /* for drawPath */
    last.roadCenter.x = VPE_OUTPUT_W/2;
    last.roadCenter.y = VPE_OUTPUT_H;
    roadSlope = 180;
    startingPoint = {(VPE_OUTPUT_W/2), VPE_OUTPUT_H};
}
/* for drawPath */
void Navigator::drawPath(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint16_t y;
    Point roadCenter = {0,};
    for(y=VPE_OUTPUT_H-1; y > 0; y--) {
        roadCenter = getRoadCenter(src, y);
        if(roadCenter.detected) {
            drawDot(des, roadCenter);
            drawDot(des, getRightPosition(src, y));
            drawDot(des, getLeftPosition(src, y));
        }
        if(roadCenter.y && roadCenter.x) {
            startingPoint = roadCenter;
            break;
        }
    }
    for(y; y > 0; y--) {
        roadCenter = getRoadCenter(src, y);
        if(roadCenter.detected) {
            drawDot(des, roadCenter);
            drawDot(des, getRightPosition(src, y));
            drawDot(des, getLeftPosition(src, y));
        }
        if(isRoadEndDetected(src, y)) break;
    }
    last.roadCenter = startingPoint;
}
Navigator::Point Navigator::getRoadCenter(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y)
{
    Point roadCenter = {0,};
    Point right_point = getRightPosition(src,y);
    Point left_point = getLeftPosition(src,y);
    
    if(right_point.detected & left_point.detected) {
        roadCenter.x = (right_point.x + left_point.x)/2;
        roadCenter.y = (right_point.y + left_point.y)/2;
        roadCenter.detected = true;
        roadSlope = getRoadSlope(roadCenter, last.roadCenter);
        last.roadCenter = roadCenter;
    }
    else if( right_point.detected ) {
        roadCenter.x = right_point.x / 2;
        roadCenter.y = right_point.y;
        roadCenter.detected = true;
        roadSlope = getRoadSlope(roadCenter, last.roadCenter);
        last.roadCenter = roadCenter;
    }
    else if( left_point.detected ) {
        roadCenter.x = left_point.x + (VPE_OUTPUT_W - left_point.x)/ 2;
        roadCenter.y = left_point.y;
        roadCenter.detected = true;
        roadSlope = getRoadSlope(roadCenter, last.roadCenter);
        last.roadCenter = roadCenter;
    }
    else roadCenter.detected = false;
    return roadCenter;
}
Navigator::Point Navigator::getRightPosition(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y)
{
    uint16_t temp,i,j;
    // detect direction from Right
    temp = 0;
    Point point = {0,};
    for(i = last.roadCenter.x; i < VPE_OUTPUT_W; i++) {
        if( src[y][i][0] )
        {
            for(j=1; j<11; j++) {
                if( src[y][i+j][0] )    temp++;
            }
            if(temp > lineDectectTHRESHOLD) {
                point = {i, y, true};
                return point;
            }
        }
    }
    return point;
}
Navigator::Point Navigator::getLeftPosition(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y)
{
    uint16_t temp,i,j;
    // detect direction from Left
    temp = 0;
    Point point = {0,};
    for(i = last.roadCenter.x; i > 0; i--) {
        if( src[y][i][0] )
        {
            for(j=1; j<11; j++) {
                if( src[y][i-j][0] )    temp++;
            }
            if(temp > lineDectectTHRESHOLD) {
                point = {i, y, true};
                return point;
            }
        }
    }
    return point;
}
double Navigator::getRoadSlope(Point currentRoadCenter, Point lastRoadCenter)
{
    int x_Variation, y_Variation;
    x_Variation = current.roadCenter.x - last.roadCenter.x;
    y_Variation = current.roadCenter.y - last.roadCenter.y;
    if( !(x_Variation || y_Variation) ) return roadSlope;
    else if( x_Variation == 0 ) return 180;
    else if( y_Variation == 0 ) return 0;
    else return (double)y_Variation / x_Variation;
}
void Navigator::drawDot(uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], Point point)
{
    uint16_t x,y;
    x = point.x;
    y = point.y;
    #ifdef bgr24
        des[y][x][2] = 255;
        des[y][x][0] = des[y][x][1] = 0;
    #endif
}
void Navigator::drawBigdot(uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], Point point)
{
    int32_t i,j;
    uint16_t x,y;
    x = point.x;
    y = point.y;
    #ifdef bgr24
        for(i=-1; i<2; i++) {
            for(j=-1; j<2; j++) {
                des[y+j][x+i][1] = 255;
                des[y+j][x+i][0] = des[y+j][x+i][2] = 0;
            }
        }
    #endif
}
bool Navigator::isRoadEndDetected(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y)
{
    Point right_point = getRightPosition(src,y);
    Point left_point = getLeftPosition(src,y);
    if(right_point.detected & left_point.detected) {
        if(right_point.x == left_point.x+4) {
            return true;
        }
    }
    return false;
}
/*
uint16_t Navigator::getDirection(uint8_t (*src)[VPE_OUTPUT_W*3])
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
void Navigator::getUpperRightPosition(uint8_t (*src)[VPE_OUTPUT_W*3])
{
	// detect direction from Right UPPER_LINE
    temp = 0;
    for(i = VPE_OUTPUT_W / 2; i < VPE_OUTPUT_W; i++)
    {
        j = 3*i;
        if( src[UPPER_LINE][j] )
        {
            for(k=1; k<11; k++) {
                if( src[UPPER_LINE][j+3*k] )    temp++;
            }
            if(temp > threshold) {
                right_up = i;
                drawBigdot(src, i, UPPER_LINE);
                UpperRightDetected();
                break;
            }
        }
    }
}
void Navigator::getLowerRightPosition(uint8_t (*src)[VPE_OUTPUT_W*3])
{
	// detect direction from Right LOWER_LINE
    temp = 0;
    for(i = VPE_OUTPUT_W / 2; i < VPE_OUTPUT_W; i++)
    {
        j = 3*i;
        if( src[LOWER_LINE][j] )
        {
            for(k=1; k<11; k++) {
                if( src[LOWER_LINE][j+3*k] )    temp++;
            }
            if(temp > threshold) {
                right_low = i;
                drawBigdot(src, i, LOWER_LINE);
                LowerRightDetected();
                break;
            }
        }
    }
}
void Navigator::getUpperLeftPosition(uint8_t (*src)[VPE_OUTPUT_W*3])
{
	// detect direction from Left UPPER_LINE
    temp = 0;
    for(i = VPE_OUTPUT_W / 2; i > 0; i--)
    {
        j = 3*i;
        if( src[UPPER_LINE][j] )
        {
            for(k=1; k<11; k++) {
                if( src[UPPER_LINE][j-3*k] )    temp++;
            }
            if(temp > threshold) {
                left_up = i;
                drawBigdot(src, i, UPPER_LINE);
                UpperLeftDetected();
                break;
            }
        }
    }
}
void Navigator::getLowerLeftPosition(uint8_t (*src)[VPE_OUTPUT_W*3])
{
    temp = 0;
    for(i = VPE_OUTPUT_W / 2; i > 0; i--)
    {
        j = 3*i;
        if( src[LOWER_LINE][j] )
        {
            for(k=1; k<11; k++) {
                if( src[LOWER_LINE][j-3*k] )    temp++;
            }
            if(temp > threshold) {
                left_low = i;
                drawBigdot(src, i, LOWER_LINE);
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
    else if( isLeftDetected() )
        vector = (float)(left_up - left_low) / (LOWER_LINE - UPPER_LINE);
    else return;

    if(vector > 1.11)         direction = 1000;
    else if (vector < -1.11) direction = 2000;
    else                       direction = (uint16_t)(1500 - 450 * vector);
}
void Navigator::drawDot(uint8_t (*des)[VPE_OUTPUT_W*3], uint16_t x, uint16_t y)
{
    #ifdef bgr24
        des[y][3*x+1] = 255;
        des[y][3*x] = des[y][3*x+2] = 0;
    #endif
}
void Navigator::drawBigdot(uint8_t (*des)[VPE_OUTPUT_W*3], uint16_t x, uint16_t y)
{
    #ifdef bgr24
        for(i=-1; i<2; i++) {
            for(j=-1; j<2; j++) {
                des[y+j][3*(x+i)+1] = 255;
                des[y+j][3*(x+i)] = des[y+j][3*(x+i)+2] = 0;
            }
        }
    #endif
}
*/
/**
  * @ Traffic Lights
  */
uint8_t Navigator::isTrafficLightsGreen(uint8_t (green)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t (yellow)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t (red)[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint16_t i,j,temp;
    temp = 0;
    for(i=0; i<VPE_OUTPUT_H/2; i++) {
        for(j=0; j<VPE_OUTPUT_W; j++) {
            if( green[i][j][0] ) temp++;
        }
    }
    if(temp > colorDetectTHRESHOLD)    return 1;

    temp = 0;
    for(i=0; i<VPE_OUTPUT_H/2; i++) {
        for(j=0; j<VPE_OUTPUT_W; j++) {
            if( yellow[i][j][0] ) temp++;
        }
    }
    if(temp > colorDetectTHRESHOLD)    return 2;

    temp = 0;
    for(i=0; i<VPE_OUTPUT_H/2; i++) {
        for(j=0; j<VPE_OUTPUT_W; j++) {
            if( red[i][j][0] ) temp++;
        }
    }
    if(temp > colorDetectTHRESHOLD)    return 2;

    return 3;
}
