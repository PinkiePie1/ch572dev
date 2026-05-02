#include "ST7305.h"


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

void ST7305_Init(void)
{
    //peripheral init

    GPIOA_ModeCfg(RES_PIN|SDI_PIN|SCLK_PIN|DC_PIN|CS_PIN, GPIO_ModeOut_PP_5mA);

    CS_HIGH;
    SDI_HIGH;
    SCLK_HIGH;
    DC_HIGH;
    

    
    //HW reset
    RES_HIGH;
    Dev_DelayMs(50);
    RES_LOW;
    Dev_DelayMs(100);
    RES_HIGH;

    dCMD(0xD6);dDAT(0x13);dDAT(0x02);
    dCMD(0xD1);dDAT(0x01);
    dCMD(0xC0);dDAT(0x08);dDAT(0x06);
    dCMD(0xC1);dDAT(0x3C);dDAT(0x3E);dDAT(0x3C);dDAT(0x3C);
    dCMD(0xC2);dDAT(0x23);dDAT(0x21);dDAT(0x23);dDAT(0x23);
    dCMD(0xC4);dDAT(0x5A);dDAT(0x5C);dDAT(0x5A);dDAT(0x5A);
    dCMD(0xC5);dDAT(0x37);dDAT(0x35);dDAT(0x37);dDAT(0x37);

    dCMD(0xD8);dDAT(0x80);dDAT(0xE9);
    dCMD(0xB2);dDAT(0x12);

    dCMD(0xB3);dDAT(0xE5);dDAT(0xF6);dDAT(0x17);dDAT(0x77);dDAT(0x77);dDAT(0x77);dDAT(0x77);dDAT(0x77);dDAT(0x77);dDAT(0x71);
    dCMD(0xB4);dDAT(0x05);dDAT(0x46);dDAT(0x77);dDAT(0x77);dDAT(0x77);dDAT(0x77);dDAT(0x76);dDAT(0x45);
    dCMD(0x62);dDAT(0x32);dDAT(0x03);dDAT(0x1F);
    
    dCMD(0xB7);dDAT(0x13);
    dCMD(0xB0);dDAT(0x60);
    dCMD(0x11);
    Dev_DelayMs(120);

    dCMD(0xC9);dDAT(0x00);
    dCMD(0x36);dDAT(0x00);
    dCMD(0x3A);dDAT(0x11);
    dCMD(0xB9);dDAT(0x20);
    dCMD(0xB8);dDAT(0x29);
    dCMD(0x2A);dDAT(0x17);dDAT(0x24);
    dCMD(0x2B);dDAT(0x00);dDAT(0xBF);
    //dCMD(0x35);dDAT(0x00);
    dCMD(0xD0);dDAT(0xFF);
    dCMD(0x39);
    dCMD(0x29);
    dCMD(0x20);
    //dCMD(0xBB);dDAT(0x4F);
    Dev_DelayMs(100);

}

void ST7305_RAM(uint8_t *display)
{
    dCMD(0x2A);dDAT(0x17);dDAT(0x24);
    dCMD(0x2B);dDAT(0x00);dDAT(0xBF);
    dCMD(0x2C);
    uint8_t buf[8064] = {0};
    for(uint16_t i=0;i<5000;i++){buf[i]=0xAA;}
    dMulDAT(buf,8064);

}