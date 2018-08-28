class Calibrator
{
	public:
		bool wait_properEnviroment();
		void vertical(uint8_t *mono_src);
		void horizontal(uint8_t *mono_src);
		bool isReady(const char* str, uint8_t *mono_src);
	private:
		bool isVertical_Ready(uint8_t *mono_src);
		bool isHorizontal_Ready(uint8_t *mono_src);
		void draw_dot(uint8_t *des, uint16_t x, uint16_t y);
};