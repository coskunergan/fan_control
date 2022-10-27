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

__code uint16_t __at (_CONFIG)   __configword = 0xF304;

#define LED_PORT GPIObits.GP2
#define LED_TRIS TRISIObits.TRISIO2

void delay(uint16_t iterations)
{
    for(uint16_t i = 0; i < iterations; i++)
    {
        __asm nop __endasm;
    }
}

int main(void)
{
    ANSELbits.ANS=0;          
    ANSELbits.ADCS=0;   
    CMCON0bits.CM=0x7;          
    // LED_TRIS = 0;
   	// LED_PORT = 0;
    TRISIObits.TRISIO=0;	
    while(1)
    {
        // LED_PORT = 1;
        GPIObits.GP=255;
        delay(300);
        // LED_PORT = 0;
        GPIObits.GP=0;
        delay(300);
    }
}