/**
  * @brief
  */
#include <stdint.h>
class Navigator
{
	public:
		Navigator(uint8_t THRESHOLD);
		uint16_t getDirection(uint8_t (*src)[VPE_OUTPUT_W*3]);
        bool checkTrafficLights(uint8_t (*src)[VPE_OUTPUT_H][VPE_OUTPUT_W*3]);
	private:
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

        void drawDot(uint8_t (*des)[VPE_OUTPUT_W*3], uint16_t x, uint16_t y);
        void drawBigdot(uint8_t (*des)[VPE_OUTPUT_W*3], uint16_t x, uint16_t y);
};
