#include "sys.h"
#include "stdio.h"
#include "OLED.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "JY62.h"
#include "PID.h"
#include "Driver.h"
#include "Software_iic.h"
#include <math.h>
#include <stdbool.h>

struct COLOR RGB;
int count = 0;
extern float VOFA_Data[4];
extern u8 already_turned, Y_have_achieved, X_have_achieved;
extern int32_t Y_Speed;
extern int32_t X_Speed;
extern int32_t angle_speed;

extern struct PID Coord, Turn_Angle_PID;
extern int32_t position_of_car[3];
extern int32_t CCR_wheel[4];

extern bool check_rgb(int color);


void OLED_SHOW_TASK()
{
    OLED_Clear();
    char buff[20];
	float current_angle = (float)stcAngle.Angle[2] / 32768 * 180;
    sprintf(buff, "Angle_Z:%.2f", current_angle);
    OLED_ShowString(1, 1, buff);
	sprintf(buff, "R:%d G:%d B:%d", RGB.R,RGB.G,RGB.B);
    OLED_ShowString(2, 1, buff);
    sprintf(buff, "RD:%.1f", VOFA_Data[2]);
    OLED_ShowString(3, 1, buff);
    sprintf(buff, "FD:%.1f", VOFA_Data[3]);
    OLED_ShowString(3, 9, buff);
    sprintf(buff, "%d", check_rgb(1));
    OLED_ShowString(4,1,buff);
}
void Read_RGB()
{
    IIC_Read_Len(0X5A, 0X0C, 3, (u8*)&RGB);
}
void Forward()
{
    Advance(2);
    Advance(3);
    Advance(4);
    Advance(5);
}
void Reverse()
{
    Back(2);
    Back(3);
    Back(4);
    Back(5);
}
void Turn_Right_Founction()
{
    Back(2);
    Advance(3);
    Advance(4);
    Back(5);
}
void Turn_Left_Founction()
{
    Advance(2);
    Back(3);
    Back(4);
    Advance(5);
}
void Walking_Right()
{
    Advance(2);
    Back(3);
    Advance(4);
    Back(5);
}
void Walking_Left()
{
    Back(2);
    Advance(3);
    Back(4);
    Advance(5);
}


//-------������HSV��ɫ�ռ��RGB��ɫ�ռ��໥ת���ӿ�------------
/**
 * @brief HSV��ɫ�ռ�
*/
typedef struct {
    float h;    // ɫ��H(hue)  0~360�� {R(0��),G(120��),B(240��)}
    float s;    // ���Ͷ�S(saturation)  0~1.0
    float v;    // ����V(value)  0~1.0  
}color_hsv_t;

typedef struct {
    unsigned char r;    // 0-255
    unsigned char g;    // 0-255
    unsigned char b;    // 0-255
    unsigned int color; // 0x000000 ~ 0xffffff
}color_rgb_t;


/**
 * @brief   RGB��ɫ�ռ� ת  HSV��ɫ�ռ� 
 * @param   rgb:RGB��ɫ�ռ����
 * @param   hsv:HSV��ɫ�ռ����
 * @return  none
*/
void rgb2hsv(color_rgb_t *rgb, color_hsv_t *hsv);

/**
 * @brief   HSV��ɫ�ռ� ת RGB��ɫ�ռ�
 * @param   hsv:HSV��ɫ�ռ����
 * @param   rgb:RGB��ɫ�ռ����
 * @return  none
*/
void hsv2rgb(color_hsv_t *hsv, color_rgb_t *rgb);

/**
 * @brief   ����HSV��ɫ�ռ� Hֵ(0.0~360.0)
 * @param   hsv:HSV��ɫ�ռ����
 * @param   h_offset:���ڲ��� ����>0.0����С<0.0
 * @return  none
*/
void hsv_adjust_h(color_hsv_t *hsv, float h_offset);

/**
 * @brief   ����HSV��ɫ�ռ� Sֵ(0.0~1.0)
 * @param   hsv:HSV��ɫ�ռ����
 * @param   s_offset:���ڲ��� ����>0.0����С<0.0
 * @return  none
*/
void hsv_adjust_s(color_hsv_t *hsv, float s_offset);

/**
 * @brief   ����HSV��ɫ�ռ� Vֵ(0.0~1.0)
 * @param   hsv:HSV��ɫ�ռ����
 * @param   v_offset:���ڲ��� ����>0.0����С<0.0
 * @return  none
*/
void hsv_adjust_v(color_hsv_t *hsv, float v_offset);



// ������HSV��ɫ�ռ��RGB��ɫ�ռ��໥ת���ӿ�
/*********************************************************************************************
�죺    
    R   255         H:0
    G   0           S:100
    B   0           V:100    
�̣�
    R   0           H:120   
    G   255         S:100
    B   0           V:100    
����
    R   0           H:240   
    G   0           S:100
    B   255         V:100    
*********************************************************************************************/

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max3(a,b,c) (((a) > (b) ? (a) : (b)) > (c) ? ((a) > (b) ? (a) : (b)) : (c))
#define min3(a,b,c) (((a) < (b) ? (a) : (b)) < (c) ? ((a) < (b) ? (a) : (b)) : (c))


/*********************************************************************************************
RGBת����HSV���㷨:
    max=max(R,G,B)��
    min=min(R,G,B)��
    V=max(R,G,B)��
    S=(max-min)/max��
    if (R = max) H =(G-B)/(max-min)* 60��
    if (G = max) H = 120+(B-R)/(max-min)* 60��
    if (B = max) H = 240 +(R-G)/(max-min)* 60��
    if (H < 0) H = H + 360��
***********************************************************************************************/

/**
 * @brief   RGB��ɫ�ռ� ת  HSV��ɫ�ռ� 
 * @param   rgb:RGB��ɫ�ռ����
 * @param   hsv:HSV��ɫ�ռ����
 * @note    The R,G,B values are divided by 255 to change the range from 0..255 to 0..1:
 * @return  none
*/
void rgb2hsv(color_rgb_t *rgb, color_hsv_t *hsv)
{
    float max, min, delta=0;
    float r = (float)((float)((int)rgb->r)/255);
    float g = (float)((float)((int)rgb->g)/255);
    float b = (float)((float)((int)rgb->b)/255);

    max = max3(r, g, b);
    min = min3(r, g, b);
    delta = (max - min);

    //printf("r:%f, g:%f, b:%f\n", r, g, b);

    if (delta == 0) {
        hsv->h = 0;
    } else {
        if (r == max) {
            hsv->h = ((g-b)/delta)*60; 
        } else if (g == max) {
            hsv->h = 120+(((b-r)/delta)*60); 
        } else if (b == max) {
            hsv->h = 240 + (((r-g)/delta)*60);
        }

        if (hsv->h < 0) {
            hsv->h += 360;
        }
    }

    if (max == 0) {
        hsv->s = 0;
    } else {
        hsv->s = (float)(delta/max);
    }

    hsv->v = max;

    rgb->color = (unsigned int)(((rgb->r&0xff)<<16) | ((rgb->g&0xff)<<8) | (rgb->b&0xff));
}


/*************************************************************************
HSVת����RGB���㷨:
    if (s = 0)
    R=G=B=V;
    else
    H /= 60;
    i = INTEGER(H);
    f = H - i;
    a = V * ( 1 - s );
    b = V * ( 1 - s * f );
    c = V * ( 1 - s * (1 - f ) );
    switch(i)
    case 0: R = V; G = c; B = a;
    case 1: R = b; G = v; B = a;
    case 2: R = a; G = v; B = c;
    case 3: R = a; G = b; B = v;
    case 4: R = c; G = a; B = v;
    case 5: R = v; G = a; B = b;
*******************************************************************************/

/**
 * @brief   HSV��ɫ�ռ� ת RGB��ɫ�ռ�
 * @param   hsv:HSV��ɫ�ռ����
 * @param   rgb:RGB��ɫ�ռ����
 * @note    When 0 �� H < 360, 0 �� S �� 1 and 0 �� V �� 1:
 * @return  none
*/
void hsv2rgb(color_hsv_t *hsv, color_rgb_t *rgb)
{
    int i;
    float f,a,b,c;

    float h = hsv->h;
    float s = hsv->s;
    float v = hsv->v;
	if (h >= 360) {
		h = 0;
	}
	
    if (s == 0) {
        rgb->r = (unsigned char)((int)(v*255));
        rgb->g = (unsigned char)((int)(v*255));
        rgb->b = (unsigned char)((int)(v*255));
    } else {
        h /= 60.0;  // sector 0 to 5, h_max=360 360/60=6[0,1,2,3,4,5]
        i = (int)floor(h); // floor(h)
        f = h-i; // factorial path of h
        a = v * (1-s);
        b = v * (1-s*f);
        c = v * (1-s*(1-f));
        switch(i) {
            case 0:
                rgb->r = (unsigned char)((int)(v*255)); //v*255
                rgb->g = (unsigned char)((int)(c*255)); //c*255;
                rgb->b = (unsigned char)((int)(a*255)); //a*255;
                break;
            case 1:
                rgb->r = (unsigned char)((int)(b*255)); //b*255;
                rgb->g = (unsigned char)((int)(v*255)); //v*255;
                rgb->b = (unsigned char)((int)(a*255)); //a*255;
                break;
            case 2:
                rgb->r = (unsigned char)((int)(a*255)); //a*255;
                rgb->g = (unsigned char)((int)(v*255)); //v*255;
                rgb->b = (unsigned char)((int)(c*255)); //c*255;
                break;
            case 3:
                rgb->r = (unsigned char)((int)(a*255));//a*255;
                rgb->g = (unsigned char)((int)(b*255));//b*255;
                rgb->b = (unsigned char)((int)(v*255));//v*255;
                break;
            case 4:
                rgb->r = (unsigned char)((int)(c*255)); //c*255;
                rgb->g = (unsigned char)((int)(a*255)); //a*255;
                rgb->b = (unsigned char)((int)(v*255)); //v*255;
                break;
            default:
                rgb->r = (unsigned char)((int)(v*255)); //v*255;
                rgb->g = (unsigned char)((int)(a*255)); //a*255;
                rgb->b = (unsigned char)((int)(b*255)); //b*255;
                break;
        }

    }

    rgb->color = (unsigned int)(((rgb->r&0xff)<<16) | ((rgb->g&0xff)<<8) | (rgb->b&0xff));
}

/**
 * @brief   ����HSV��ɫ�ռ� Hֵ(0.0~360.0)
 * @param   hsv:HSV��ɫ�ռ����
 * @param   h_offset:���ڲ��� ����>0.0����С<0.0
 * @return  none
*/
void hsv_adjust_h(color_hsv_t *hsv, float h_offset)
{
    hsv->h += h_offset;
    if (hsv->h >= 360) {
        hsv->h = 360;
    } else if (hsv->h<=0) {
        hsv->h = 0;
    }
}


/**
 * @brief   ����HSV��ɫ�ռ� Sֵ(0.0~1.0)
 * @param   hsv:HSV��ɫ�ռ����
 * @param   s_offset:���ڲ��� ����>0.0����С<0.0
 * @return  none
*/
void hsv_adjust_s(color_hsv_t *hsv, float s_offset)
{
    hsv->s += s_offset;
    if (hsv->s >= 1.0) {
        hsv->s = 1.0;
    } else if (hsv->s <= 0) {
        hsv->s = 0;
    }
}


/**
 * @brief   ����HSV��ɫ�ռ� Vֵ(0.0~1.0)
 * @param   hsv:HSV��ɫ�ռ����
 * @param   v_offset:���ڲ��� ����>0.0����С<0.0
 * @return  none
*/
void hsv_adjust_v(color_hsv_t *hsv, float v_offset)
{
    hsv->v += v_offset;
    if (hsv->v >= 1.0) {
        hsv->v = 1.0;
    } else if (hsv->v<=0) {
        hsv->v = 0;
    }
}



