/**
  * @brief
  */
#include <stdint.h>

class Navigator
{
	public:
		Navigator();        uint8_t isTrafficLightsGreen(uint8_t (green)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t (yellow)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t (red)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool waitGreenLights(uint8_t (green)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);

        /* for drawPath */
        void drawPath(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        /* Result info */
        CVinfo getInfo(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool isSafezoneDetected(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t white[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool isTunnelDetected(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        int greenLightReply(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);

        /**/
        int getGreenHeight(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        /**/
	private:
        /* Structures */
        struct Point {
            uint16_t x;
            uint16_t y;
            bool detected;
            bool isCenterPoint;
            bool isRightPoint;
            bool isLeftPoint;
        };
        /* Valuables */
        Point lastPoint;
        Point startingPoint;

        /* Get information */
        Point getStartingPoint(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]); // for loop control
        Point getRoadCenter(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y);
        Point getRoadPoint(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y);
        Point getRightPosition(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y);
        Point getLeftPosition(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y);
        float getRoadDiff(Point current, Point last);
        uint16_t getDirection(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);

        // Traffic lights
        Point getLeftGreenPoint(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t greenHeight);
        Point getRightGreenPoint(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t greenHeight);
        uint16_t getGreenUp(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t greenHeight, Point leftPoint, Point rightPoint);
        uint16_t getGreenDown(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t greenHeight, Point leftPoint, Point rightPoint);

        /* Status Check */
        // before using isRoadEndDetected() function, confirm the last.roadCenter has been updated.
        bool isRoadEndDetected(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y);

        bool isRightDetected(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool isLeftDetected(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);

        bool isPathRight(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool isPathLeft(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool isPathStraight(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);

        bool isRoadClose(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);

        bool isGreenLightReliable(uint16_t y_down, uint16_t y_up, uint16_t greenHeight);

        /* Functions for CV class */
        bool isDifferentType(Point first, Point second);

        /* Draw functions */
        void drawDot(uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], Point point);
        void drawBigdot(uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], Point point);
};
