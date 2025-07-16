/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2020/08/06
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "CONFIG.h"
#include "RF_PHY.h"
#include "EPD_1IN54_SSD1680.h"
#include "miniGUI.h"
#include "SHT40.h"
#include "imageData.h"
#include <stdlib.h>

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

#define RF_AUTO_MODE_EXAM       1

uint8_t taskID;
uint8_t AHT_init_flag = 0;

uint8_t rawData[9];
uint32_t humid;
uint32_t temperature;
uint8_t *imageCache;
uint8_t refreshCount = 250;
uint8_t img_index = 0;
uint8_t txchannel = 37;

rfConfig_t rf_Config;

uint8_t ble_adv_test_data[] = {
                                0x3c,0x10,0x2D, 0xE4, 0xC2, 0x84,      //MAC ADDR
                                9,0x09,'T','E','P','-','T','E','S','T', //ADV data
                                7,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x01//20-23
                              };

volatile uint8_t tx_end_flag=0;
volatile uint8_t rx_end_flag=0;

/*********************************************************************
 * @fn      RF_Wait_Tx_End
 *
 * @brief   手动模式等待发送完成，自动模式等待发送-接收完成，必须在RAM中等待，等待时可以执行用户代码，但需要注意执行的代码必须运行在RAM中，否则影响发送
 *
 * @return  none
 */
__HIGH_CODE
__attribute__((noinline))
void RF_Wait_Tx_End()
{
    uint32_t i=0;
    while(!tx_end_flag)
    {
        i++;
        __nop();
        __nop();
        // 约5ms超时
        if(i>(FREQ_SYS/1000))
        {
            tx_end_flag = TRUE;
        }
    }
}

/*********************************************************************
 * @fn      RF_Wait_Rx_End
 *
 * @brief   自动模式等待应答发送完成，必须在RAM中等待，等待时可以执行用户代码，但需要注意执行的代码必须运行在RAM中，否则影响发送
 *
 * @return  none
 */
__HIGH_CODE
__attribute__((noinline))
void RF_Wait_Rx_End()
{
    uint32_t i=0;
    while(!rx_end_flag)
    {
        i++;
        __nop();
        __nop();
        // 约5ms超时
        if(i>(FREQ_SYS/1000))
        {
            rx_end_flag = TRUE;
        }
    }
}

/*********************************************************************
 * @fn      RF_2G4StatusCallBack
 *
 * @brief   RF 状态回调，此函数在中断中调用。注意：不可在此函数中直接调用RF接收或者发送API，需要使用事件的方式调用
 *          在此回调中直接使用或调用函数涉及到的变量需注意，此函数在中断中调用。
 *
 * @param   sta     - 状态类型
 * @param   crc     - crc校验结果
 * @param   rxBuf   - 数据buf指针
 *
 * @return  none
 */
void RF_2G4StatusCallBack(uint8_t sta, uint8_t crc, uint8_t *rxBuf)
{
    switch(sta)
    {
        case TX_MODE_TX_FINISH:
        {
#if(!RF_AUTO_MODE_EXAM)
            tx_end_flag = TRUE;
#endif
            break;
        }
        case TX_MODE_TX_FAIL:
        {
            tx_end_flag = TRUE;
            break;
        }
        case TX_MODE_RX_DATA:
        {
#if(RF_AUTO_MODE_EXAM)
            tx_end_flag = TRUE;
            if (crc == 0) {
                uint8_t i;

                PRINT("tx recv,rssi:%d\n", (int8_t)rxBuf[0]);
                PRINT("len:%d-", rxBuf[1]);

                for (i = 0; i < rxBuf[1]; i++) {
                    PRINT("%x ", rxBuf[i + 2]);
                }
                PRINT("\n");
            } else {
                if (crc & (1<<0)) {
                    PRINT("crc error\n");
                }

                if (crc & (1<<1)) {
                    PRINT("match type error\n");
                }
            }
#endif
            break;
        }
        case TX_MODE_RX_TIMEOUT: // Timeout is about 200us
        {
#if(RF_AUTO_MODE_EXAM)
            tx_end_flag = TRUE;
#endif
            break;
        }
        case RX_MODE_RX_DATA:
        {
            if (crc == 0) {
                uint8_t i;
#if(RF_AUTO_MODE_EXAM)
                RF_Wait_Rx_End();
#endif
                PRINT("rx recv, rssi: %d\n", (int8_t)rxBuf[0]);
                PRINT("len:%d-", rxBuf[1]);
                
                for (i = 0; i < rxBuf[1]; i++) {
                    PRINT("%x ", rxBuf[i + 2]);
                }
                PRINT("\n");
            } else {
                if (crc & (1<<0)) {
                    PRINT("crc error\n");
                }

                if (crc & (1<<1)) {
                    PRINT("match type error\n");
                }
            }
#if(!RF_AUTO_MODE_EXAM)
            tmos_set_event(taskID, SBP_RF_RF_RX_EVT);
#endif
            break;
        }
        case RX_MODE_TX_FINISH:
        {
#if(RF_AUTO_MODE_EXAM)
            rx_end_flag = TRUE;
            tmos_set_event(taskID, SBP_RF_RF_RX_EVT);
#endif
            break;
        }
        case RX_MODE_TX_FAIL:
        {
#if(RF_AUTO_MODE_EXAM)
            rx_end_flag = TRUE;
            tmos_set_event(taskID, SBP_RF_RF_RX_EVT);
#endif
            break;
        }
    }
}

/*********************************************************************
 * @fn      RF_ProcessEvent
 *
 * @brief   RF 事件处理
 *
 * @param   task_id - 任务ID
 * @param   events  - 事件标志
 *
 * @return  未完成事件
 */
uint16_t RF_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;

        if((pMsg = tmos_msg_receive(task_id)) != NULL)
        {
            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }
    if(events & SBP_RF_START_DEVICE_EVT)
    {   

        tmos_start_task(taskID, HAL_TEMP_GET, 20);
        
        
        return events ^ SBP_RF_START_DEVICE_EVT;
    }
    if(events & SBP_RF_PERIODIC_EVT)
    {
        if(txchannel == 39)
        {txchannel = 37;}
        else
        {txchannel++;}
        RF_Shut();
        rf_Config.Channel = txchannel;
        RF_Config(&rf_Config);
//      tx_end_flag = FALSE;
        RF_Tx(ble_adv_test_data, sizeof(ble_adv_test_data), 0x02, 0xFF);
/*
        RF_Shut();
        rf_Config.Channel = 38;
        RF_Config(&rf_Config);
        tx_end_flag = FALSE;
        if(!RF_Tx(ble_adv_test_data, sizeof(ble_adv_test_data), 0x02, 0xFF))
        {
            RF_Wait_Tx_End();
        }
        RF_Shut();
        rf_Config.Channel = 39;
        RF_Config(&rf_Config);
        tx_end_flag = FALSE;
        if(!RF_Tx(ble_adv_test_data, sizeof(ble_adv_test_data), 0x02, 0xFF))
        {
            RF_Wait_Tx_End();
        }
*/
        tmos_start_task(taskID, SBP_RF_PERIODIC_EVT, 1300);

        return events ^ SBP_RF_PERIODIC_EVT;
    }

    
    if(events & SBP_RF_RF_RX_EVT)
    {
        uint8_t state;
        RF_Shut();
        return events ^ SBP_RF_RF_RX_EVT;
    }

    if(events & HAL_TEMP_GET)
    {
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

        ble_adv_test_data[20] = ( (humid/1000) << 4) | ( (humid%1000) / 100 );
    	ble_adv_test_data[21] = ( ((humid%100)/10) << 4 ) | humid%10;
    	ble_adv_test_data[22] = ( (temperature/1000) << 4 ) | ((temperature%1000) / 100);
    	ble_adv_test_data[23] = ( ((temperature%100)/10) << 4 ) | temperature %10;

        uint8_t error = 0 ;
 		imageCache = malloc(5000);
        if(imageCache==NULL){
            imageCache = malloc(2888);
            error=1;
        }
		uint8_t textcolor = BLACK;
		img_index = (0x0001&temperature);
		if(imageCache != NULL)
		{
			if (img_index == 0)
			{
				//memcpy(imageCache,gImage_dither,2888);
                memset(imageCache,0xFF,5000);
				textcolor = WHITE;
			}
			else if (img_index == 1)
			{
				//memcpy(imageCache,gImage_dither2,2888);
                memset(imageCache,0x00,5000);
				textcolor = BLACK;
			}
			else
			{
				img_index = 0;
			}
		
		
			paint_SetImageCache(imageCache);
            

			EPD_Printf(0,150,font14,textcolor,
				"T:%02d.%02d H:%02d.%02d%%",
				temperature/100,temperature%100,
				humid/100,humid%100);
            EPD_Printf(20,150,font14,textcolor,"er:%d",error);
            //memset(imageCache,textcolor,5);

			//send dispaly data, partial refresh 8 times.
            
			if(refreshCount < 8)
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
            
			free(imageCache);  

        }
        tmos_start_task(taskID, SBP_RF_PERIODIC_EVT, 300);
        tmos_start_task(taskID, EPD_WAITBUSY, 1400);
		
    	return events ^ HAL_TEMP_GET;
    }

    if(events & EPD_WAITBUSY)
    {   
    	if(IS_BUSY == 0)
    	{   		
    		
    		tmos_start_task(taskID, HAL_TEMP_GET,192000);
    		EPD_Sleep();
       	} 
    	else 
    	{
    		tmos_start_task(taskID, EPD_WAITBUSY,10);
    	}

    	return (events ^ EPD_WAITBUSY);
    }

    return 0;
}

/*********************************************************************
 * @fn      RF_Init
 *
 * @brief   RF 初始化
 *
 * @return  none
 */
void RF_Init(void)
{
    uint8_t    state;
    

    tmos_memset(&rf_Config, 0, sizeof(rfConfig_t));
    taskID = TMOS_ProcessEventRegister(RF_ProcessEvent);
    rf_Config.accessAddress = 0x8E89BED6; // 禁止使用0x55555555以及0xAAAAAAAA ( 建议不超过24次位反转，且不超过连续的6个0或1 )
    rf_Config.CRCInit = 0x555555;
    rf_Config.Channel = 37;
    rf_Config.Frequency = 2480000;
#if(RF_AUTO_MODE_EXAM)
    rf_Config.LLEMode = LLE_MODE_BASIC;
#else
    rf_Config.LLEMode = LLE_MODE_BASIC | LLE_MODE_EX_CHANNEL; // 使能 LLE_MODE_EX_CHANNEL 表示 选择 rf_Config.Frequency 作为通信频点
#endif
    rf_Config.rfStatusCB = RF_2G4StatusCallBack;
    rf_Config.RxMaxlen = 251;
    state = RF_Config(&rf_Config);
    PRINT("rf 2.4g init: %x\n", state);

    EPD_Hal_Init();
    EPD_Init();	

    EPD_Sleep();
    SoftI2CInit();

    tmos_set_event( taskID , SBP_RF_START_DEVICE_EVT );

}

/******************************** endfile @ main ******************************/
