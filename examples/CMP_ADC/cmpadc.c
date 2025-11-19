#include "cmpadc.h"

/*初始化*/
void cmp_adc_init(void)
{
    R32_CMP_CTRL = 0;
}


/*开启CMP，读取是否更大，然后关闭CMP*/
/*输入：0-15对应0V到0.8V*/
/*输出：引脚电压是否大于对应的值*/
uint8_t cmp_adc_isbigger(uint8_t target)
{
    uint8_t result = 0;
    R32_CMP_CTRL = (0x01 << 2) | (target << 4) | RB_CMP_EN; //PA7输入,开启比较
    //DelayUs(5000);
    if ( R32_CMP_CTRL & (0x01 << 25) ){ result = 1; }//如果输出为1,则说明大于设定
    R32_CMP_CTRL = 0;
    return result; 
}