/*
 *  Fan Control Software
 *
 *  Created on: Oct 27, 2022
 *
 *  Author: Coskun ERGAN
 */

#include <pic14regs.h>
#include <stdint.h>
#include <stdbool.h>

__code uint16_t __at(_CONFIG) __configword = _INTRC_OSC_NOCLKOUT & _WDTE_OFF & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _BOREN_ON & _FCMEN_OFF & _IESO_OFF;

#define ASIZE(x) (sizeof(x)/sizeof(*x)) // K.Kurt
#define MIN(a,b) (((a)<(b))?(a):(b)) // ??
#define MAX(a,b) (((a)>(b))?(a):(b))

#define ADC_PIN         GP4
#define ADC_TRIS        TRISIO4

#define PWM_PIN         GP2
#define PWM_TRIS        TRISIO2

#define LED_PIN         GP5
#define LED_TRIS        TRISIO5

#define UART_TX_TRIS    TRISIO0
#define UART_TX         GP0
#define UART_TX_DELAY   18 //~2400 baudrate

#define TICK_PERIOD       1e-3   /*1mS period must be capable of resolution all procces. */
#define ADC_PERIOD        10e-3  /*10mS*/
#define CALC_PERIOD       100e-3 /*100mS*/
#define PWM_PERIOD        1e-3   /*1mS*/
#define LED_PERIOD        2e-3   /*2mS*/
#define UART_BIT_PERIOD   1e-3   /*1mS(~1200baud)*/
#define PRINT_PERIOD      250e-3 /*250mS*/  
#define TICK_COUNT        (PRINT_PERIOD / TICK_PERIOD) // must be max. process period  ???????????????

typedef enum
{
    ADC = 0,
    CALC,
    PWM,
    LED,
    UART_BIT,
    PRINT
} Procces_t; // sizeof ??

const unsigned Period_Table[] = // sizeof ??
{
    ADC_PERIOD / TICK_PERIOD,
    CALC_PERIOD / TICK_PERIOD,
    PWM_PERIOD / TICK_PERIOD,
    LED_PERIOD / TICK_PERIOD,
    UART_BIT_PERIOD / TICK_PERIOD,
    PRINT_PERIOD / TICK_PERIOD
};

bool Event = false;
uint8_t TimerTick = 0;
Procces_t Procces;
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
void InitIO(void)
{
    ANSEL = 0x00;
    CMCON0 = 0x07;
    ADC_TRIS = 1;
    ADC_PIN = 0;
    LED_TRIS = 0;
    LED_PIN = 0;
    UART_TX_TRIS = 0;
    UART_TX = 1;
}
/**********************************/
void InitTMR0(void)
{
    T0CS   = 0;     // Fosc/4 kullanilacak
    PSA    = 0;     // Timer0 icin prescaler
    PS0    = 0;     // (Fosc/4)/8 olarak ayarlaniyor!
    PS1    = 1;
    PS2    = 0;
    TMR0IF = 0;
    TMR0IE = 1;
}
/**********************************/
void InitADC(void)
{
    ANSEL  = 0x38;                 // AN3 girisi ADC
    ADCON0 = 0x0D;
}
/**********************************/
void InitOSC(void)
{
    IRCF0 = 1;
    IRCF1 = 1;
    IRCF2 = 1;
    SCS  = 1;
    while(!HTS);
}
/**********************************/
void main(void)
{
    InitOSC();
    InitIO();
    InitTMR0();
    InitADC();

    PEIE = 1;
    GIE = 1;

    UART_Transmit(0xAA);
    UART_Transmit(0x55);
    UART_Transmit(0xA5);

    //Led_State(SUCCES);

    while(1)
    {
        if(Event)
        {
            for(; Procces < ASIZE(Period_Table); Procces++)
            {
                if((TimerTick % Period_Table[Procces]) == 0)
                {
                    break;
                }
            }
            switch(Procces)
            {
                case ADC:
                    break;
                case CALC:
                    break;
                case PWM:
                    break;
                case LED:
                    break;
                case UART_BIT:
                    break;
                case PRINT:
                    break;                    
                default:
                    Procces = 0;
                    Event = false;
                    if(++TimerTick >= TICK_COUNT)
                    {
                        TimerTick = 0;
                    }
                    break;
            }
        }
    }
}
/**********************************/
/**********************************/
/**********************************/
void TC0_ISR(void) __interrupt(0)
{
    //if(TMR0IF)
    {
        TMR0    = -((125.0 / 1e-3) * TICK_PERIOD);
        TMR0IF  = 0;
        Event = true;
    }
}
