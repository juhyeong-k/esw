/**
  * @brief
  */
#include <stdint.h>
#include <stdio.h>
#include "system_management.h"
#include "cv.h"
Navigator::Navigator()
{
    lastPoint.x = VPE_OUTPUT_W/2;
    lastPoint.y = VPE_OUTPUT_H;
    startingPoint = {(VPE_OUTPUT_W/2), VPE_OUTPUT_H, 0,};
}
void Navigator::cvTest(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    startingPoint = getStartingPoint(src);
    drawPath(src, des);
    printf("direction %d\n\r", getDirection(src));
}
CVinfo Navigator::getInfo(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    CVinfo cvInfo = {1500, 0,};
    cvInfo.direction = getDirection(src);
    if(cvInfo.direction == 1000) cvInfo.isRightTurnDetected = true;
    else if(cvInfo.direction == 2000) cvInfo.isLeftTurnDetected = true;
    cvInfo.isRightDetected = isRightDetected(src);
    cvInfo.isLeftDetected = isLeftDetected(src);
    cvInfo.isRoadClose = isRoadClose(src);
    printf("* CV\r\n");
    printf("direction : %d\r\n", cvInfo.direction);
    printf("isRightTurnDetected : %d\r\n", cvInfo.isRightTurnDetected);
    printf("isLeftTurnDetected : %d\r\n", cvInfo.isLeftTurnDetected);
    printf("isRightDetected : %d\r\n", cvInfo.isRightDetected);
    printf("isLeftDetected : %d\r\n", cvInfo.isLeftDetected);
    printf("isRoadClose : %d\r\n", cvInfo.isRoadClose);
    return cvInfo;
}
bool Navigator::isSafezoneDetected(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t white[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    int x,y,temp;
    bool yellowDetected = false;
    bool whiteDetected = true;
    uint8_t threshold = IS_SAFEZONE_CLOSE_THRESHOLD;
    temp = 0;

    for( y = SIDE_DIRECTION_SIDE_UP; y < 179; y++ ) {
        for( x = 0; x < 319; x++)
        {
            if( whiteDetected ) {
                if( yellow[y][x][0] ) {
                    temp++;
                    yellowDetected = true;
                    whiteDetected = false;
                }
            }
            if( yellowDetected ) {
                if( white[y][x][0] ) {
                    temp++;
                    yellowDetected = false;
                    whiteDetected = true;
                }
            }
        }
        if(temp > threshold) {
            printf("IS_SAFEZONE_DETECTED\r\n");
            return true;
        }
        else temp = 0;
    }
    return false;
}
/**
 *  Get informations
 */
Navigator::Point Navigator::getStartingPoint(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint8_t y;
    Point roadCenter;
    for(y=VPE_OUTPUT_H-1; y > 0; y--) {
        roadCenter = getRoadCenter(src, y);
        if(roadCenter.detected) {
            return roadCenter;
        }
    }
    return {(VPE_OUTPUT_W/2), VPE_OUTPUT_H, 0,};
}
Navigator::Point Navigator::getRoadCenter(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y)
{
    Point roadPoint = {0,};
    Point right_point = getRightPosition(src,y);
    Point left_point = getLeftPosition(src,y);
    if(right_point.detected & left_point.detected) {
        roadPoint.x = (right_point.x + left_point.x)/2;
        roadPoint.y = (right_point.y + left_point.y)/2;
        roadPoint.detected = true;
    }
    else if( right_point.detected ) {
        roadPoint.x = right_point.x / 2;
        roadPoint.y = right_point.y;
        roadPoint.detected = true;
    }
    else if( left_point.detected ) {
        roadPoint.x = left_point.x + (VPE_OUTPUT_W - left_point.x)/ 2;
        roadPoint.y = left_point.y;
        roadPoint.detected = true;
    }
    else roadPoint.detected = false;
    return roadPoint;
}
Navigator::Point Navigator::getRoadPoint(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y)
{
    Point roadPoint = {0,};
    Point right_point = getRightPosition(src,y);
    Point left_point = getLeftPosition(src,y);
    
    if(right_point.detected & left_point.detected) {
        roadPoint.x = (right_point.x + left_point.x)/2;
        roadPoint.y = (right_point.y + left_point.y)/2;
        roadPoint.detected = true;
        roadPoint.isCenterPoint = true;
    }
    else if( right_point.detected ) {
        roadPoint.x = right_point.x;
        roadPoint.y = right_point.y;
        roadPoint.detected = true;
        roadPoint.isRightPoint = true;
    }
    else if( left_point.detected ) {
        roadPoint.x = left_point.x;
        roadPoint.y = left_point.y;
        roadPoint.detected = true;
        roadPoint.isLeftPoint = true;
    }
    else roadPoint.detected = false;
    return roadPoint;
}
Navigator::Point Navigator::getRightPosition(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y)
{
    uint16_t temp,i,j;
    // detect direction from Right
    temp = 0;
    Point point = {0,};
    for(i = lastPoint.x; i < VPE_OUTPUT_W; i++) {
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
    for(i = lastPoint.x; i > 0; i--) {
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
uint16_t Navigator::getDirection(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint8_t y;
    uint8_t i,j;
    i = j = 0;
    uint8_t threshold = ( (double)(SIDE_DOWN - SIDE_UP)*((double)GET_DIRECTION_THRESHOLD/100) );
    float totalRoadDiff = 0;
    uint16_t direction = 1500;
    float slope;
    Point lastRoadPoint = {0,};
    Point roadPoint = {0,};
    Point roadCenter = {0,};
    for(y=SIDE_DOWN; y > SIDE_UP; y--) {
        roadCenter = getRoadCenter(src, y);
        roadPoint = getRoadPoint(src, y);
        if(roadPoint.detected) {
            i++;
            lastRoadPoint = roadPoint;
            lastPoint = roadCenter;
            break;
        }
        j++;
    }
    for(y--; y > SIDE_UP; y--) {
        roadCenter = getRoadCenter(src, y);
        roadPoint = getRoadPoint(src, y);
        if(roadPoint.detected) {
            if( !isDifferentType(roadPoint, lastRoadPoint) ) {
                totalRoadDiff += getRoadDiff(roadPoint, lastRoadPoint);
                lastRoadPoint = roadPoint;
                lastPoint = roadCenter;
            }
            i++;
            if(isRoadEndDetected(src, y)) break;
        }
        j++;
    }
    lastPoint = startingPoint;

    if(((float)i/j)*100 > threshold) slope = (totalRoadDiff / i)/2;
    else slope = 0;
    if(slope == 0)              direction = 1500;
    else if(slope > 1.11)      direction = 2000;    // Left
    else if (slope < -1.11)   direction = 1000;     // Right
    else                        direction = (uint16_t)(1500 + 450 * slope);
    return direction;
}
float Navigator::getRoadDiff(Point current, Point last)
{
    int x_Variation, y_Variation;
    x_Variation = current.x - last.x;
    y_Variation = current.y - last.y;
    if( !(x_Variation && y_Variation) ) return 0;
    else return (float)x_Variation / y_Variation;
}
/**
 *  Check state
 */
bool Navigator::isRoadClose(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint8_t y,i,temp;
    temp = 0;
    for( y = 179; y > 100; y--) {
        if( src[y][159][0] ) {
            for(i=1; i<11; i++) {
                if( src[y-i][159][0] ) temp++;
            }
            if( temp >  ISROADCLOSE_THRESHOLD ) {
                if( y > (179 - ISROADCLOSE_DISTANCE) ) return true;
            }
        }
    }
    return false;
}
bool Navigator::isRightDetected(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint8_t y;
    uint8_t i = 0;
    uint8_t threshold = SIDE_DIRECTION_THRESHOLD;
    Point lastRoadPoint = {0,};
    Point roadPoint = {0,};
    Point roadCenter = {0,};
    for(y=SIDE_DIRECTION_SIDE_DOWN; y > SIDE_DIRECTION_SIDE_UP; y--) {
        roadCenter = getRoadCenter(src, y);
        roadPoint = getRoadPoint(src, y);
        if(roadPoint.isRightPoint | roadPoint.isCenterPoint) {
            i++;
            lastRoadPoint = roadPoint;
            lastPoint = roadCenter;
            break;
        }
    }
    for(y--; y > SIDE_DIRECTION_SIDE_UP; y--) {
        roadCenter = getRoadCenter(src, y);
        roadPoint = getRoadPoint(src, y);
        if(roadPoint.isRightPoint | roadPoint.isCenterPoint) {
            lastRoadPoint = roadPoint;
            lastPoint = roadCenter;
            i++;
            if(isRoadEndDetected(src, y)) break;
        }
    }
    lastPoint = startingPoint;

    if(i > threshold) return true;
    else return false;
}
bool Navigator::isLeftDetected(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint8_t y;
    uint8_t i = 0;
    uint8_t threshold = SIDE_DIRECTION_THRESHOLD;
    Point lastRoadPoint = {0,};
    Point roadPoint = {0,};
    Point roadCenter = {0,};
    for(y=SIDE_DIRECTION_SIDE_DOWN; y > SIDE_DIRECTION_SIDE_UP; y--) {
        roadCenter = getRoadCenter(src, y);
        roadPoint = getRoadPoint(src, y);
        if(roadPoint.isLeftPoint | roadPoint.isCenterPoint) {
            i++;
            lastRoadPoint = roadPoint;
            lastPoint = roadCenter;
            break;
        }
    }
    for(y--; y > SIDE_DIRECTION_SIDE_UP; y--) {
        roadCenter = getRoadCenter(src, y);
        roadPoint = getRoadPoint(src, y);
        if(roadPoint.isLeftPoint | roadPoint.isCenterPoint) {
            lastRoadPoint = roadPoint;
            lastPoint = roadCenter;
            i++;
            if(isRoadEndDetected(src, y)) break;
        }
    }
    lastPoint = startingPoint;

    if(i > threshold) return true;
    else return false;
}
bool Navigator::isPathStraight(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint8_t y;
    int x_Variation = 0;
    int y_Variation = 0;
    Point last;
    Point current;
    for(y=FRONT_DOWN; y > FRONT_UP; y--) {
        getRoadPoint(src,y);
    }
}
bool Navigator::isDifferentType(Point first, Point second)
{
    if(first.isCenterPoint & second.isCenterPoint) return false;
    else if(first.isRightPoint & second.isRightPoint) return false;
    else if(first.isLeftPoint & second.isLeftPoint) return false;
    else return true;
}
/**
 *  Draw functions 
 */
void Navigator::drawPath(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint16_t y;
    Point roadCenter = {0,};
    for(y=VPE_OUTPUT_H-1; y > 0; y--) {
        roadCenter = getRoadCenter(src, y);\
        if(roadCenter.detected) {
            drawDot(des, roadCenter);
            drawDot(des, getRightPosition(src, y));
            drawDot(des, getLeftPosition(src, y));
            lastPoint = roadCenter;
            if(isRoadEndDetected(src, y)) break;
        }
    }
    lastPoint = startingPoint;
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
        if( (right_point.x - left_point.x) < 6 ) return true;
    }
    if(right_point.detected) {
        if( right_point.x < 3 ) return true;
    }
    else if(left_point.detected) {
        if( left_point.x > 315 ) return true;
    }
    return false;
}
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
int Navigator::greenLightReply(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint16_t x,y,temp,i;
    int y_sum = 0;
    uint8_t checkNumber = 0;

    /* Green Light information */
    uint16_t greenHeight;
    uint16_t greenCenter;
    Point leftPoint, rightPoint;
    leftPoint = {0,};
    rightPoint = {0,};

    for(y = 0; y < VPE_OUTPUT_H; y++) {
        for(x = 0; x < VPE_OUTPUT_W; x++) {
            if( green[y][x][0] ) {
                temp = 0;
                for(i=0; i < VPE_OUTPUT_W-x; i++) {
                    if(green[y][x+i][0]) temp++;
                }
                if(temp > GREENLIGHT_WIDTH_THRESHOLD) {
                    y_sum += y;
                    checkNumber++;
                    break;
                }
            }
        }
    }
    greenHeight = getGreenHeight(green);
    if(greenHeight) {
        /* greenHeight, leftPoint, rightPoint */
        for(x = 0; x < VPE_OUTPUT_W; x++) {
            if(green[greenHeight][x][0]) {
                temp = 0;
                for(i=0; i < VPE_OUTPUT_W-x; i++) {
                    if(green[greenHeight][x+i][0]) temp++;
                }
                if(temp > GREENLIGHT_WIDTH_THRESHOLD) {
                    leftPoint = {x, greenHeight, 0,};
                    break;
                }
            }
        }
        for(x = 0; x < VPE_OUTPUT_W; x++) {
            if(!green[greenHeight][x][0]) {
                temp = 0;
                for(i = x; i > 0; i--) {
                    if(green[greenHeight][i][0]) temp++;
                }
                if(temp > GREENLIGHT_WIDTH_THRESHOLD) {
                    rightPoint = {x, greenHeight};
                    break;
                }
            }
        }
        if(leftPoint.x & rightPoint.x) {
            /* Get greenCenter, y_high, y_low */
            greenCenter = (leftPoint.x + rightPoint.x)/2;
            int y_high = greenHeight;
            int y_low = greenHeight;
            for(x = leftPoint.x; x < rightPoint.x; x++) {
                for(y = greenHeight; y > 5; y--) {
                    temp = 0;
                    if(!green[y][x][0]) {
                        for(i = 1; i < 6; i++) {
                            if(!green[y-i][x][0]) temp++;
                        }
                        if(temp == 5) {
                            if(y < y_high)  y_high = y;
                            break;
                        }
                    }
                }
                for(y = greenHeight; y < VPE_OUTPUT_H-5; y++) {
                    temp = 0;
                    if(!green[y][x][0]) {
                        for(i = 1; i < 6; i++) {
                            if(!green[y+i][x][0]) temp++;
                        }
                        if(temp == 5) {
                            if(y > y_low) y_low = y;
                            break;
                        }
                    }
                }
            }
            if( (y_low + y_high - 2 * greenHeight) < GREENLIGHT_DETECTED_THRESHOLD ) {
                int leftNumber = 0;
                int rightNumber = 0;
                for(x = leftPoint.x; x <= greenCenter; x++) {
                    for(y = y_low; y >= y_high; y--) {
                        if( green[y][x][0] ) leftNumber++;
                    }
                }
                for(x = greenCenter; x <= rightPoint.x; x++) {
                    for(y = y_low; y >= y_high; y--) {
                        if( green[y][x][0] ) rightNumber++;
                    }
                }
                printf("%f\r\n", (float)leftNumber/rightNumber);
                if(leftNumber < rightNumber)
                    printf("Right!!\r\n");
                else if( (int)(((float)leftNumber/rightNumber - (int)(float)leftNumber/rightNumber)*100) > 6 )
                    printf("Left!!\r\n");
                else 
                    printf("Right!!\r\n");
                return 0;
            }
        }
    }
    return 0;
}
int Navigator::getGreenHeight(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint16_t y,x,i,temp;
    uint16_t y_sum = 0;
    uint16_t checkNumber = 0;
    for(y = 0; y < VPE_OUTPUT_H; y++) {
        for(x = 0; x < VPE_OUTPUT_W; x++) {
            if( green[y][x][0] ) {
                temp = 0;
                for(i=0; i < VPE_OUTPUT_W-x; i++) {
                    if(green[y][x+i][0]) temp++;
                }
                if(temp > GREENLIGHT_WIDTH_THRESHOLD) {
                    y_sum += y;
                    checkNumber++;
                    break;
                }
            }
        }
    }
    if(checkNumber) {
        y = (int)((float)y_sum/checkNumber);
        return y;
    }
    else return 0;
}
Navigator::Point Navigator::getLeftGreenPoint(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t greenHeight) {
    uint16_t x,i,temp;
    Point leftPoint = {0,};
    for(x = 0; x < VPE_OUTPUT_W; x++) {
        if(green[greenHeight][x][0]) {
            temp = 0;
            for(i=0; i < VPE_OUTPUT_W-x; i++) {
                if(green[greenHeight][x+i][0]) temp++;
            }
            if(temp > GREENLIGHT_WIDTH_THRESHOLD) {
                leftPoint = {x, greenHeight, 0,};
                return leftPoint;
            }
        }
    }
    return leftPoint;
}
Navigator::Point Navigator::getRightGreenPoint(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t greenHeight) {
    uint16_t x,i,temp;
    Point rightPoint = {0,};
    for(x = 0; x < VPE_OUTPUT_W; x++) {
        if(!green[greenHeight][x][0]) {
            temp = 0;
            for(i = x; i > 0; i--) {
                if(green[greenHeight][i][0]) temp++;
            }
            if(temp > GREENLIGHT_WIDTH_THRESHOLD) {
                rightPoint = {x, greenHeight};
                return rightPoint;
            }
        }
    }
    return rightPoint;
}