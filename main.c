/*
 *  Fan Control Software
 *
 *  Created on: Oct 27, 2022
 *
 *  Author: Coskun ERGAN
 */

#define NO_BIT_DEFINES
#include <pic16regs.h>
#include <stdint.h>

__code uint16_t __at (_CONFIG) __configword = _INTRC_OSC_NOCLKOUT & _WDTE_OFF;

#define LED_PORT GPIObits.GP0
#define LED_TRIS TRISIObits.TRISIO0

void delay(volatile uint16_t iterations)
{
    for(uint16_t i = 0; i < iterations; i++)
    {
        __asm nop __endasm;
    }
}

void main(void)
{
    LED_TRIS = 0;
   	LED_PORT = 0;	

    while(1)
    {
        LED_PORT = 1;
        delay(30000);
        LED_PORT = 0;
        delay(30000);
    }
}