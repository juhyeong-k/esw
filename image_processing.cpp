#include "system_management.h"
#include "image_processing.h"
extern std::ofstream fileout;

#define isBlue   0
#define isGreen  1
#define isRed    2
/**
  * @breif  BGR24_to_HSV class
  *          get Max(B,G,R), Min(B,G,R), V_BGR for BGR24 to HSV
  */
uint8_t BGR24_to_HSV::getMaxBGR_VBGR(uint8_t b, uint8_t g, uint8_t r, uint8_t *V_BGR) {
    // V_BGR : B = 0, G = 1, R = 2.
    *V_BGR = 0;
    uint8_t max = isBlue;
    if (g > max) { max = g; *V_BGR = isGreen; }
    if (r > max) { max = r; *V_BGR = isRed; }
    return max;
}
uint8_t BGR24_to_HSV::getMinBGR(uint8_t b, uint8_t g, uint8_t r) {
    uint8_t min = b;
    if (g < min) min = g;
    if (r < min) min = r;
    return min;
}
void BGR24_to_HSV::bgr24_to_hsv(uint8_t (*src)[VPE_OUTPUT_W*3], uint8_t (*des)[VPE_OUTPUT_W*3])
{
    int i, j, k;
    uint8_t temp_buf[VPE_OUTPUT_H][VPE_OUTPUT_W*3];
    memcpy(temp_buf, src, VPE_OUTPUT_IMG_SIZE);

    uint8_t V_BGR;
    uint8_t B, G, R; int16_t H; uint8_t S; uint8_t V;
    uint8_t Max, Min;
    for(i = 0; i < VPE_OUTPUT_H; i++)
    {
        for(j = 0; j < VPE_OUTPUT_W; j++)
        {
            k = j*3;
            B = temp_buf[i][j];    G = temp_buf[i][j+1];    R = temp_buf[i][j+2];
            Max = getMaxBGR_VBGR(B, G, R, &V_BGR);
            Min = getMinBGR(B, G, R);// Obtaining V
            V = Max;
            // Obtaining S
            if (V == 0)    S = 0;
            else           S = 255 * (float)(V - Min) / V;
            // Obtaining H
            switch(V_BGR)
            {
                case isBlue  : H = 240 + (float)60 * (R - G) / (V - Min); break;    // V is Blue
                case isGreen : H = 120 + (float)60 * (B - R) / (V - Min); break;    // V is Green
                case isRed   : H =       (float)60 * (G - B) / (V - Min); break;    // V is Red
                default : H = 0;                                      break;
            }
            if(H < 0)    H = H + 360;
            H = H / 2;
            des[i][j] = H; des[i][j+1] = S; des[i][j+2] = V;
        }
    }
}
/**
  * @breif  Draw class
  *
  */
void Draw::horizontal_line(uint8_t (*des)[VPE_OUTPUT_W*3], uint16_t y)
{
    #ifdef bgr24
        int i,j;
        int index = y * VPE_OUTPUT_W * 3;
        for(i = 0; i < VPE_OUTPUT_W; i++)
        {
            j = 3 * i;
            **(des + index + j) = 255;
            **(des + index + j + 1) =**(des + index + j + 2) = 0;
        }
    #endif
}
void Draw::vertical_line(uint8_t (*des)[VPE_OUTPUT_W*3], uint16_t x)
{
    #ifdef bgr24
        int i,j;
        int index = x * 3;
        for(i = 0; i < VPE_OUTPUT_H; i++)
        {
            j = index + 3 * VPE_OUTPUT_W * i;
            **(des+j)  = 255;
            **(des+j+1) = **(des+j+2) = 0;
        }
    #endif
}
void Draw::dot(uint8_t (*des)[VPE_OUTPUT_W*3], uint16_t x, uint16_t y)
{
    #ifdef bgr24
        int i,j;
        uint32_t StartPosition = y * VPE_OUTPUT_W * 3 + x * 3 + 1;
        for(i = 0; i < 3; i++) {
            **(des + StartPosition - 3 + 3*i) = 255;
        }
        for(i = -1; i < 2; i++) {
            j = StartPosition + 3*VPE_OUTPUT_W*i;
            **(des+j) = 255;
            **(des + j + 1) = **(des + j + 2) = 0;
        }
    #endif
}
/**
  * @breif  colorFilter class
  *
  */
colorFilter::colorFilter(uint8_t colorName)
{
    switch(colorName)
    {
        case YELLOW :
            HUE_MAX = yellow_HUE_MAX;
            HUE_MIN = yellow_HUE_MIN;
            SAT_MAX = yellow_SAT_MAX;
            SAT_MIN = yellow_SAT_MIN;
            VAL_MAX = yellow_VAL_MAX;
            VAL_MIN = yellow_VAL_MIN;
            break;
        default : fileout << "colorName was not defined(image_processing.cpp)\n"; break;
    }
}
void colorFilter::detectColor(uint8_t (*src)[VPE_OUTPUT_W*3], uint8_t (*des)[VPE_OUTPUT_W*3])
{
    int i, j;
    uint8_t h,s,v;
    uint8_t temp_buf[VPE_OUTPUT_IMG_SIZE];
    memcpy(temp_buf, src, VPE_OUTPUT_IMG_SIZE);
    for(i = 0; i < VPE_OUTPUT_RESOLUTION; i++)
    {
        j = 3 * i;
        h = temp_buf[j];
        s = temp_buf[j+1];
        v = temp_buf[j+2];
        if(inRange(h,s,v))
            **(des+j) = **(des+j+1) = **(des+j+2) = 255;
        else
            **(des+j) = **(des+j+1) = **(des+j+2) = 0;
    }
}
bool colorFilter::inRange(uint8_t h, uint8_t s, uint8_t v)
{
    if( ( HUE_MIN < h && h < HUE_MAX ) && 
        ( SAT_MIN < s && s < SAT_MAX ) &&
        ( VAL_MIN < v && v < VAL_MAX) )
          return true;
    else return false;
}
