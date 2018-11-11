/**
  * @brief
  */
#include <stdint.h>
class Navigator
{
	public:
		Navigator();
		//uint16_t getDirection(uint8_t (*src)[VPE_OUTPUT_W*3]);
        uint8_t isTrafficLightsGreen(uint8_t (green)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t (yellow)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t (red)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool waitGreenLights(uint8_t (green)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);

        /* for drawPath */
        void drawPath(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
	private:
        /*
        uint8_t detected_flag;
        uint8_t threshold;
        uint16_t direction;
        int32_t temp,i,j,k;
        uint16_t right_up, right_low, left_up, left_low;

        void calculateDirection();

        void getUpperRightPosition(uint8_t (*src)[VPE_OUTPUT_W*3]);
        void getLowerRightPosition(uint8_t (*src)[VPE_OUTPUT_W*3]);
        void getUpperLeftPosition (uint8_t (*src)[VPE_OUTPUT_W*3]);
        void getLowerLeftPosition (uint8_t (*src)[VPE_OUTPUT_W*3]);

        void UpperRightDetected();
        void LowerRightDetected();
        void UpperLeftDetected();
        void LowerLeftDetected();

        bool isRightDetected();
        bool isLeftDetected();
        */
        /* for drawPath */
        uint8_t threshold, detected_flag;

        struct Point {
            uint16_t x;
            uint16_t y;
        };
        Point lastRoadCenter;
        Point startingPoint;
        Point lastDirection;
        uint16_t shiftAmount;

        double roadSlope;
        double getRoadSlope(Point currentRoadCenter, Point lastRoadCenter);

        Point getRoadCenter(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y);
        Point getRightPosition(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y);
        Point getLeftPosition(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y);

        void RightDetected();
        void LeftDetected();
        bool isRightDetected();
        bool isLeftDetected();

        void drawDot(uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], Point point);
        void drawBigdot(uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], Point point);
};
