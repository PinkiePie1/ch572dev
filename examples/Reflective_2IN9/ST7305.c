#include "ST7305.h"

#if ST7305_SPI_MODE

void dCMD(uint8_t dat)
{  
    DC_LOW;
    CS_LOW;
    SPI_MasterSendByte(dat);
    CS_HIGH;

}

void dDAT(uint8_t dat)
{
    DC_HIGH;
    CS_LOW;
    SPI_MasterSendByte(dat);
    CS_HIGH;
}

void dMulDAT(uint8_t * buf, uint16_t length)
{
    DC_HIGH;
    CS_LOW;
    SPI_MasterDMATrans(buf,length);
    CS_HIGH;
}


#else

void dCMD(uint8_t dat)
{
    DC_LOW;
    CS_LOW;
    SCLK_LOW;
    for(uint8_t i = 0; i < 8;i++)
    {
        if( dat & 0x80 ) {SDI_HIGH;} else {SDI_LOW;}
        SCLK_HIGH;
        SCLK_LOW;
        dat <<= 1;
    }
    CS_HIGH;
    SDI_HIGH;
    SCLK_HIGH;

}

void dDAT(uint8_t dat)
{
    DC_HIGH;
    CS_LOW;
    SCLK_LOW;
    for(uint8_t i = 0; i < 8;i++)
    {
        if( dat & 0x80 ) {SDI_HIGH;} else {SDI_LOW;}
        SCLK_HIGH;
        SCLK_LOW;
        dat <<= 1;
    }
    CS_HIGH;
    SDI_HIGH;
    SCLK_HIGH;

}

void dMulDAT(uint8_t * buf, uint16_t length)
{
    DC_HIGH;
    CS_LOW;
    SCLK_LOW;
    for(uint16_t j = 0; j < length; j++){
        uint8_t dat = buf[j];
        for(uint8_t i = 0; i < 8;i++)
        {
            if( dat & 0x80 ) {SDI_HIGH;} else {SDI_LOW;}
            SCLK_HIGH;
            SCLK_LOW;
            dat <<= 1;
        }
    }
    CS_HIGH;
    SDI_HIGH;
    SCLK_HIGH;
}

#endif

void ST7305_Init(void)
{
    //peripheral init
#if ST7305_SPI_MODE

    GPIOA_ModeCfg(RES_PIN|DC_PIN|CS_PIN, GPIO_ModeOut_PP_5mA);
    GPIOA_ModeCfg(TE_PIN,GPIO_ModeIN_PU);
    GPIOA_ModeCfg(SCLK_PIN|SDI_PIN, GPIO_ModeOut_PP_20mA);
    SPI_MasterDefInit();
    SPI_CLKCfg(2);



#else
    GPIOA_ModeCfg(RES_PIN|SDI_PIN|SCLK_PIN|DC_PIN|CS_PIN, GPIO_ModeOut_PP_5mA);
    GPIOA_ModeCfg(TE_PIN,GPIO_ModeIN_PU);

    CS_HIGH;
    SDI_HIGH;
    SCLK_HIGH;
    DC_HIGH;
#endif

    
    //HW reset
    RES_HIGH;
    Dev_DelayMs(50);
    RES_LOW;
    Dev_DelayMs(100);
    RES_HIGH;

    dCMD(0xD6);dDAT(0x13);dDAT(0x02);
    dCMD(0xD1);dDAT(0x01);//enable boost
    dCMD(0xC0);dDAT(0x08);dDAT(0x06);//gate voltage vgh and vgl
    dCMD(0xC1);dDAT(0x3C);dDAT(0x3E);dDAT(0x3C);dDAT(0x3C);//VSHP volage
    dCMD(0xC2);dDAT(0x23);dDAT(0x21);dDAT(0x23);dDAT(0x23);//VSLP voltage
    dCMD(0xC4);dDAT(0x5A);dDAT(0x5C);dDAT(0x5A);dDAT(0x5A);//VSHN voltage
    dCMD(0xC5);dDAT(0x37);dDAT(0x35);dDAT(0x37);dDAT(0x37);//VSLN voltage

    dCMD(0xD8);dDAT(0xA6);dDAT(0xE9);//80 for OSC at 51Hz, A6 for 31Hz
    dCMD(0xB2);dDAT(0x02);//frame rate 0.25Hz

    dCMD(0xB3);dDAT(0xE5);dDAT(0xF6);dDAT(0x17);dDAT(0x77);dDAT(0x77);dDAT(0x77);dDAT(0x77);dDAT(0x77);dDAT(0x77);dDAT(0x71);
    dCMD(0xB4);dDAT(0x05);dDAT(0x46);dDAT(0x77);dDAT(0x77);dDAT(0x77);dDAT(0x77);dDAT(0x76);dDAT(0x45);
    dCMD(0x62);dDAT(0x32);dDAT(0x03);dDAT(0x1F);
    
    dCMD(0xB7);dDAT(0x13);
    dCMD(0xB0);dDAT(0x60);
    dCMD(0x11);
    Dev_DelayMs(10);

    dCMD(0xC9);dDAT(0x00);
    dCMD(0x36);dDAT(0x48);
    dCMD(0x3A);dDAT(0x11);
    dCMD(0xB9);dDAT(0x20);
    dCMD(0xB8);dDAT(0x29);
    dCMD(0x2A);dDAT(0x17);dDAT(0x24);
    dCMD(0x2B);dDAT(0x00);dDAT(0xBF);
    dCMD(0x35);dDAT(0x00);
    dCMD(0xD0);dDAT(0xFF);//auto power down
    dCMD(0x39);
    dCMD(0x29);
    dCMD(0x20);
    //dCMD(0xBB);dDAT(0x4F);
    Dev_DelayMs(100);

}

void ST7305_RAM(uint8_t *display)
{

    while(TE_ISLOW);
#if ST7305_SPI_MODE == 0
    while(TE_ISHIGH); 
    Dev_DelayUs(400);
                      //if using soft spi, which is slower than the LCD, the data 
                      //transfer should happen after the panel starts reading data from ram
                      //so that tearing does not catch up.
                      //while if using hardware SPI, the data transfer should happen before
                      //the panel starts reading data. so the update never catches up with the data writes.
#endif

    GPIOA_ResetBits(GPIO_Pin_9);
    dCMD(0x2A);dDAT(0x17);dDAT(0x24);
    dCMD(0x2B);dDAT(0x00);dDAT(0xBF);
    dCMD(0x2C);
    
    dMulDAT(display,4000);
    dMulDAT(display+4000,4064);
    GPIOA_SetBits(GPIO_Pin_9);
}