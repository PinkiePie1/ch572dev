#include "miniGUI.h"
#include "CH57x_common.h"

#define FONT_GETWIDTH(x) (x[1])
#define FONT_GETHEIGHT(x) (x[0])
/*
写显存的重新实现
*/

uint8_t *image;//指向显存的指针

//指定显存位置。用户应该在主函数中调用这个函数
//并为它准备好一个5000的数组作为显存。
void paint_SetImageCache(uint8_t *imagePtr)
{
	image = imagePtr;
}

//设定某个像素的值
__HIGH_CODE
static inline void setPixel(uint16_t x, uint16_t y, uint8_t color)
{
//    x = x>128?128:x; 
//    y = y>295?295:y; 
      //这两行可以避免越界访问
      //但太占地方了,指令多了将近一半
      //所以把不越界的责任交给用户。
    uint16_t index = (x>>3)*152 + y; 
    //像素所对应的字节的位置
    if( color )
    {
	    //x%8可以用x&(8-1)代替,速度更快。
	    //但这招只在取余的数是2的指数倍的时候能用。
	    image[index] |= (0x80 >> (x&7UL));
	}
	else
	{
		image[index] &= ~(0x80 >> (x&7UL));
	}

}

//放大式地画像素，比如2倍放大就会在对应位置画四个像素。注意xy值也会被相应的放大。
__HIGH_CODE
static inline void scaledSetPixel(uint16_t x, uint16_t y, uint8_t color, uint8_t scale)
{
	;//暂未实现。
}

//画线。只能画横着或者竖着的线。斜着的线没有必要。
void drawLine(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint8_t color)
{
	if(xStart==xEnd)
	{
	    uint16_t i = (yStart>yEnd) ? yEnd : yStart;
	    uint16_t end = (i==yEnd) ? yStart : yEnd;
		for(;i<=end;i++)
		{
			setPixel(xStart,i,color);
		}
	}
	else if (yStart==yEnd)
	{
	    uint16_t i = (xStart>xEnd) ? xEnd : xStart;
	    uint16_t end = (i==xEnd) ? xStart : xEnd;
		for(;i<=end;i++)
		{
			setPixel(i,yStart,color);
		}
	}
	
}

//画方框
void drawRect(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint8_t color)
{
	drawLine(xStart, yStart, xEnd, yStart, color);
	drawLine(xEnd, yStart, xEnd, yEnd, color);
	drawLine(xEnd, yEnd, xStart, yEnd, color);
	drawLine(xStart, yEnd, xStart, yStart, color);
}


//画实心方块
void fillRect(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint8_t color)
{
	uint16_t i = yStart<yEnd ? yStart : yEnd;
	uint16_t end = ( i==yStart ) ? yEnd : yStart;
	for ( ; i <= end; i++  )
	{
		drawLine(xStart,i,xEnd,i,color);
	}
}

//  快速画方块，注意第三和第四个参数是长度，不是终点。
//  在需要擦除一块文字或者画面时会有用
//  未实现。
void fastFill(uint16_t x, uint16_t y, uint16_t xblock, uint16_t yblock, uint8_t color)
{
	uint16_t index;
	//这是边缘的像素块，需要相应的移位到正确的地方
	uint8_t mask1 = color ? ~(0xFF >> ((x+xblock+1)&7)) : (0xFF >> ((x+xblock+1)&7));
	uint8_t mask2 = color ?  (0xFF>>(x&7)) : ~(0xFF>>(x&7));
	uint8_t tmp1;
	uint8_t tmp2;

	//覆盖对应的位置，先覆盖头，再覆盖中间，最后覆盖尾。
    for( uint16_t i = y; i <= (y+yblock); i++ )
    {	
		index = (x>>3) + (i*25);
		tmp1 = image[(x>>3) + (i*25)];
		tmp2 = image[((x+xblock)>>3) + (i*25)];
		
		image[index] = color ? ( tmp1 | mask2 ) : ( tmp1 & mask2 );
		
    	for (uint16_t j = x+8; j < ( x + xblock ); j += 8 )
    	{
    		index = (j>>3) + (i*25);
    		image[index] = color;
		}
				
		index = ((x+xblock)>>3) + (i*25);
		image[index] = color ? ( tmp2 | mask1 ) : ( tmp2 & mask1 );
    }	

}

//快速画方框，注意不检查参数顺序是否合理,输入的End必须大于start。
void fastRect(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint8_t color)
{	
	uint16_t xblock = xEnd-xStart;
	uint16_t yblock = yEnd-yStart;
	uint16_t index;
	uint8_t tmp;
	//这是边缘的像素块，需要相应的移位到正确的地方
	//同时需要利用-1移位是补1不是补0
	int8_t mask1 = color ? 0x80 >> ((xEnd)&7) : ~(0x80>>((xEnd)&7));
	int8_t mask2 = color ? 0x80 >> ((xStart)&7) : ~(0x80)>>((xStart)&7);

	//覆盖对应的位置
    for( uint16_t i = yStart; i <= (yEnd); i++ )
    {	
		index = (xStart>>3) + (i*25);
		tmp = image[index];
		image[index] = color ? ( tmp | mask2 ) : ( tmp & mask2 );

		if( i==yStart || i==yEnd )
		{
			tmp = image[(xStart>>3)+(i*25)];
			image[(xStart>>3)+(i*25)] = color ? 
			                            (tmp | (0xFF>>(xStart&7))) :
			                            (tmp & ~(0xFF>>(xStart&7)));
			                            
			tmp = image[(xEnd>>3)+(i*25)];
			image[(xEnd>>3)+(i*25)] = color ?
			                          (tmp | ~(0xFF>>(xEnd&7))) :
			                          (tmp & (0xFF>>(xEnd&7))); 
			                          
	    	for (uint16_t j = xStart+8; j < xEnd; j += 8 )
	    	{
	    		index = (j>>3) + (i*25);
	    		image[index] = color;
	    		
			}
			
		}	
					
		index = (xEnd>>3) + (i*25);
		tmp = image[index];
		image[index] = color ? ( tmp | mask1 ) : ( tmp & mask1 );
    }	
	
}


//快速显示图像，图像的的宽度必须是200，两个参数指定了图像
//在屏幕上的起点和终点。
//未实现
void FastImg(uint16_t xStart, uint16_t xEnd, const char *imgDat)
{
	uint32_t length = (xEnd-xStart) << 4;
	memcpy( (void *)( (uint32_t)image+ (xStart<<4)), 
	imgDat, 
	length);
}

//快速画单个字
void fastDrawChar(uint16_t xStart, uint16_t yStart, char chara, const char *font, uint8_t color)
{	
	if (FONT_GETHEIGHT(font) == 16)
	{	
    	uint16_t index = ((xStart>>3)*152) + yStart;
		if(color){
    	memcpy( image+index , font+(chara-' '+1)*16 ,16);
		} else {
			for(uint8_t i = 0; i<16; i++)
			{
				image[index+i] = ~font[(chara-' '+1)*16+i];
			}
		}
	}
}

static void fastShiftedChar(uint16_t xStart, uint16_t yStart, char chara, const uint8_t *font, uint8_t color)
{
	if (FONT_GETHEIGHT(font) == 16)
	{	
		uint8_t offset = xStart&7UL;
		uint16_t index = ((xStart>>3)*152) + yStart;
		if(color)
		{
			for(uint8_t i = 0; i< 16; i++)
			{
				uint8_t temp = font[(chara-' '+1)*16+i];
				image[index+i] &= ~(0xFF>>offset);
				image[index+i] |= (temp>>offset);
				image[index+152+i] &= ~(0xFF<<(8-offset));
				image[index+152+i] |= (temp<<(8-offset));
			}
		}
		else
		{
			for(uint8_t i = 0; i< 16; i++)
			{
				uint8_t temp = ~font[(chara-' '+1)*16+i];
				image[index+i]       &= ~(0xFF >> offset);
				image[index+i]       |=  (temp   >> offset);
				image[index+152+i]   &= ~(0xFF << (8 - offset));
				image[index+152+i]   |=  (temp   << (8 - offset));
			}
		}
	}
}

//快速画字符串。
void fastDrawString(uint16_t xStart, uint16_t yStart, char *stringToPrint, const char *font, uint8_t color)
{
	if (FONT_GETHEIGHT(font) == 16)
	{
		uint16_t y = yStart;
		uint16_t x = xStart;
		if( (xStart&7UL) == 0 ) //如果起点能被8整除，则每次只需要memcpy即可
		{	
			for( ; *stringToPrint; stringToPrint++)
			{			
				fastDrawChar(x,y,*stringToPrint,font,color);
				y = (x<136) ? y : y+16 ;
				x = (x<136) ? x+8  : xStart;
			}
		}
		else
		{
			for( ; *stringToPrint; stringToPrint++)
			{	
				fastShiftedChar(x,y,*stringToPrint,font,color);
				y = (x<136) ? y : y+16 ;
				x = (x<136) ? x+8  : xStart;	
			}	
		}	
	}
	
}

//正常画单个字符，可支持12、18等非8字节对齐的字符大小和任意位置，可指定颜色
void drawChar(uint16_t xStart, uint16_t yStart, char charToPrint, const char *font, uint8_t color)
{
	uint8_t height = FONT_GETHEIGHT(font);
	uint8_t width = FONT_GETWIDTH(font);
	int8_t shift = ((FONT_GETHEIGHT(font)-1)>>3)+1;
	for (int i = 0; i<width; i++)
	{
		for(int j = 0; j<height; j++)
		{
			if(font[width*shift*(charToPrint-' '+1)+i*shift+(j>>3)] & (0x80>>(j&7)))
				{setPixel(xStart+j,yStart+i,color);}
			else
				{setPixel(xStart+j,yStart+i,~color);}
		}
	}

}

//正常画字符串
void drawStr(uint16_t xStart, uint16_t yStart,char *stringToPrint, const char *font, uint8_t color)
{
	int x = xStart;
	int y = yStart;
	int width = FONT_GETWIDTH(font);
	for( ; *stringToPrint; stringToPrint++)
	{
		x = (y>=width)&&(*stringToPrint!='\n')? x : x+FONT_GETHEIGHT(font);
		y = (y>=width)? y-width : yStart-width;
		y = (*stringToPrint=='\n')? yStart:y;
		drawChar(x,y,*stringToPrint,font,color);	

	}	
	
}

void EPD_Printf(uint16_t xStart, uint16_t yStart, const char *font, uint8_t color, const char *format, ...)
{
	va_list args;
	va_start(args,format);
	char buffer[40];
	vsnprintf(buffer,40, format, args);
	va_end(args);
	fastDrawString(xStart,yStart,buffer,font,color);
}
