/****************************************************************
 * File Name          : Main.c
 * Author             : JCQ
 * Version            : V1.0
 * Date               : 2025/04/07
 * Description        : 裸机驱动墨水屏
 *********************************************************************************/
#include "CH57x_common.h"
#include "main.h"
#include "miniGUI.h"
#include "MAX30102.h"
#include "EPD_1IN54_SSD1680.h"
#include <CH572rf.h>
//#include <stdlib.h>

#define POWER_PIN 0 //如果用5V，这里改成1

uint32_t humid;
uint32_t temperature;
__attribute__((__aligned__(4))) uint8_t imageCache[2888]={0};
__attribute__((__aligned__(4))) uint8_t buf[6*32]={0};
__attribute__((__aligned__(4))) uint16_t Meas_Data[152]={0};
uint8_t refreshCount = 250;
uint8_t img_index = 0;
uint8_t rawData[9];
uint8_t MacAddr[6]={0x1A,0x2A,0x3A,0x4A,0x5A,0x6A};
rfRoleConfig_t conf ={0};

void MySleep(uint8_t use5v);

rfipTx_t gTxParam;
uint8_t tx_flag = 0;
__attribute__((__aligned__(4))) uint8_t TxBuf[64];

const uint8_t advert_data[]={
	0x02,
	0x00,//长度,后面再填入
	0x1A,0x2A,0x3A,0x4A,0x5A,0x6A,//MAC地址，反过来的
	0x05,0x09,'t','e','m','p', //完整名字
	0x07,0xFF,0xFF,0xFF,0x00,0x00,0x01,0x02 //18-21	
};

__HIGH_CODE
void MySleep(uint8_t use5v)
{
    uint16_t ldoconfig = 0;
	if(use5v)
    {
        ldoconfig |= RB_PWR_LDO5V_EN;
    }
    sys_safe_access_enable();
	R8_CLK_SYS_CFG = CLK_SOURCE_HSE_PLL_60MHz;
    sys_safe_access_disable();

	PFIC->SCTLR |= (1 << 2); //deep sleep
    ldoconfig |= RB_PWR_PLAN_EN | RB_PWR_CORE | RB_PWR_RAM12K | RB_PWR_EXTEND | RB_XT_PRE_EN |(1<<12) ;
    sys_safe_access_enable();
 	R8_SLP_POWER_CTRL |= 0x40; //longest wake up delay
  	R16_POWER_PLAN = ldoconfig;
    sys_safe_access_disable();
	
	asm volatile ("wfi\nnop\nnop" );
	uint16_t i = 400;
    do {
    __nop();
    }while (i--);
    sys_safe_access_enable();
    R16_POWER_PLAN &= ~RB_PWR_PLAN_EN;
    R16_POWER_PLAN &= ~RB_XT_PRE_EN;
    R8_CLK_SYS_CFG = CLK_SOURCE_HSE_PLL_100MHz;
    sys_safe_access_disable();

}

__HIGH_CODE
void RF_ProcessCallBack( rfRole_States_t sta,uint8_t id  )
{

	if( sta&RF_STATE_TX_FINISH )
    {
    	tx_flag = 0;
    }
    if( sta&RF_STATE_TIMEOUT )
    {
    	  	
    }
}

void main(void)
{
	uint8_t line = 0;

    sys_safe_access_enable();
    R8_CLK_SYS_CFG = CLK_SOURCE_HSE_PLL_100MHz;
    sys_safe_access_disable();
    
	//GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
	EPD_Hal_Init();
    //GPIOA_ITModeCfg(EPD_BUSY_PIN, GPIO_ITMode_FallEdge); 
    //PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_GPIO_WAKE, Fsys_Delay_4096);

	EPD_Init();
	paint_SetImageCache(imageCache);	
	EPD_Printf(0, line*16, font16, BLACK, "MAX30102 demo.");
	line++;
	EPD_SendDisplay(imageCache);
	WAIT_BUSY;
	EPD_Sleep();

	SoftI2CInit();

	//Read chip id.	
	MAX30102_ReadReg( 0xFF, buf, 1 );
	if( buf[0] == 0x15 )
	{
		EPD_Printf(0, line*16, font16, BLACK,"MAX30102 OK");
	} 
	else 
	{
		EPD_Printf(0, line*16, font16, BLACK,"MAX30102 ERR");
	}
	line++;
	//EPD_PartialDisplay(imageCache);
	//WAIT_BUSY;
	//EPD_Sleep();

	//Reset chip.
	uint8_t dat[5] = {0};
	dat[0] = 0x01<<6;
	MAX30102_WriteReg( 0x09, dat, 1);
	DelayMs(10);
	MAX30102_ReadReg( 0x09, buf, 1 );
	if( buf[0] == 0)
	{
		EPD_Printf(0, line*16, font16, BLACK,"RESET OK");
	} 
	else 
	{
		EPD_Printf(0, line*16, font16, BLACK,"RESET ERR, return: %d",buf[0]);
	}
	line++;
	//EPD_PartialDisplay(imageCache);
	//WAIT_BUSY;
	//EPD_Sleep();

	//Get temperature
	dat[0] = 0x01<<1;
	MAX30102_WriteReg( 0x03, dat, 1);//enable temperature interrupt
	dat[0] = 0x01;
	MAX30102_WriteReg( 0x21, dat, 1);//enable temperature measurement
	uint16_t overtime = 0;
	while(1)
	{
		MAX30102_ReadReg( 0x01, buf, 1);//check temp measure is complete or not.
		if(buf[0] == 0x01<<1){break;}
		else{DelayMs(1);overtime++;}
	}

	if( overtime < 1000 )
	{	
		EPD_Printf(8*10+1, line*16, font16, BLACK, "INT:%d",buf[0]); //print int status
		MAX30102_ReadReg( 0x1F, buf, 2); //read temp
		EPD_Printf(0, line*16, font16, BLACK,"TEMP:%02d.%02d", buf[0], buf[1]*6+buf[1]>>2); //show temp
	}
	else
	{
		EPD_Printf(0, line*16, font16, BLACK,"TEMP OVERTIME.");
	}
	line++;
	EPD_PartialDisplay(imageCache);
	WAIT_BUSY;
	EPD_Sleep();

	//Set LED pulse, ADC resolution and averaging
measure:
	dat[0] = 0x30; //fifo rollover enabled, average over 2 samples.
	MAX30102_WriteReg( 0x08, dat, 1);

	dat[0] = 0x61;//highest adc range, 50 samples per second,16bit resolution
	MAX30102_WriteReg( 0x0A, dat, 1);

	memset(dat,0x00,4);
	MAX30102_WriteReg( 0x04, dat, 4);//clear fifo.

	dat[0] = 0xC0;
	MAX30102_WriteReg( 0x02, dat, 1);//set interrupt

	//pulse amplitude
	dat[0] = 0x2F;
	dat[1] = 0x2F;
	MAX30102_WriteReg( 0x0C, dat, 2);

	dat[0] = 0x02;//heartrate mode,turn on both LEDs, start reading.
	MAX30102_WriteReg( 0x09, dat, 1);

	uint16_t samples_to_collect = 152;
	uint16_t samples_to_ignore = 3;
	uint8_t l = 0;

	while(samples_to_ignore > 0)
	{	
		DelayMs(10);
		MAX30102_ReadReg( 0x04, buf, 3);
		l = (buf[0] >= buf[2]) ? buf[0]-buf[2] : buf[0]+32-buf[2];
		if(l==0){continue;}
		MAX30102_ReadReg( 0x07, buf, 3*l );
		l = samples_to_ignore > l ? l : samples_to_ignore;
		samples_to_ignore -= l;
	}
	
	while( samples_to_collect != 0 )
	{
		DelayMs(1000);
		MAX30102_ReadReg( 0x04, buf, 3);//read fifo read and write pointer

		l = (buf[0] >= buf[2]) ? buf[0]-buf[2] : buf[0]+32-buf[2];
		if(l==0){continue;}
		//read fifo
		memset(buf,0x00,3*l);
		MAX30102_ReadReg( 0x07, buf, 3*l );
		//process data
		uint16_t data_index = 152-samples_to_collect;
		l = samples_to_collect > l ? l : samples_to_collect;
		samples_to_collect -= l;
		for(uint16_t i=0; i<l;i++)
			{
				Meas_Data[data_index]= (buf[i*3+1] << 8) | buf[i*3+2];
				data_index++;

			}
		
	}
	
	//running average
	for(uint16_t i = 0; i<150;i++)
	{
		Meas_Data[i] = ( Meas_Data[i] >> 1 ) + ( Meas_Data[i+1] >> 2 ) + ( Meas_Data[i+2] >> 2 );
	}
	
	uint32_t peak_meas = 0;
	uint32_t min_meas = 0xFFFF;
	for(uint16_t i = 0; i<150;i++)
	{
		if( Meas_Data[i] > peak_meas )
		{
			peak_meas = Meas_Data[i];
		}

		if( min_meas > Meas_Data[i] )
		{
			min_meas = Meas_Data[i];
		}

	}

	uint32_t amplitude = peak_meas-min_meas;
	//amplitude = amplitude/128;
	

	for(uint16_t i = 0; i<150;i++)
	{
		Meas_Data[i] = Meas_Data[i]-min_meas;
		Meas_Data[i] = (uint32_t)Meas_Data[i]*128/amplitude;
	}
	
	memset(imageCache,0x00,2888);

	EPD_Printf(0,0,font16,BLACK,"IR max:%d,min:%d",peak_meas,min_meas);
	
	for(uint16_t i = 0; i<149;i++)
	{
		drawLine(i,Meas_Data[i]+20,i,Meas_Data[i+1]+20,BLACK);
		//drawPixel(i,Meas_Data[i]+20,BLACK);
	}

	if(refreshCount>8)
	{
	EPD_Init();
	EPD_SendDisplay(imageCache);
	WAIT_BUSY;
	EPD_Sleep();
	refreshCount=0;
	}
	else
	{
	EPD_PartialDisplay(imageCache);
	WAIT_BUSY;
	EPD_Sleep();
	refreshCount++;
	}
	dat[0] = 1 << 7;
	MAX30102_WriteReg(0x09,dat,1);//shutdown.

	DelayMs(20000);
	goto measure;
	while(1);

}

__INTERRUPT
__HIGH_CODE
void GPIOA_IRQHandler(void)
{
    GPIOA_ClearITFlagBit(0xFFFF);
    
}

__INTERRUPT
__HIGH_CODE
void RTC_IRQHandler(void)
{
	R8_RTC_FLAG_CTRL =  RB_RTC_TRIG_CLR;
	
}

//这个不能漏掉
__INTERRUPT
__HIGH_CODE
void LLE_IRQHandler( void )
{
    LLE_LibIRQHandler( );
}

__INTERRUPT
__HIGH_CODE
void BB_IRQHandler( void )
{
    BB_LibIRQHandler( );
}

