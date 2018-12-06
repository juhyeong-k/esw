/**
  * @brief
  */
#include <stdint.h>
#include <stdio.h>
#include "system_management.h"
#include "cv.h"
Navigator::Navigator()
{
    departedFlag = {0,};
    lastPoint.x = VPE_OUTPUT_W/2;
    lastPoint.y = VPE_OUTPUT_H;
    startingPoint = {(VPE_OUTPUT_W/2), VPE_OUTPUT_H, 0,};
}
CVinfo Navigator::getInfo(uint8_t display_buf[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3],
uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t red[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t white[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    startingPoint = getStartingPoint(yellow);
    CVinfo cvInfo = {1500, 0,};
    /**
     *  Normal Drive
     */
    cvInfo.direction = getDirection(yellow);
    if(cvInfo.direction == MAX_DIRECTION)         cvInfo.isLeftTurnDetected = true;
    else if(cvInfo.direction == MIN_DIRECTION)   cvInfo.isRightTurnDetected = true;

    cvInfo.isLeftDetected = isLeftDetected(yellow);
    cvInfo.isRightDetected = isRightDetected(yellow);
    if( !cvInfo.isLeftDetected && !cvInfo.isRightDetected)
        cvInfo.direction = getDirection(white);

    cvInfo.isDepartedLeft = isDepartedLeft(yellow);
    cvInfo.isDepartedRight = isDepartedRight(yellow);

    cvInfo.isLeftReinstation = isLeftReinstation(yellow);
    cvInfo.isRightReinstation = isRightReinstation(yellow);
    cvInfo.isRoadClose = isRoadClose(yellow, ISROADCLOSE_DISTANCE);
    cvInfo.isPathStraight = isPathStraight(yellow);
    cvInfo.isForwadPathExist = isForwadPathExist(yellow);
    cvInfo.isWhiteLineDetected_CV = isWhiteLineDetected_CV(yellow, white);
    /************************************************************************************/
    /**
     *  Emergency
     */
    cvInfo.isEmergency = isEmergency(red);
    /**
     *  Passing
     */
    cvInfo.isCarinFront_CV = isCarinFront_CV(yellow);//( isLeftDetected(yellow) | isRightDetected(yellow) );
    cvInfo.isSideRoadClose = isRoadClose(yellow, IS_SIDE_ROADCLOSE_DISTANCE);
    cvInfo.isSideRoadClose2 = isRoadClose(yellow, IS_SIDE_ROADCLOSE_DISTANCE2);
    cvInfo.exitDirection = getExitDirection(yellow);
    cvInfo.passNumber = getWidePass(yellow);
    /**
     *  Traffic Light
     */
    cvInfo.isTrafficLightsGreen = isTrafficLightsGreen(green);
    cvInfo.isTrafficLightsRed = isTrafficLightsRed(red);
    cvInfo.greenLightReply = greenLightReply(green);
    cvInfo.isGreenLightRoadClose = isRoadClose(yellow, IS_GREENLIGHT_ROADCLOSE_DISTANCE);
    /**
     *  Safe Zone
     */
    cvInfo.isUpperSafezoneDetected = isSafezoneDetected(yellow, white, SAFEZONE_UPLINE);
    cvInfo.isLowerSafezoneDetected = isSafezoneDetected(yellow, white, SAFEZONE_DOWNLINE);
    /**
     *  Roundabout
     */
    cvInfo.isWhiteRightDetected = isRightDetected(white);
    /**
     *  Tunnel
     */
    cvInfo.isTunnelDetected = isTunnelDetected(display_buf);
    /**
     *  DownHill
     */
    cvInfo.isDownHillDetected = isDownHillDetected(yellow);

    // Depart handling
    if(cvInfo.isDepartedLeft)            departedFlag.isDepartedLeft = true;
    else if(cvInfo.isDepartedRight)     departedFlag.isDepartedRight = true;

    if(cvInfo.isRightReinstation)       departedFlag.isDepartedLeft = false;
    else if(cvInfo.isLeftReinstation)   departedFlag.isDepartedRight = false;

    if(departedFlag.isDepartedLeft) {
        cvInfo.isDepartedLeft = true;
        startingPoint = {319, VPE_OUTPUT_H, 0,};
    }
    else if(departedFlag.isDepartedRight) {
        cvInfo.isDepartedRight = true;
        startingPoint = {0, VPE_OUTPUT_H, 0,};
    }

    // Print CV information
    /* Normal Drive */
    /*
    printf("\r\n--- Normal Drive\r\n");
    printf("\tdirection\t\t%d\r\n", cvInfo.direction);
    printf("\tLeftDetected\t\t%d\tRightDetected\t\t%d\r\n", cvInfo.isLeftDetected, cvInfo.isRightDetected);
    printf("\tLeftTurnDetected\t%d\tRightTurnDetected\t%d\r\n", cvInfo.isLeftTurnDetected, cvInfo.isRightTurnDetected);
    printf("\tisDepartedLeft\t\t%d\tisDepartedRight\t\t%d\r\n", cvInfo.isDepartedLeft, cvInfo.isDepartedRight);
    printf("\tisLeftReinstation\t%d\tisRightReinstation\t%d\r\n", cvInfo.isLeftReinstation, cvInfo.isRightReinstation);
    printf("\tisForwadPathExist\t%d", cvInfo.isForwadPathExist);
    printf("\tisPathStraight\t\t%d", cvInfo.isPathStraight);
    printf("\tRoadClose\t\t%d\r\n", cvInfo.isRoadClose);
    printf("\tisWhiteLineDetected_CV\t%d\r\n", cvInfo.isWhiteLineDetected_CV);
    */
    /* Emergency */
    /*
    printf("\r\n--- Emergency\r\n");
    printf("\tisEmergency\t\t%d\r\n", cvInfo.isEmergency);
    */
    /* Passing */
    /*
    printf("\r\n--- Passing\r\n");
    printf("\tisCarinFront_CV\t\t%d", cvInfo.isCarinFront_CV);
    printf("\tSideRoadClose\t\t%d\r\n", cvInfo.isSideRoadClose);*/
    /* Traffic Light */
    /*
    printf("\r\n--- Traffic Light\r\n");
    printf("\tisTrafficLightsGreen\t%d", cvInfo.isTrafficLightsGreen);
    printf("\tgreenLightReply\t\t%d", cvInfo.greenLightReply);
    printf("\tisGreenLightRoadClose\t%d\r\n", cvInfo.isGreenLightRoadClose);*/
    /* Safe Zone */
    /*
    printf("\r\n--- Safe Zone\r\n");
    printf("\tUpperSafezoneDetected\t%d", cvInfo.isUpperSafezoneDetected);
    printf("\tLowerSafezoneDetected\t%d\r\n", cvInfo.isLowerSafezoneDetected);
    /* Tunnel */
    /*
    printf("\r\n--- Tunnel\r\n");
    printf("\tTunnelDetected\t\t%d\r\n", cvInfo.isTunnelDetected);*/
    /* Down Hill */
    /*
    printf("\r\n--- Down Hill\r\n");
    printf("\tcvInfo.isDownHillDetected : %d\r\n", cvInfo.isDownHillDetected);
*/
    return cvInfo;
}
/**
 *  Passing
 */
bool Navigator::isCarinFront_CV(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint8_t y;
    uint8_t i = 0;
    uint8_t number = 0;
    Point lastRoadPoint = {0,};
    Point roadPoint = {0,};
    Point roadCenter = {0,};
    for(y=isCarinFront_CV_DOWN; y > isCarinFront_CV_UP; y--) {
        roadCenter = getRoadCenter(yellow, y);
        roadPoint = getRoadPoint(yellow, y);
        if(roadPoint.isLeftPoint | roadPoint.isCenterPoint) {
            i++;
            lastRoadPoint = roadPoint;
            lastPoint = roadCenter;
            break;
        }
    }
    for(y--; y > isCarinFront_CV_UP; y--) {
        roadCenter = getRoadCenter(yellow, y);
        roadPoint = getRoadPoint(yellow, y);
        if(roadPoint.isLeftPoint | roadPoint.isCenterPoint) {
            lastRoadPoint = roadPoint;
            lastPoint = roadCenter;
            i++;
            if(isRoadEndDetected(yellow, y)) break;
        }
    }
    lastPoint = startingPoint;
    printf("right : %d ",i);
    if(i < isCarinFront_CV_THRESHOLD) number++;

    //////////////////////////////////////////////////////////////////////

    i = 0;
    lastRoadPoint = {0,};
    roadPoint = {0,};
    roadCenter = {0,};
    for(y=isCarinFront_CV_DOWN; y > isCarinFront_CV_UP; y--) {
        roadCenter = getRoadCenter(yellow, y);
        roadPoint = getRoadPoint(yellow, y);
        if(roadPoint.isRightPoint | roadPoint.isCenterPoint) {
            i++;
            lastRoadPoint = roadPoint;
            lastPoint = roadCenter;
            break;
        }
    }
    for(y--; y > isCarinFront_CV_UP; y--) {
        roadCenter = getRoadCenter(yellow, y);
        roadPoint = getRoadPoint(yellow, y);
        if(roadPoint.isRightPoint | roadPoint.isCenterPoint) {
            lastRoadPoint = roadPoint;
            lastPoint = roadCenter;
            i++;
            if(isRoadEndDetected(yellow, y)) break;
        }
    }
    lastPoint = startingPoint;
    printf("left : %d\r\n",i);
    if(i < isCarinFront_CV_THRESHOLD) number++;
    if(number == 2) return true;
    else return false;
    /*
    uint16_t x,y;
    int temp = 0;
    for(x = 0; x < VPE_OUTPUT_W; x++) {
        for(y = 0; y < PASSING_WHITE_DETECT_HEIGHT; y++) {
            if(white[y][x][0]) temp++;
        }
    }
    //printf("\r\n\r\n white : %d\r\n", temp);
    if(temp > PASSING_WHITE_DETECT_THRESHOLD) return true;
    else return false;
    */
}
uint16_t Navigator::getExitDirection(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint16_t x,y,i;
    int temp = 0;
    int number = 0;
    int x_sum = 0;
    int min = FIND_EXIT_FRONT_UP;
    int exitDirection = 1500;
    for(x = 0; x < VPE_OUTPUT_W; x++) {
        for(y = FIND_EXIT_FRONT_DOWN; y > FIND_EXIT_FRONT_UP; y--) {
            if(yellow[y][x][0]) {
                temp = 0;
                for(i = 0; i < 3; i++) {
                    if(yellow[y-i][x][0]) temp++;
                }
                if(temp == 3) {
                    if(y < min) min = y;
                    break;
                }
            }
        }
        if( y < min + 2) {
            x_sum += x;
            number++;
        }
    }
    if(number == 0) return 1500;
    exitDirection = x_sum / number;
    exitDirection = 1500 + (160 - exitDirection)*3;
    return exitDirection;
}
uint16_t Navigator::getWidePass(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint16_t x,y;
    int i;
    int temp;
    int result = 0;
    for(x = 0; x < VPE_OUTPUT_W; x++) {
        for(y = 179; y > 0; y--) {
            if(yellow[y][x][0]) {
                temp = 0;
                for(i = 0; i < 3; i++) {
                    if(yellow[y-i][x][0]) temp++;
                }
                if(temp == 3) {
                    result++;
                    break;
                }
            }
        }
    }
    //printf("\r\nresult : %d\r\n", result);
    return result;
}
/**
 *  Emergency
 */
bool Navigator::isEmergency(uint8_t red[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint16_t x,y;
    int temp = 0;
    for(x = 0; x < VPE_OUTPUT_W; x++) {
        for(y = 0; y < VPE_OUTPUT_H; y++) {
            if( red[y][x][0] ) temp++;
        }
    }
    printf("temp : %d\r\n", temp);
    if(temp > EMERGENCY_THRESHOLD) return true;
    else return false;
}
/**
 *  Safe zone
 */
bool Navigator::isSafezoneDetected(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t white[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t line)
{
    int x,y,temp;
    bool yellowDetected = false;
    bool whiteDetected = true;
    uint8_t threshold = IS_SAFEZONE_CLOSE_THRESHOLD;
    temp = 0;

    for( y = line; y < 179; y++ ) {
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
            return true;
        }
        else temp = 0;
    }
    return false;
}
/**
 *  Draw functions 
 */
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
 *  Traffic Lights.
 */
bool Navigator::isTrafficLightsGreen(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint16_t i,j,temp;
    temp = 0;
    for(i=0; i<VPE_OUTPUT_H/2; i++) {
        for(j=0; j<VPE_OUTPUT_W; j++) {
            if( green[i][j][0] ) temp++;
        }
    }
    //printf("\r\n%d\r\n", temp);
    if(temp > GREEN_DETECT_PIXEL_THRESHOLD)    return true;
    else return false;
}
bool Navigator::isTrafficLightsRed(uint8_t red[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint16_t i,j,temp;
    temp = 0;
    for(i=0; i<VPE_OUTPUT_H/2; i++) {
        for(j=0; j<VPE_OUTPUT_W; j++) {
            if( red[i][j][0] ) temp++;
        }
    }
    //printf("\r\n%d\r\n", temp);
    if(temp > RED_DETECT_PIXEL_THRESHOLD)    return true;
    else return false;
}
uint8_t Navigator::greenLightReply(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint16_t y;
    uint16_t temp = 0;

    /* Green Light information */
    uint16_t greenHeight;
    uint16_t greenCenter;

    greenHeight = getGreenHeight(green);
    if(greenHeight) {

        Point leftPoint = getLeftGreenPoint(green, greenHeight);
        Point rightPoint = getRightGreenPoint(green, greenHeight);

        if(leftPoint.x & rightPoint.x) {
            greenCenter = (leftPoint.x + rightPoint.x)/2;
            uint16_t y_up = greenHeight;
            uint16_t y_down = greenHeight;

            y_up = getGreenUp(green, greenHeight, leftPoint, rightPoint);
            y_down = getGreenDown(green, greenHeight, leftPoint, rightPoint);
            if( isGreenLightReliable(y_down, y_up, greenHeight) ) {
                for(y = y_down; y > y_up; y--) {
                    if(green[y][greenCenter][0]) temp++;
                }
                //printf("\r\n\r\n%d  %d  %d\r\n\r\n", temp, y_down, y_up);
                if(temp < (y_down - y_up - 2) ) return 1;
                else return 2;
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
uint16_t Navigator::getGreenUp(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3], 
                                            uint16_t greenHeight, Point leftPoint, Point rightPoint) 
{
    uint16_t y,x,i,temp;
    uint16_t y_up = greenHeight;
    for(x = leftPoint.x; x < rightPoint.x; x++) {
        for(y = greenHeight; y > 5; y--) {
            temp = 0;
            if(green[y][x][0]) {
                for(i = 1; i < 6; i++) {
                    if(!green[y-i][x][0]) temp++;
                }
                if(temp == 5) {
                    if(y < y_up)  {
                        y_up = y;
                    }
                }
            }
        }
    }
    return y_up;
}
uint16_t Navigator::getGreenDown(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3], 
                                            uint16_t greenHeight, Point leftPoint, Point rightPoint)
{
    uint16_t y,x,i,temp;
    uint16_t y_down = greenHeight;
    for(x = leftPoint.x; x < rightPoint.x; x++) {
        for(y = greenHeight; y < VPE_OUTPUT_H-5; y++) {
            temp = 0;
            if(green[y][x][0]) {
                for(i = 1; i < 6; i++) {
                    if(!green[y+i][x][0]) temp++;
                }
                if(temp == 5) {
                    if(y > y_down) {
                        y_down = y;
                    }
                }
            }
        }
    }
    return y_down;
}
bool Navigator::isGreenLightReliable(uint16_t y_down, uint16_t y_up, uint16_t greenHeight)
{
    if( (y_down + y_up - 2 * greenHeight) < GREENLIGHT_DETECTED_THRESHOLD) return true;
    else return false;
}
bool Navigator::isTunnelDetected(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint16_t x,y;
    int temp = 0;
    for(x = 0; x < VPE_OUTPUT_W; x++) {
        for(y = 0; y < VPE_OUTPUT_H; y++) {
            if(src[y][x][2] < 50) temp++; 
        }
    }
    //printf("Value < 50 : %d\r\n", temp);
    if(temp > TUNNEL_DETECT_THRESHOLD) return true;
    else return false;
}
/**
 *  Normal Drive
 */
uint16_t Navigator::getDirection(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
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

    if(((float)i/j)*100 > threshold) slope = (totalRoadDiff / i)/SLOPE_DIVIDE_FACTOR;
    else slope = 0;
    if(slope == 0)              direction = 1500;
    else if(slope > 1.11)      direction = MAX_DIRECTION;    // Left
    else if (slope < -1.11)   direction = MIN_DIRECTION;     // Right
    /*
    else if(slope > 1.11)      direction = 2000;    // Left
    else if (slope < -1.11)   direction = 1000;     // Right
    */
    else                        direction = (uint16_t)(1500 + 450 * slope);
    return direction;
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
bool Navigator::isDownHillDetected(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    int y;
    int temp = 0;
    for(y = DOWNHILL_CHECK_DOWN; y > DOWNHILL_CHECK_UP; y--) {
        if(!isBothSideDetected(yellow, y)) break;
    }
    Point right_point = getRightPosition(yellow,y);
    Point left_point = getLeftPosition(yellow,y);

    if( (right_point.x - left_point.x) < 45 ) return true;
    else return false;
}
bool Navigator::isBothSideDetected(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y)
{
    Point right_point = getRightPosition(yellow,y);
    Point left_point = getLeftPosition(yellow,y);
    if(right_point.detected & left_point.detected) return true;
    else false;
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
            if(temp > LINE_DETECT_THRESHOLD) {
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
            if(temp > LINE_DETECT_THRESHOLD) {
                point = {i, y, true};
                return point;
            }
        }
    }
    return point;
}
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
float Navigator::getRoadDiff(Point current, Point last)
{
    int x_Variation, y_Variation;
    x_Variation = current.x - last.x;
    y_Variation = current.y - last.y;
    if( !(x_Variation && y_Variation) ) return 0;
    else return (float)x_Variation / y_Variation;
}
bool Navigator::isDepartedRight(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    Point point = getRoadPoint(yellow, 179);
    if(point.y) {
        if(point.x < 2) return true;
    }
    return false;
}
bool Navigator::isDepartedLeft(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    Point point = getRoadPoint(yellow, 179);
    if(point.y) {
        if(point.x > 317) return true;
    }
    return false;
}
bool Navigator::isLeftReinstation(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint8_t i;
    uint8_t temp = 0;
    Point point = getRoadPoint(yellow, 179);
    if(point.y) {
        for(i=0; i<5; i++) {
            if(!yellow[179][i][0]) temp++;
        }
        if(temp == 5) {
            if(point.x < REINSTATION_WIDTH) return true;
        }
    }
    return false;
}
bool Navigator::isRightReinstation(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint8_t i;
    uint8_t temp = 0;
    Point point = getRoadPoint(yellow, 179);
    if(point.y) {
        for(i=0; i<5; i++) {
            if(!yellow[179][319-i][0]) temp++;
        }
        if(temp == 5) {
            if(point.x > 319 - REINSTATION_WIDTH) return true;
        }
    }
    return false;
}
bool Navigator::isRoadClose(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t distance)
{
    uint8_t y,i,temp;
    temp = 0;
    for( y = 179; y > 0; y--) {
        if( src[y][159][0] ) {
            for(i=1; i<11; i++) {
                if( src[y-i][159][0] ) temp++;
            }
            if( temp >  ISROADCLOSE_THRESHOLD ) {
                if( y > (179 - distance) ) return true;
            }
        }
    }
    return false;
}
bool Navigator::isForwadPathExist(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint8_t y,i,temp;
    temp = 0;
    for( y = 179; y > 179 - FOWARD_PATH_EXIST_DISTANCE; y--) {
        if( yellow[y][159][0] ) {
            for(i=1; i<11; i++) {
                if( yellow[y-i][159][0] ) temp++;
            }
            if( temp >  FOWARD_PATH_EXIST_THRESHOLD )   return false;
        }
    }
    return true;
}
bool Navigator::isRightDetected(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint8_t y;
    uint8_t i = 0;
    Point lastRoadPoint = {0,};
    Point roadPoint = {0,};
    Point roadCenter = {0,};
    for(y=SIDE_DOWN; y > SIDE_UP; y--) {
        roadCenter = getRoadCenter(src, y);
        roadPoint = getRoadPoint(src, y);
        if(roadPoint.isRightPoint | roadPoint.isCenterPoint) {
            i++;
            lastRoadPoint = roadPoint;
            lastPoint = roadCenter;
            break;
        }
    }
    for(y--; y > SIDE_UP; y--) {
        roadCenter = getRoadCenter(src, y);
        roadPoint = getRoadPoint(src, y);
        if(roadPoint.isRightPoint | roadPoint.isCenterPoint) {
            lastRoadPoint = roadPoint;
            lastPoint = roadCenter;
            i++;
            if(isRoadEndDetected(src, y)) return true;
        }
    }
    lastPoint = startingPoint;

    if(i > SIDE_DIRECTION_THRESHOLD) return true;
    else return false;
}
bool Navigator::isLeftDetected(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint8_t y;
    uint8_t i = 0;
    Point lastRoadPoint = {0,};
    Point roadPoint = {0,};
    Point roadCenter = {0,};
    for(y=SIDE_DOWN; y > SIDE_UP; y--) {
        roadCenter = getRoadCenter(src, y);
        roadPoint = getRoadPoint(src, y);
        if(roadPoint.isLeftPoint | roadPoint.isCenterPoint) {
            i++;
            lastRoadPoint = roadPoint;
            lastPoint = roadCenter;
            break;
        }
    }
    for(y--; y > SIDE_UP; y--) {
        roadCenter = getRoadCenter(src, y);
        roadPoint = getRoadPoint(src, y);
        if(roadPoint.isLeftPoint | roadPoint.isCenterPoint) {
            lastRoadPoint = roadPoint;
            lastPoint = roadCenter;
            i++;
            if(isRoadEndDetected(src, y)) return true;
        }
    }
    lastPoint = startingPoint;

    if(i > SIDE_DIRECTION_THRESHOLD) return true;
    else return false;
}
bool Navigator::isPathStraight(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint8_t y;
    uint8_t i,j;
    i = j = 0;
    uint8_t threshold = ( (double)(FRONT_DOWN - FRONT_UP)*((double)GET_DIRECTION_THRESHOLD/100) );
    float totalRoadDiff = 0;
    float slope;
    Point lastRoadPoint = {0,};
    Point roadPoint = {0,};
    Point roadCenter = {0,};
    for(y=FRONT_DOWN; y > FRONT_UP; y--) {
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
    for(y--; y > FRONT_UP; y--) {
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
    else return false;

    if(abs(slope) < 0.3)  return true;
    else                   return false;
}
bool Navigator::isWhiteLineDetected_CV(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t white[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint16_t y;
    int temp = 0;
    Point roadCenter = {0,};
    for(y=VPE_OUTPUT_H-1; y > 0; y--) {
        roadCenter = getRoadCenter(yellow, y);
        if(roadCenter.detected) {
            if(white[roadCenter.y][roadCenter.x][0])
                temp++;
            lastPoint = roadCenter;
            if(isRoadEndDetected(yellow, y)) break;
        }
    }
    lastPoint = startingPoint;
    //printf("\r\n\r\n%d\r\n\r\n", temp);
    if(temp > WHITELINE_DETECT_THRESHOLD_CV) return true;
    else return false;
}
bool Navigator::isDifferentType(Point first, Point second)
{
    if(first.isCenterPoint & second.isCenterPoint) return false;
    else if(first.isRightPoint & second.isRightPoint) return false;
    else if(first.isLeftPoint & second.isLeftPoint) return false;
    else return true;
}


















/*
uint8_t Navigator::greenLightReply_2(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint16_t greenTop_x;

    // Green Light information
    uint16_t greenHeight;
    uint16_t greenCenter;

    greenHeight = getGreenHeight(green);
    if(greenHeight) {

        Point leftPoint = getLeftGreenPoint(green, greenHeight);
        Point rightPoint = getRightGreenPoint(green, greenHeight);

        if(leftPoint.x & rightPoint.x) {
            greenCenter = (leftPoint.x + rightPoint.x)/2;
            uint16_t y_up = greenHeight;
            uint16_t y_down = greenHeight;
            greenTop_x = getGreenTop_x(green, greenHeight, leftPoint, rightPoint);
            if(greenTop_x < (greenCenter - 1)) return 1;
            else return 2;
        }
    }
    return 0;
}
uint16_t Navigator::getGreenTop_x(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3], 
                                            uint16_t greenHeight, Point leftPoint, Point rightPoint) 
{
    uint16_t y,x,i,temp;
    uint16_t y_up = greenHeight;
    uint16_t GreenTop_x = 0;
    for(x = leftPoint.x; x < rightPoint.x; x++) {
        for(y = greenHeight; y > 5; y--) {
            temp = 0;
            if(green[y][x][0]) {
                for(i = 1; i < 4; i++) {
                    if(!green[y-i][x][0]) temp++;
                }
                if(temp == 3) {
                    if(y < y_up)  {
                        y_up = y;
                        GreenTop_x = x;
                    }
                }
            }
        }
    }
    return GreenTop_x;
}
int Navigator::greenLightReply(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3])
{
    uint16_t x,y;

    // Green Light information
    uint16_t greenHeight;
    uint16_t greenCenter;

    greenHeight = getGreenHeight(green);
    if(greenHeight) {

        Point leftPoint = getLeftGreenPoint(green, greenHeight);
        Point rightPoint = getRightGreenPoint(green, greenHeight);

        if(leftPoint.x & rightPoint.x) {
            greenCenter = (leftPoint.x + rightPoint.x)/2;
            uint16_t y_up = greenHeight;
            uint16_t y_down = greenHeight;

            y_up = getGreenUp(green, greenHeight, leftPoint, rightPoint);
            y_down = getGreenDown(green, greenHeight, leftPoint, rightPoint);
            if( isGreenLightReliable(y_down, y_up, greenHeight) ) {
                int leftNumber = 0;
                int rightNumber = 0;
                for(x = leftPoint.x; x <= greenCenter; x++) {
                    for(y = y_down; y >= y_up; y--) {
                        if( green[y][x][0] ) leftNumber++;
                    }
                }
                for(x = greenCenter; x <= rightPoint.x; x++) {
                    for(y = y_down; y >= y_up; y--) {
                        if( green[y][x][0] ) rightNumber++;
                    }
                }
                printf("\r\n\r\n%d\r\n\r\n", (int)( ((float)leftNumber/rightNumber - (int)(float)leftNumber/rightNumber) * 100));
                if(leftNumber < rightNumber + 5) {
                    return 2;
                }
                else if( (int)( ((float)leftNumber/rightNumber - (int)(float)leftNumber/rightNumber) * 100) > 12 ) {
                    return 1;
                }
                else {
                    return 2;
                }
            }
        }
    }
    return 0;
}
*/