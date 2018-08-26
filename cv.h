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
////////////////////////////////////////////////////////////////////////////////
/*
{
    if (flag == 15)    {
        vector = (float)(right_high - right_low) / (LOWER_LINE - UPPER_LINE) + (float)(left_high - left_low) / (LOWER_LINE - UPPER_LINE);
    }
    else if ((flag & 1) && (flag & 2))     {
        vector = (float)(right_high - right_low) / (LOWER_LINE - UPPER_LINE);
    }
    else if ((flag & 4) && (flag & 8))     {
        vector = (float)(left_high - left_low) / (LOWER_LINE - UPPER_LINE);
    }
    else    {
        printf("Didn't detected\n");
    }
    printf("vector : %f\n", vector);

    if(vector > 1.11)         return 1000;
    else if (vector < -1.11) return 2000;
    else                       return (uint16_t)(1500 - 450 * vector);
}
*/