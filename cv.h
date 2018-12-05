/**
  * @brief
  */
#include <stdint.h>

class Navigator
{
	public:
		Navigator();
        bool waitGreenLights(uint8_t (green)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);

        /* for drawPath */
        void drawPath(uint8_t (src)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        /* Result info */
        CVinfo getInfo(uint8_t display_buf[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3],
        uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t red[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t white[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);

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
        struct DepartedFlag {
            bool isDepartedLeft;
            bool isDepartedRight;
        };
        /* Valuables */
        Point lastPoint;
        Point startingPoint;
        DepartedFlag departedFlag;

        /* Emergency */
        bool isEmergency(uint8_t red[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        /* Passing */
        bool isCarinFront_CV(uint8_t white[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        uint16_t getExitDirection(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        uint16_t getWidePass(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        /*  */
        bool isDownHillDetected(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool isBothSideDetected(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y);
        /* Tunnel */
        bool isTunnelDetected(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        /* Safezone */
        bool isSafezoneDetected(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t white[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t line);
        /* GreenLight */
        bool isTrafficLightsGreen(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool isTrafficLightsRed(uint8_t Red[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        uint8_t greenLightReply(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        int getGreenHeight(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        uint16_t getGreenTop_x(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t greenHeight, Point leftPoint, Point rightPoint);
        uint16_t getGreenUp(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t greenHeight, Point leftPoint, Point rightPoint);
        uint16_t getGreenDown(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t greenHeight, Point leftPoint, Point rightPoint);
        Point getLeftGreenPoint(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t greenHeight);
        Point getRightGreenPoint(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t greenHeight);

        bool isGreenLightReliable(uint16_t y_down, uint16_t y_up, uint16_t greenHeight);

        /**
         * Normal Drive
         */
        Point getStartingPoint(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3]); // for loop control
        Point getRoadCenter(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y);
        Point getRoadPoint(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y);
        Point getRightPosition(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y);
        Point getLeftPosition(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y);
        float getRoadDiff(Point current, Point last);
        uint16_t getDirection(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        // before using isRoadEndDetected() function, confirm the last.roadCenter has been updated.
        bool isRoadEndDetected(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t y);
        bool isRightDetected(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool isLeftDetected(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool isPathRight(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool isPathLeft(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool isPathStraight(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool isRoadClose(uint8_t src[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint16_t distance);
        bool isDepartedRight(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool isDepartedLeft(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool isLeftReinstation(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool isRightReinstation(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool isForwadPathExist(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        bool isWhiteLineDetected_CV(uint8_t yellow[VPE_OUTPUT_H][VPE_OUTPUT_W][3], uint8_t white[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        /* Functions for CV class */
        bool isDifferentType(Point first, Point second);

        /* Draw functions */
        void drawDot(uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], Point point);
        void drawBigdot(uint8_t (des)[VPE_OUTPUT_H][VPE_OUTPUT_W][3], Point point);

        //uint8_t greenLightReply_2(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
        //uint8_t greenLightReply_3(uint8_t green[VPE_OUTPUT_H][VPE_OUTPUT_W][3]);
};
