/********************************** (C) COPYRIGHT *******************************
 * File Name          : peripheral.C
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        : ����ӻ�������Ӧ�ó��򣬳�ʼ���㲥���Ӳ�����Ȼ��㲥������������
 *                      ����������Ӳ�����ͨ���Զ������������
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include "peripheral.h"
#include "EPD_1IN54_SSD1680.h"
#include "miniGUI.h"
#include "SHT40.h"
#include "imageData.h"
#include <stdlib.h>

uint8_t rawData[9];
uint32_t humid;
uint32_t temperature;
uint8_t *imageCache;
uint8_t refreshCount = 250;
uint8_t img_index = 0;

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// How often to perform periodic event
#define SBP_PERIODIC_EVT_PERIOD              1600

// How often to perform read rssi event
#define SBP_READ_RSSI_EVT_PERIOD             3200

// Parameter update delay
#define SBP_PARAM_UPDATE_DELAY               6400

// PHY update delay
#define SBP_PHY_UPDATE_DELAY                 2400

// What is the advertising interval when device is discoverable (units of 625us, 80=50ms)
#define DEFAULT_ADVERTISING_INTERVAL         1300

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely
#define DEFAULT_DISCOVERABLE_MODE            GAP_ADTYPE_FLAGS_GENERAL

// Minimum connection interval (units of 1.25ms, 6=7.5ms)
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL    6

// Maximum connection interval (units of 1.25ms, 100=125ms)
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL    100

// Slave latency to use parameter update
#define DEFAULT_DESIRED_SLAVE_LATENCY        0

// Supervision timeout value (units of 10ms, 100=1s)
#define DEFAULT_DESIRED_CONN_TIMEOUT         100

// Company Identifier: WCH
#define WCH_COMPANY_ID                       0x07D7

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8_t Peripheral_TaskID = INVALID_TASK_ID; // Task ID for internal task/event processing


// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertising)
static uint8_t advertData[] = {
    // Flags; this sets the device to use limited discoverable
    // mode (advertises for 30 seconds at a time) instead of general
    // discoverable mode (advertises indefinitely)
    0x02, // length of this data
    GAP_ADTYPE_FLAGS,
    GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

    0x0c,                                 // length of this data
    GAP_ADTYPE_LOCAL_NAME_COMPLETE, 
    't',                                  // 'B'
    'e',                                 // 'r'
    'm',                                 // 'o'
    'p',                                 // 'a'
    '_',                                 // 'd'
    's',                                 // 'c'
    'e',                                 // 'a'
    'n',                                 // 's'
    's',                                 // 't'
    'o',                                 // 'e'
    'r',                                 // 'r'

    // Broadcast of the data
    0x07,                             // length of this data including the data type byte
    GAP_ADTYPE_MANUFACTURER_SPECIFIC, // manufacturer specific advertisement data type
    0xFF, 0xFF, 
    0x00,0x00, //20212223
    0x00,0x00
};


static uint16_t peripheralMTU = ATT_MTU_SIZE;
/*********************************************************************
 * LOCAL FUNCTIONS
 */

// Simple GATT Profile Callbacks
/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      Peripheral_Init
 *
 * @brief   Initialization function for the Peripheral App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by TMOS.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void Peripheral_Init()
{
    Peripheral_TaskID = TMOS_ProcessEventRegister(Peripheral_ProcessEvent);

    // Setup the GAP Peripheral Role Profile
    {
        uint8_t  initial_advertising_enable = TRUE;
        uint8_t  initial_adv_event_type = GAP_ADTYPE_ADV_NONCONN_IND;
 

        // Set the GAP Role Parameters
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);
        GAPRole_SetParameter(GAPROLE_ADV_EVENT_TYPE, sizeof(uint8_t), &initial_adv_event_type);
        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);

    }

    {
        uint16_t advInt = DEFAULT_ADVERTISING_INTERVAL;

        // Set advertising interval
        GAP_SetParamValue(TGAP_DISC_ADV_INT_MIN, advInt);
        GAP_SetParamValue(TGAP_DISC_ADV_INT_MAX, advInt);

        // Enable scan req notify
        GAP_SetParamValue(TGAP_ADV_SCAN_REQ_NOTIFY, DISABLE);
    }


	tmos_set_event(Peripheral_TaskID, SBP_START_DEVICE_EVT);

    EPD_Hal_Init();
    EPD_Init();	
    EPD_Sleep();
    SoftI2CInit();
    tmos_start_task(Peripheral_TaskID, AHT_GETDAT_EVT,10);
	

}

/*********************************************************************
 * @fn      peripheralInitConnItem
 *
 * @brief   Init Connection Item
 *
 * @param   peripheralConnList -
 *
 * @return  NULL
 */
static void peripheralInitConnItem(peripheralConnItem_t *peripheralConnList)
{
    peripheralConnList->connHandle = GAP_CONNHANDLE_INIT;
    peripheralConnList->connInterval = 0;
    peripheralConnList->connSlaveLatency = 0;
    peripheralConnList->connTimeout = 0;
}

/*********************************************************************
 * @fn      Peripheral_ProcessEvent
 *
 * @brief   Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16_t Peripheral_ProcessEvent(uint8_t task_id, uint16_t events)
{
    //  VOID task_id; // TMOS required parameter that isn't used in this function

    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;

        if((pMsg = tmos_msg_receive(Peripheral_TaskID)) != NULL)
        {
            tmos_msg_deallocate(pMsg);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if(events & SBP_START_DEVICE_EVT)
    {
        // Start the Device
        GAPRole_PeripheralStartDevice(Peripheral_TaskID,NULL, NULL);
        return (events ^ SBP_START_DEVICE_EVT);
        
    }

    if(events & AHT_GETDAT_EVT)
    {
    	SHT40_beginMeasure();
    	DelayMs(2);
		SHT40_getDat(rawData);
		humid = (rawData[3] << 8);
		humid |= rawData[4];
		humid = (humid >> 3) + (humid >> 4) + (humid >> 9) + (humid >> 10);

		temperature = (rawData[0] << 8);
		temperature |= rawData[1];
		temperature = (temperature >> 2) + (temperature >> 6) + (temperature >> 10);
										
		temperature = temperature - 4500;
		humid = humid-600;

		//update advertisement
		advertData[20]=temperature/100;
		advertData[21]=temperature%100;
		advertData[22]=humid/100;
		advertData[23]=humid%100;
		GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);

		//print to screen
		imageCache = malloc(2888);
		uint8_t textcolor = BLACK;
		img_index = (0x0001&temperature);
		if(imageCache != NULL)
		{
			if (img_index == 0)
			{
				memcpy(imageCache,gImage_dither,2888);
				textcolor = WHITE;
			}
			else if (img_index == 1)
			{
				memcpy(imageCache,gImage_dither2,2888);
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
			tmos_start_task(Peripheral_TaskID,EPD_WAITBUSY_EVT,1400);

		}			
    	return (events ^ AHT_GETDAT_EVT);
    }

    if(events & EPD_WAITBUSY_EVT)
    {
    	if(IS_BUSY == 0)
    	{   		
    		
    		tmos_start_task(Peripheral_TaskID, AHT_GETDAT_EVT,192000);
    		EPD_Sleep();
       	} 
    	else 
    	{
    		tmos_start_task(Peripheral_TaskID, EPD_WAITBUSY_EVT,10);
    	}

    	return (events ^ EPD_WAITBUSY_EVT);
    }

    // Discard unknown events
    return 0;
}
