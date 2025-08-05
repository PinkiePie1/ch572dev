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
#include "imageData.h"
#include "SHT40.h"
#include "EPD_1IN54_SSD1681.h"
#include <CH572rf.h>
#include <stdlib.h>

#define POWER_PIN 0 //如果用5V，这里改成1

uint32_t humid;
uint32_t temperature;
uint8_t *imageCache;
uint8_t refreshCount = 250;
uint8_t img_index = 0;
uint8_t rawData[9];
uint8_t MacAddr[6]={0x1A,0x2A,0x3A,0x4A,0x5A,0x6A};
rfRoleConfig_t conf ={0};

//sleep.



void MySleep(uint8_t use5v);

rfipTx_t gTxParam;
uint8_t tx_flag = 0;
__attribute__((__aligned__(4))) uint8_t TxBuf[64];

const uint8_t advert_data[]={
	0x02,
	0x00,//长度,后面再填入
	0x1A,0x2A,0x3A,0x4A,0x5A,0x6A,//MAC地址，反过来的
	0x05,0x09,'T','E','M','P', //完整名字
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
    sys_safe_access_enable();
    R8_CLK_SYS_CFG = CLK_SOURCE_HSE_PLL_100MHz;
    sys_safe_access_disable();
    
	GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);

	EPD_Hal_Init();
    GPIOA_ITModeCfg(EPD_BUSY_PIN, GPIO_ITMode_FallEdge); 
    PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_GPIO_WAKE, Fsys_Delay_4096);
	EPD_Init();	
	EPD_Sleep();
	SoftI2CInit();

	//读取芯片MAC地址
	//有需求可以启用这一行，不过会多800k字节的内存需求。
	//FLASH_EEPROM_CMD( CMD_GET_ROM_INFO, ROM_CFG_MAC_ADDR, MacAddr, 0 );

    sys_safe_access_enable( );
    R32_MISC_CTRL = (R32_MISC_CTRL&(~(0x3f<<24)))|(0xe<<24);
    sys_safe_access_disable( );
	
	//基本设置

	conf.rfProcessCB = RF_ProcessCallBack;
	conf.processMask = RF_STATE_TX_FINISH|RF_STATE_TIMEOUT;
	RFRole_BasicInit( &conf );

	//tx相关参数
	gTxParam.accessAddress = 0X8e89bed6;
	gTxParam.accessAddressEx = 0;
	gTxParam.crcInit = 0X555555;
	gTxParam.crcPoly = 0x80032d;
	gTxParam.properties = 0;
	gTxParam.waitTime = 80*2;
	gTxParam.txPowerVal = LL_TX_POWEER_0_DBM;
	gTxParam.whiteChannel=0x37; // whitening channel
	gTxParam.txLen = 34;
    gTxParam.frequency = 37;//37信道，注意和上面的0x37对应但不一样。考虑
    gTxParam.txDMA = (uint32_t)TxBuf;
    gTxParam.waitTime = 40*2; // 如果需要切换通道发送，稳定时间不低于80us

	sys_safe_access_enable();
	R32_RTC_TRIG = 0;
	R32_RTC_CTRL |= RB_RTC_LOAD_HI;
	R32_RTC_CTRL |= RB_RTC_LOAD_LO;
	R8_RTC_MODE_CTRL |= RB_RTC_TRIG_EN;  //enable RTC trigger
   	R8_SLP_WAKE_CTRL |= RB_SLP_RTC_WAKE; // enable wakeup control
	sys_safe_access_disable();

	PFIC_EnableIRQ( BLEB_IRQn );
    PFIC_EnableIRQ( BLEL_IRQn );
    PFIC_EnableIRQ( RTC_IRQn );
    PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_RTC_WAKE, Fsys_Delay_4096);//开启RTC唤醒使能

measure:
	// 初始化发送的数据
	memcpy(TxBuf,advert_data,sizeof(advert_data));
	memcpy(TxBuf+2,MacAddr,6);
	TxBuf[1] = (uint8_t) (sizeof(advert_data)-2);


	SHT40_beginMeasure();
	DelayMs(3);
	SHT40_getDat(rawData);

	humid = (rawData[3] << 8);
	humid |= rawData[4];
	humid = (humid >> 3) + (humid >> 4) + (humid >> 9) + (humid >> 10);

	temperature = (rawData[0] << 8);
	temperature |= rawData[1];
	temperature = (temperature >> 2) + (temperature >> 6) + (temperature >> 10);

	temperature = temperature - 4500;
	humid = humid-600;

	TxBuf[18] = ( (humid/1000) << 4) | ( (humid%1000) / 100 );
	TxBuf[19] = ( ((humid%100)/10) << 4 ) | humid%10;
	TxBuf[20] =  ( (temperature/1000) << 4 ) | ((temperature%1000) / 100);
	TxBuf[21] = ( ((temperature%100)/10) << 4 ) | temperature %10;

	imageCache = malloc(5000);
	uint8_t textcolor = BLACK;
	img_index = (0x0001&temperature);
	if(imageCache != NULL)
	{
		if (img_index == 0)
		{
			memcpy(imageCache,gImage_alicew,5000);
			textcolor = BLACK;
		}
		else if (img_index == 1)
		{
			memcpy(imageCache,gImage_aliceb,5000);
			textcolor = WHITE;
		}
		else
		{
			img_index = 0;
		}

		paint_SetImageCache(imageCache);

		EPD_Printf(0,0,font16,textcolor,"T:%02d.%02d",temperature/100,temperature%100);
		EPD_Printf(0,16,font16,textcolor,"H:%02d.%02d%%",humid/100,humid%100);

		//send dispaly data, partial refresh 8 times.

		if(refreshCount < 5)
		{	
			EPD_PartialDisplay(imageCache);
			refreshCount++;
		}
		else
		{
			EPD_Init();	
			EPD_SendDisplay(imageCache);
			refreshCount = 0;			
		}
	}

	free(imageCache);

	PFIC_EnableIRQ( GPIO_A_IRQn) ;
	MySleep(POWER_PIN);
	EPD_Sleep();
	RFIP_WakeUpRegInit();
	PFIC_DisableIRQ( GPIO_A_IRQn) ;

	for(uint16_t i = 0;i<3;i++)
	{
		gTxParam.whiteChannel=0x37; 
		gTxParam.frequency = 37;
		tx_flag = 1;
		RFIP_StartTx( &gTxParam );
		do{__nop();}while(tx_flag == 1); // 等待发送完成
		gTxParam.whiteChannel=0x38; 
		gTxParam.frequency = 38;
		tx_flag = 1;
		RFIP_StartTx( &gTxParam );
		do{__nop();}while(tx_flag == 1); // 等待发送完成
		gTxParam.whiteChannel=0x39; 
		gTxParam.frequency = 39;
		tx_flag = 1;
		RFIP_StartTx( &gTxParam );
		do{__nop();}while(tx_flag == 1); // 等待发送完成
		RTC_TRIGFunCfg(32*200);
		MySleep(POWER_PIN);	
		RFIP_WakeUpRegInit();
	}

	RTC_TRIGFunCfg(32768*60);
	MySleep(POWER_PIN);	
	RFIP_WakeUpRegInit();

	goto measure;


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

