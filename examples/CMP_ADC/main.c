/****************************************************************
 * File Name          : Main.c
 * Author             : JCQ
 * Version            : V1.0
 * Date               : 2025/04/07
 * Description        : 比较器充当4位ADC
 *********************************************************************************/
#include "CH57x_common.h"
#include "main.h"
#include "cmpadc.h"

#define LED_PIN GPIO_Pin_9

void MySleep(uint8_t use5v);


static void GPIOInit(void)
{
    GPIOA_ModeCfg(GPIO_Pin_All,GPIO_ModeIN_PU);
    GPIOA_ModeCfg(LED_PIN,GPIO_ModeOut_PP_5mA);
    GPIOA_ModeCfg(GPIO_Pin_3,GPIO_ModeIN_Floating);

}

void main(void)
{
    HSECFG_Capacitance(HSECap_18p);
    SetSysClock(CLK_SOURCE_HSE_PLL_100MHz);
    GPIOA_ModeCfg(LED_PIN, GPIO_ModeOut_PP_5mA);
    GPIOInit();
    PFIC_EnableIRQ( RTC_IRQn );
    PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_RTC_WAKE, Fsys_Delay_4096);//开启RTC唤醒使能
	sys_safe_access_enable();
	R32_RTC_TRIG = 0;
	R32_RTC_CTRL |= RB_RTC_LOAD_HI;
	R32_RTC_CTRL |= RB_RTC_LOAD_LO;
	R8_RTC_MODE_CTRL |= RB_RTC_TRIG_EN;  //enable RTC trigger
   	R8_SLP_WAKE_CTRL |= RB_SLP_RTC_WAKE; // enable wakeup control
	sys_safe_access_disable();

    for(uint8_t i=0;i<5;i++)
    {
        GPIOA_SetBits(LED_PIN);
        DelayMs(500);
        GPIOA_ResetBits(LED_PIN);
        DelayMs(50);
    }
    GPIOA_ResetBits(LED_PIN);

    cmp_adc_init();
    while(1)
    {
        if (cmp_adc_isbigger(15UL)){GPIOA_SetBits(LED_PIN);}
        else {GPIOA_ResetBits(LED_PIN);}
        RTC_TRIGFunCfg(32000);
        MySleep(0);
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

__INTERRUPT
__HIGH_CODE
void RTC_IRQHandler(void)
{
	R8_RTC_FLAG_CTRL =  RB_RTC_TRIG_CLR;
	
}
