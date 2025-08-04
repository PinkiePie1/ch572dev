/****************************************************************
 * File Name          : Main.c
 * Author             : JCQ
 * Version            : V1.0
 * Date               : 2025/04/07
 * Description        : 裸机驱动墨水屏
 *********************************************************************************/
#include "CH57x_common.h"
#include "main.h"
#include <CH572rf.h>
//#include <CH572BLEPeri_LIB.h>
#include <stdlib.h>

#define LED GPIO_Pin_9

rfipTx_t gTxParam;
uint8_t tx_flag = 0;
rfRoleConfig_t conf ={0};
__attribute__((__aligned__(4))) uint8_t TxBuf[64];

const uint8_t advert_data[]={
	0x02,
	0x00,//长度,后面再填入
	0x1A,0x2A,0x3A,0x4A,0x5A,0x6A,//MAC地址，反过来的
	0x02,0x09,'X' //完整名字	
};

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

__HIGH_CODE
void main(void)
{
	SetSysClock(CLK_SOURCE_HSE_PLL_100MHz);
	GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
	GPIOA_SetBits(LED);
	GPIOA_ModeCfg(LED,GPIO_ModeOut_PP_5mA);

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

	// 初始化发送的数据
	memcpy(TxBuf,advert_data,sizeof(advert_data));
	TxBuf[1] = (uint8_t) (sizeof(advert_data)-2);

	while(1){
		GPIOA_ResetBits(LED);
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
		GPIOA_SetBits(LED);
		RTC_TRIGFunCfg(32*200);
		LowPower_Sleep(RB_PWR_RAM12K | RB_PWR_EXTEND | RB_XT_PRE_EN );
		HSECFG_Current(HSE_RCur_100);	
		RFIP_WakeUpRegInit();
	};

}



__INTERRUPT
__HIGH_CODE
void RTC_IRQHandler(void)
{
	R8_RTC_FLAG_CTRL =  RB_RTC_TRIG_CLR;
	
}


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

