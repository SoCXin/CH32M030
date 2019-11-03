/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/01/19
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
/*
 *@Note
  This routine demonstrates the use of USBFS to emulate a custom CH372 device, 
  with endpoints 1/3/5 downloading data and uploading it via endpoints 2/4/6 respectively
  where endpoint 1/2 is implemented via a ring buffer and the data is not inverted, 
  and endpoints 3/4 and 5/6 are directly copied and inverted for upload.
  The device can be operated using Bushund or other upper computer software.
  Note: This routine needs to be demonstrated in conjunction with the host software.
*/

#include "debug.h"
#include "ch32m030_usbfs_device.h"

/* Global define */

/* Global Variable */


/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    uint8_t ret;

    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );

    USBFS_RCC_Init();
    USBFS_Device_Init(ENABLE);

    while(1)
    {
        /* Determine if enumeration is complete, perform data transfer if completed */
        if(USBFS_DevEnumStatus)
        {
            /* Data Transfer */
            if(RingBuffer_Comm.RemainPack)
            {
                ret = USBFS_Endp_DataUp(DEF_UEP2, &Data_Buffer[(RingBuffer_Comm.DealPtr) * DEF_USBD_FS_PACK_SIZE], RingBuffer_Comm.PackLen[RingBuffer_Comm.DealPtr], DEF_UEP_CPY_LOAD);
                if( ret == 0 )
                {
                    NVIC_DisableIRQ(USBFS_IRQn);
                    RingBuffer_Comm.RemainPack--;
                    RingBuffer_Comm.DealPtr++;
                    if(RingBuffer_Comm.DealPtr == DEF_Ring_Buffer_Max_Blks)
                    {
                        RingBuffer_Comm.DealPtr = 0;
                    }
                    NVIC_EnableIRQ(USBFS_IRQn);
                }
            }

            /* Monitor whether the remaining space is available for further downloads */
            if(RingBuffer_Comm.RemainPack < (DEF_Ring_Buffer_Max_Blks - DEF_RING_BUFFER_RESTART))
            {
                if(RingBuffer_Comm.StopFlag)
                {
                    NVIC_DisableIRQ(USBFS_IRQn);
                    RingBuffer_Comm.StopFlag = 0;
                    NVIC_EnableIRQ(USBFS_IRQn);
                    USBFSD->UEP1_RX_CTRL = (USBFSD->UEP1_RX_CTRL & ~USBFS_UEP_R_RES_MASK) | USBFS_UEP_R_RES_ACK;
                }
            }
        }
    }
}
