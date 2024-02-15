/*
 * nyh_mLoop.c
 *
 *  Created on: 2020. 10. 22.
 *      Author: noyou
 */
#include <stdint.h>

#include "nyh_mLoop.h"
#include "main.h"
#include "usbd_cdc_if.h"

extern USBD_HandleTypeDef 	hUsbDeviceFS;
extern PCD_HandleTypeDef	hpcd_USB_FS;
extern uint8_t 				UserRxBufferFS[APP_RX_DATA_SIZE];
extern uint8_t 				UserTxBufferFS[APP_RX_DATA_SIZE];

//1=USBD_STATE_DEFAULT, 2=USBD_STATE_ADDRESSED, 3=USBD_STATE_CONFIGURED, 4=USBD_STATE_SUSPENDED
uint32_t 	nyh_nUSBdev_state = USBD_STATE_DEFAULT;

uint8_t 	UserTxBuffer[31] = "STM32 Virtual COM Port Demo.\r\n";

uint8_t	g_mSW2_buf = 0;

/*
void Regist_VcomO_Queue(void)
{
	uint8_t i = 0;
	
	for( i=0; i<24; i++ )	g_mVcomO_Quebuf[g_mVcomO_inptr][i] = g_mOutBuf_usbVcom[i];
	g_mVcomO_inptr++;	g_mVcomO_inptr &= 0x0F;
}

void mCheck_VirtualCOMsend(void)
{
	if (g_mFsend_usbVcom == 0)	return;
	g_mFsend_usbVcom = 0;
	
	g_mPutCount_usbVcom++;
	g_mPutCount_usbVcom &= 0xFF;

	g_mOutBuf_usbVcom[2] = g_mMain_step;
	g_mOutBuf_usbVcom[3] = g_mFunc_step;
	g_mOutBuf_usbVcom[4] = g_mTestSW2;
	g_mOutBuf_usbVcom[5] = g_mInkey;
	g_mOutBuf_usbVcom[6] = g_mPutCount_usbVcom;
	g_mOutBuf_usbVcom[7] = g_mGetCount_usbVcom;
	
	g_mOutBuf_usbVcom[23] = 0xAA;

	Regist_VcomO_Queue();
	//UserToPMABufferCopy(g_mOutBuf_usbVcom, ENDP1_TXADDR, g_nTxCount_usbVcom);	//Tx to PC = Sendout to virtualport
	//SetEPTxCount(ENDP1, g_nTxCount_usbVcom);
	//SetEPTxValid(ENDP1);	
}
*/
void	nyh_mLoop(void)
{
	if (nyh_nUSBdev_state != USBD_STATE_CONFIGURED )	return;
	
	static GPIO_PinState bSW2_st = GPIO_PIN_RESET;

	g_mSW2_buf = HAL_GPIO_ReadPin(inBD_sw2_GPIO_Port , inBD_sw2_Pin );
	
	if(HAL_GPIO_ReadPin(inBD_sw2_GPIO_Port , inBD_sw2_Pin ) != bSW2_st)
	{
		bSW2_st = HAL_GPIO_ReadPin(inBD_sw2_GPIO_Port , inBD_sw2_Pin );
		if (bSW2_st == GPIO_PIN_RESET)
		{
			for (int i=0; i<10;i++)	UserTxBufferFS[i] = 0x30+i;
			CDC_Transmit_FS(UserTxBufferFS, 10);					//º¸³»±â Vcom
		}
	}	
}
//=================
//=================
uint8_t	g_mLED_life = 0, mTestPulse1_st = 0;
void iTIM2_lifeLED(void)
{
	g_mLED_life++;
	if (g_mLED_life < 100)	return;
	g_mLED_life = 0;
	
	mTestPulse1_st ^= 0xFF;
	if (mTestPulse1_st == 0)
		HAL_GPIO_WritePin(oBD_LED_GPIO_Port, oBD_LED_Pin, GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(oBD_LED_GPIO_Port, oBD_LED_Pin, GPIO_PIN_SET);	
}

void 	nyh_TIM2_IRQHandler(void)	//from TIM2_IRQHandler
{
	nyh_nUSBdev_state = hUsbDeviceFS.dev_state;

	iTIM2_lifeLED();
}

//=================
void nyh_HAL_PCD_IRQHandler(void)
{
	volatile int i=0;
	//PCD_HandleTypeDef *hpcd;
	//hpcd = &hpcd_USB_FS;	
	
	//PCD_EPTypeDef *ep;
        //ep = &hpcd->OUT_ep[0];
	//ep->xfer_count = PCD_GET_EP_RX_CNT(hpcd->Instance, ep->num);
	
	if (UserRxBufferFS[0] != '\0')
	{
		memcpy(UserTxBufferFS, UserRxBufferFS, sizeof(UserRxBufferFS));
		CDC_Transmit_FS(UserTxBufferFS, 10);
		memset(UserRxBufferFS, 0, sizeof(UserRxBufferFS));
		i += 1;
	}
}
