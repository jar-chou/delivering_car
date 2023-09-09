/*
 * @Author: jar-chou 2722642511@qq.com
 * @Date: 2023-09-09 17:15:48
 * @LastEditors: jar-chou 2722642511@qq.com
 * @LastEditTime: 2023-09-09 18:09:55
 * @FilePath: \delivering_car\User\oled\oled_draw.c
 * @Description: 
 */

#include "oled_draw.h"
#include "oled_buffer.h"
#include "OLED_Font.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
//显示一个字符
//关于字体尺寸及使用请看SetFontSize()的注释
//当size=0时 x为第几行 y为第几列
void DrawChar(int x, int y, unsigned char c)
{
	int i;
	if((x>4) || (y > 16))
		return;
	c = c - ' ';			//得到偏移后的位置
	y = 8 * y;
	for(i=0;i<8;i++)
		WriteByteBuffer(x*2, y+i, *(OLED_F8x16[c]+i));
	for(i=0;i<8;i++)
		WriteByteBuffer(x*2+1, y+i, *(OLED_F8x16[c]+i+8));
}
//显示字符串 就是显示多次显示字符
void DrawString(int x, int y,char *str)
{
	unsigned char j=0;
	
	while (str[j]!='\0')
	{
		DrawChar(x, y, str[j]);
		y++;
		if(y>15)
		{
			y=0;
			x++;
		}		//换行
		j++;
	}
}

/**
  * @brief  OLED次方函数
  * @retval 返回值等于X的Y次方
  */
uint32_t int_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}
//显示数字 就是多次显示数字的字符
/**
 * @description: 
 * @param {unsigned char} x	第几行
 * @param {unsigned char} y	第几列
 * @param {unsigned int} num	要显示的数字
 * @param {unsigned char} len	要显示的数字的位数
 * @return {*}
 */
void DrawNum(unsigned char x,unsigned char y,unsigned int num,unsigned char len)
{
	unsigned char t,temp;
	unsigned char enshow=0;
	for(t=0;t<len;t++)
	{
		temp=(num/int_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)	
			{
				DrawChar(x+(8*t),y,' ');
				continue;
			}else enshow=1;
			
		}
	 	DrawChar(x+(8*t),y,temp+'0');
	}

	
} 
//////////////////////////////////////////////////////////////////////////////////////////////////
//显示汉字
// void OLED_ShowCHinese(uint8_t x,uint8_t y,uint8_t *cn)
// {      			    
// 	uint8_t j,wordNum;

// 	if((x > 7)||(y>128-16))
// 		return;
	
// 	while ( *cn != '\0')	 																	//在C语言中字符串结束以‘\0’结尾
// 	{
// 		for (wordNum=0; wordNum<NUM_OFCHINESE; wordNum++)
// 		{
// 			if ((CN16CHAR[wordNum].Index[0] == *cn)&&(CN16CHAR[wordNum].Index[1] == *(cn+1)))   //查询要写的字在字库中的位置
// 			{
// 				for (j=0; j<32; j++) 															//写一个字
// 				{		
// 					if (j == 16)	 															//由于16X16用到两个Y坐标，当大于等于16时，切换坐标
// 					{
// 						x++;
// 					}			
// 					WriteByteBuffer(x,y+(j%16),CN16CHAR[wordNum].Msk[j]);
// 				}
// 				y += 16;
// 				x--;
// 				if(y > (128-16))
// 				{x += 2;y = 0;}
// 			}
// 		}
// 		cn += 2;																																							//此处打完一个字，接下来寻找第二个字
// 	}
// }
