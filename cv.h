/**
  * @brief
  */
#include <stdint.h>
class Navigator
{
	public:
		Navigator(uint8_t THRESHOLD);
		uint16_t getDirection(uint8_t *src);
	private:
        uint8_t detected_flag;
        uint8_t threshold;
        uint16_t direction;
        uint16_t temp,i,j,k;
        uint16_t right_up, right_low, left_up, left_low;
        uint32_t UpperStartPosition;
        uint32_t LowerStartPosition;

        void calculateDirection();

        void getUpperRightPosition(uint8_t *src);
        void getLowerRightPosition(uint8_t *src);
        void getUpperLeftPosition (uint8_t *src);
        void getLowerLeftPosition (uint8_t *src);

        void UpperRightDetected();
        void LowerRightDetected();
        void UpperLeftDetected();
        void LowerLeftDetected();

        bool isRightDetected();
        bool isLeftDetected();
};
