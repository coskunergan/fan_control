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

__code uint16_t __at(_CONFIG) __configword = _INTRC_OSC_NOCLKOUT & _WDTE_OFF & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _BOREN_ON & _FCMEN_OFF & _IESO_OFF;

#define LED_PORT GPIObits.GP5
#define LED_TRIS TRISIObits.TRISIO5

#define UART_TX_TRIS TRISIObits.TRISIO0
#define UART_TX GPIObits.GP0
#define UART_TX_DELAY 18 // baudrate 2400

typedef enum
{
    SUCCES = 0,
    ERROR
} State_t;
/**********************************/
/**********************************/
/**********************************/
void delay(volatile uint16_t iterations)
{
    for(uint16_t i = 0; i < iterations; i++)
    {
        __asm nop __endasm;
    }
}
/**********************************/
void Led_State(State_t s)
{
    if(s == ERROR)
    {
        while(1)
        {
            LED_PORT = 1;
            delay(3000);
            LED_PORT = 0;
            delay(3000);
        }
    }
    else
    {
        while(1)
        {
            LED_PORT = 1;
            delay(30000);
            LED_PORT = 0;
            delay(30000);
        }
    }
}
/**********************************/
void UART_Transmit(const char DataValue)
{
    delay(UART_TX_DELAY);
    UART_TX = 0;
    delay(UART_TX_DELAY);
    for(unsigned char i = 0; i < 8; i++)
    {
        if(((DataValue >> i) & 0x1) == 0x1)
        {
            UART_TX = 1;
        }
        else
        {
            UART_TX = 0;
        }
        delay(UART_TX_DELAY);
    }
    UART_TX = 1;
}
/**********************************/
int main(void)
{
    ANSELbits.ANS = 0;
    ANSELbits.ADCS = 0;
    CMCON0bits.CM = 0x7;
    LED_TRIS = 0;
    LED_PORT = 0;
    UART_TX_TRIS = 0;
    UART_TX = 1;   

    UART_Transmit(0xAA);
    UART_Transmit(0xAA);
    UART_Transmit(0xAA);

    Led_State(SUCCES);
}
/**********************************/
/**********************************/
/**********************************/
