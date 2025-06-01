/********************************** (C) COPYRIGHT *******************************
 * File Name          : MCU.c
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2022/01/18
 * Description        : Ӳ��������������BLE��Ӳ����ʼ��
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* ͷ�ļ����� */
#include "HAL.h"
#include "peripheral.h"
#include "AHT20.h"

tmosTaskID halTaskID;
uint32_t g_LLE_IRQLibHandlerLocation;

uint32_t temperature = 0;
uint32_t humid = 0;
uint8_t rawData[6] = {0};

#if(defined(BLE_SNV)) && (BLE_SNV == TRUE)
/*******************************************************************************
 * @fn      Lib_Read_Flash
 *
 * @brief   Callback function used for BLE lib.
 *
 * @param   addr - Read start address
 * @param   num - Number of units to read (unit: 4 bytes)
 * @param   pBuf - Buffer to store read data
 *
 * @return  None.
 */
uint32_t Lib_Read_Flash(uint32_t addr, uint32_t num, uint32_t *pBuf)
{
    FLASH_ROM_READ(addr, pBuf, num * 4);
    return 0;
}

/*******************************************************************************
 * @fn      Lib_Write_Flash
 *
 * @brief   Callback function used for BLE lib.
 *
 * @param   addr - Write start address
 * @param   num - Number of units to write (unit: 4 bytes)
 * @param   pBuf - Buffer with data to be written
 *
 * @return  None.
 */
uint32_t Lib_Write_Flash(uint32_t addr, uint32_t num, uint32_t *pBuf)
{
    FLASH_ROM_ERASE(addr, num * 4);
    FLASH_ROM_WRITE(addr, pBuf, num * 4);
    return 0;
}
#endif

/*******************************************************************************
 * @fn      CH57x_BLEInit
 *
 * @brief   BLE ���ʼ��
 *
 * @param   None.
 *
 * @return  None.
 */
void CH57x_BLEInit(void)
{
    uint8_t     i;
    bleConfig_t cfg;
    if(tmos_memcmp(VER_LIB, VER_FILE, strlen(VER_FILE)) == FALSE)
    {
        PRINT("head file error...\n");
        while(1);
    }

    __SysTick_Config(SysTick_LOAD_RELOAD_Msk);// ����SysTick

    sys_safe_access_enable( );
    R32_MISC_CTRL = (R32_MISC_CTRL&(~(0x3f<<24)))|(0xe<<24);
    sys_safe_access_disable( );
    g_LLE_IRQLibHandlerLocation = (uint32_t)LLE_IRQLibHandler;
    PFIC_SetPriority(BLEL_IRQn, 0xF0);
    tmos_memset(&cfg, 0, sizeof(bleConfig_t));
    cfg.MEMAddr = (uint32_t)MEM_BUF;
    cfg.MEMLen = (uint32_t)BLE_MEMHEAP_SIZE;
    cfg.BufMaxLen = (uint32_t)BLE_BUFF_MAX_LEN;
    cfg.BufNumber = (uint32_t)BLE_BUFF_NUM;
    cfg.TxNumEvent = (uint32_t)BLE_TX_NUM_EVENT;
    cfg.TxPower = (uint32_t)BLE_TX_POWER;
    cfg.WindowWidening = 120;
#if(defined(BLE_SNV)) && (BLE_SNV == TRUE)
    if((BLE_SNV_ADDR + BLE_SNV_BLOCK * BLE_SNV_NUM) > (0x40000))
    {
        PRINT("SNV config error...\n");
        while(1);
    }
    cfg.SNVAddr = (uint32_t)BLE_SNV_ADDR;
    cfg.SNVBlock = (uint32_t)BLE_SNV_BLOCK;
    cfg.SNVNum = (uint32_t)BLE_SNV_NUM;
    cfg.readFlashCB = Lib_Read_Flash;
    cfg.writeFlashCB = Lib_Write_Flash;
#endif
    cfg.srandCB = SYS_GetSysTickCnt;
#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    cfg.idleCB = CH57x_LowPower; // ����˯��
#endif
#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
    for(i = 0; i < 6; i++)
    {
        cfg.MacAddr[i] = MacAddr[5 - i];
    }
#else
    {
        uint8_t MacAddr[6];
        GetMACAddress(MacAddr);
        for(i = 0; i < 6; i++)
        {
            cfg.MacAddr[i] = MacAddr[i]; // ʹ��оƬmac��ַ
        }
    }
#endif
    if(!cfg.MEMAddr || cfg.MEMLen < 3 * 1024)
    {
        while(1);
    }
    // BLE_Lib ռ����VTF Interrupt 2�ź�3��
    i = BLE_LibInit(&cfg);
    if(i)
    {
        PRINT("LIB init error code: %x ...\n", i);
        while(1);
    }
}

/*******************************************************************************
 * @fn      HAL_ProcessEvent
 *
 * @brief   Ӳ����������
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events  - events to process.  This is a bit map and can
 *                      contain more than one event.
 *
 * @return  events.
 */
tmosEvents HAL_ProcessEvent(tmosTaskID task_id, tmosEvents events)
{
    uint8_t *msgPtr;

    if(events & SYS_EVENT_MSG)
    { // ����HAL����Ϣ������tmos_msg_receive��ȡ��Ϣ��������ɺ�ɾ����Ϣ��
        msgPtr = tmos_msg_receive(task_id);
        if(msgPtr)
        {
            /* De-allocate */
            tmos_msg_deallocate(msgPtr);
        }
        return events ^ SYS_EVENT_MSG;
    }
    if(events & LED_BLINK_EVENT)
    {

        return events ^ LED_BLINK_EVENT;
    }
    if(events & HAL_KEY_EVENT)
    {
//      tmos_start_task(halTaskID, HAL_KEY_EVENT, MS1_TO_SYSTEM_TIME(100));
        return events ^ HAL_KEY_EVENT;

    }
    if(events & HAL_TEMP_STARTMEASURE)
    {
		AHT20_beginMeasure();// send measure beign command
		tmos_start_task(halTaskID,HAL_TEMP_GET,MS1_TO_SYSTEM_TIME(80));
		tmos_start_task(halTaskID, HAL_TEMP_STARTMEASURE, MS1_TO_SYSTEM_TIME(10000));
        return events ^ HAL_TEMP_STARTMEASURE;

    }
        if(events & HAL_TEMP_GET)
    {
		AHT20_getDat(rawData);
		
		humid = rawData[1];
		humid <<= 8; 
		humid |= rawData[2];
		humid <<= 8; 
		humid |= (rawData[3] & 0xF0);
		humid >>= 4;
		humid = (humid >> 7)+
		(humid >> 10)+
		(humid >> 11)+
		(humid >> 12)+
		(humid >> 16);

		temperature = 0;
		temperature = rawData[3] & 0x0F;
		temperature <<= 8;
		temperature |= rawData[4];
		temperature <<= 8;
		temperature |= rawData[5];
		temperature = (temperature >> 6) + 
					(temperature >> 9) +
					(temperature >> 10) +
					(temperature >> 11) +
					(temperature >> 15) ;
		; // what we need is tem*20000/2^20-5000,so bit 6 9 10 11 15
		                                
		temperature = temperature - 5000;
		
		tmos_set_event(Peripheral_TaskID,ADV_DATA_UPDATE_EVT);
    	return events ^ HAL_TEMP_GET;
    }
    if(events & HAL_REG_INIT_EVENT)
    {
#if(defined BLE_CALIBRATION_ENABLE) && (BLE_CALIBRATION_ENABLE == TRUE) // У׼���񣬵���У׼��ʱС��10ms
        BLE_RegInit();                                                  // У׼RF����ر�RF���ı�RF��ؼĴ��������ʹ����RF�շ�������ע��У׼������������
        tmos_start_task(halTaskID, HAL_REG_INIT_EVENT, MS1_TO_SYSTEM_TIME(BLE_CALIBRATION_PERIOD));
        return events ^ HAL_REG_INIT_EVENT;
#endif
    }
    if(events & HAL_TEST_EVENT)
    {
        return events ^ HAL_TEST_EVENT;
    }
    return 0;
}

/*******************************************************************************
 * @fn      HAL_Init
 *
 * @brief   Ӳ����ʼ��
 *
 * @param   None.
 *
 * @return  None.
 */
void HAL_Init()
{
    halTaskID = TMOS_ProcessEventRegister(HAL_ProcessEvent);
    HAL_TimeInit();
#if(defined HAL_SLEEP) && (HAL_SLEEP == TRUE)
    HAL_SleepInit();
#endif
#if(defined BLE_CALIBRATION_ENABLE) && (BLE_CALIBRATION_ENABLE == TRUE)
    tmos_start_task(halTaskID, HAL_REG_INIT_EVENT, 800); // ����У׼����500ms����������У׼��ʱС��10ms
#endif
//    tmos_start_task( halTaskID, HAL_TEST_EVENT, 1600 );    // ����һ����������
	//SET VCC of AHT on here!
	SoftI2CInit();
	tmos_start_task(halTaskID, HAL_TEMP_STARTMEASURE, MS1_TO_SYSTEM_TIME(300));
	
}

/******************************** endfile @ mcu ******************************/
