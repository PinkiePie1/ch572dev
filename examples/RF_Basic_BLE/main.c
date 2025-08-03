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

#define   AA           0X8e89bed6;
#define   AA_EX        0
#define   CRC_INIT     0X555555
#define   CRC_POLY     0x80032d

//sleep.
void MySleep(uint8_t use5v);

rfipTx_t gTxParam;
__attribute__((__aligned__(4))) uint8_t TxBuf[64];

__HIGH_CODE
void RF_ProcessCallBack( rfRole_States_t sta,uint8_t id  )
{

	if( sta&RF_STATE_TX_FINISH )
    {
		PRINT("TX finished.");
    }
    if( sta&RF_STATE_TIMEOUT )
    {
        PRINT("tx error");   
    }
}

void main(void)
{
	SetSysClock(CLK_SOURCE_HSE_PLL_100MHz);

	//没人知道为什么需要这些
    sys_safe_access_enable( );
    R32_MISC_CTRL = (R32_MISC_CTRL&(~(0x3f<<24)))|(0xe<<24);
    sys_safe_access_disable( );

	//基本设置
	rfRoleConfig_t conf ={0};
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

	// 初始化发送的数据，后面改成memcpy形式
	//广播类型
	TxBuf[0] = 0x02;     
	//数据长度（MAC地址长度+数据长度）
	TxBuf[1] =0x09;              
	//mac地址
	TxBuf[2] = 0x84;
	TxBuf[3] = 0xc2;
	TxBuf[4] = 0xe4;
	TxBuf[5] = 0x03;
	TxBuf[6] = 0x02;
	TxBuf[7] = 0x22;
	//名字
	TxBuf[8] = 0x02;
	TxBuf[9] = 0x09;
	TxBuf[10] = 0x55;

	while(1)
	{
		RFIP_StartTx( &gTxParam );
		DelayMs(200);
	}
}


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
    ldoconfig |= RB_PWR_PLAN_EN | RB_PWR_CORE | RB_PWR_RAM12K |(1<<12) ;
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
