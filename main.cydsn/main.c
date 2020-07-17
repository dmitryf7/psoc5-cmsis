/*
 * Copyright (c) 2020 Dmitrii Fedotov dmitryf7@gmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the Licence at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "project.h"

#include <stdint.h>
#include "rtx_os.h"

uint32_t SystemCoreClock = BCLK__BUS_CLK__HZ;

osThreadId_t tiduart;
osThreadId_t tidblink;
    
void printUART(void *argument)
{
    (void) argument;
    
    for(;;)
    {
        osDelay(100);
        UART_1_PutString("Hello from PSoC!!!\n");
    }
}

void blinkingLED(void *argument)
{
    (void) argument;
    static uint8_t led_value = 1;
    
    for(;;)
    {
        osDelay(400);
        
        led_value = 1 - led_value;
        USER_LED_Write(led_value);
    }    
}

void app_main (void *argument)
{
    (void) argument;
    
    static osRtxThread_t uarttrcb;
    static uint64_t uarttrstack[512/8];    
    static osRtxThread_t blinktrcb;
    static uint64_t blinktrstack[512/8];
    
    const osThreadAttr_t uarttrattr   = { "Print UART",    osThreadRunning, &uarttrcb, sizeof(uarttrcb), &uarttrstack, sizeof(uarttrstack), osPriorityNormal, 0, 0};
    const osThreadAttr_t blinktrattr   = { "Blinking LED", osThreadRunning, &blinktrcb, sizeof(blinktrcb), &blinktrstack, sizeof(blinktrstack), osPriorityNormal, 0, 0};

    tiduart = osThreadNew(printUART, NULL, &uarttrattr);
    tidblink = osThreadNew(blinkingLED, NULL, &blinktrattr);
    osThreadExit();
}

int main(void)
{       
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    UART_1_Start(); /* UART configured as 115200 8N1 */
    
    /* Set interrupt vectors for CMSIS functionality */
    CyIntSetSysVector(CY_INT_SVCALL_IRQN, SVC_Handler);
    CyIntSetSysVector(CY_INT_PEND_SV_IRQN, PendSV_Handler);
    CyIntSetSysVector(CY_INT_SYSTICK_IRQN, SysTick_Handler);
        
    osKernelInitialize();	

    osThreadNew(app_main, NULL, NULL);    /* Create application main thread */

    if (osKernelGetState() == osKernelReady)
    {
        osKernelStart();                  /* Start thread execution, never returned */
    }    
}
