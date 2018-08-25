/**
  * @brief
  */
class BGR24_to_HSV
{
    public:
        void bgr24_to_hsv(uint8_t *src, uint8_t *des);
    private:
        uint8_t getMaxBGR_VBGR(uint8_t b, uint8_t g, uint8_t r, uint8_t *V_BGR);
        uint8_t getMinBGR(uint8_t b, uint8_t g, uint8_t r);
};
class Draw
{
	public:
		void horizontal_line(uint8_t *des, uint16_t y);
		void vertical_line(uint8_t *des, uint16_t x);
};
class colorFilter
{
	public:
		colorFilter(uint8_t colorName);
		void detectColor(uint8_t *src, uint8_t *des);
	private:
		uint8_t HUE_MAX;
		uint8_t HUE_MIN;
		uint8_t SAT_MAX;
		uint8_t SAT_MIN;
		uint8_t VAL_MAX;
		uint8_t VAL_MIN;
};