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

#define TICK_PERIOD       1e-3   /*1mS period must be capable of resolution all procces. */
#define ADC_PERIOD        10e-3  /*10mS*/
#define CALC_PERIOD       50e-3 /*100mS*/
#define PWM_PERIOD        1e-3   /*1mS*/
#define LED_PERIOD        2e-3   /*2mS*/
#define UART_BIT_PERIOD   2e-3   /*2mS(~490baudrate)*/
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

const uint8_t PeriodTable[] = // sizeof ??
{
    ADC_PERIOD / TICK_PERIOD,
    CALC_PERIOD / TICK_PERIOD,
    PWM_PERIOD / TICK_PERIOD,
    LED_PERIOD / TICK_PERIOD,
    UART_BIT_PERIOD / TICK_PERIOD,
    PRINT_PERIOD / TICK_PERIOD
};

bool Event = false;
Procces_t Procces;
uint8_t TimeTable[ASIZE(PeriodTable)] = {};
char UartBuffer[24];
uint8_t Uart_Gone_Index = 0;
uint8_t Uart_Going_Index = 0;
/**********************************/
/**********************************/
/**********************************/
void Transmit_Uart(const uint8_t *ptr)
{
    while(*ptr != '\0')
    {
        UartBuffer[Uart_Going_Index++] = *ptr++;
        if(Uart_Going_Index == (sizeof(UartBuffer) - 1))
        {
            Uart_Going_Index = 0;
        }
    }
}
/**********************************/
void Transmit_Bit(void)
{
    static uint8_t bit = 0;
    static uint8_t byte;

    if(Uart_Gone_Index == Uart_Going_Index)
    {
        return;
    }
    switch(bit)
    {
        case 0:
            UART_TX = 0;
            bit++;
            byte = UartBuffer[Uart_Gone_Index];
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            if(byte & (0x1 << (bit - 1)))
            {
                UART_TX = 1;
            }
            else
            {
                UART_TX = 0;
            }
            bit++;
            break;
        case 9:
        default:
            UART_TX = 1;
            bit = 0;
            Uart_Gone_Index++;
            if(Uart_Gone_Index == (sizeof(UartBuffer) - 1))
            {
                Uart_Gone_Index = 0;
            }
            break;
    }
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
    PS0    = 1;     // (Fosc/4)/8 olarak ayarlaniyor!
    PS1    = 0;
    PS2    = 1;
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

    // UART_Transmit('R');
    // UART_Transmit('S');
    // UART_Transmit('T');
    // UART_Transmit('\n');

    UartBuffer[0] = '\0';
    Transmit_Bit();

    PEIE = 1;
    GIE = 1;

    while(1)
    {
        if(Event)
        {
            LED_PIN = 1;
            for(Procces = 0; Procces < ASIZE(PeriodTable); Procces++)
            {
                if(TimeTable[Procces] == 0)
                {
                    TimeTable[Procces] = PeriodTable[Procces];
                    break;
                }
                TimeTable[Procces]--;
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
                    Transmit_Bit();
                    break;
                case PRINT:
                    static bool toggle_print = false;
                    toggle_print = !toggle_print;
                    if(toggle_print)
                    {
                        Transmit_Uart("ADC:1024\r\n");
                    }
                    else
                    {
                        Transmit_Uart("PWM:%100\r\n");
                    }
                    break;
                default:
                    Event = false;
                    break;
            }
            LED_PIN = 0;
        }
    }
}
/**********************************/
/**********************************/
/**********************************/
void TC0_ISR(void) __interrupt(4)
{
    //if(TMR0IF)
    {
        TMR0    = -((31.25 / 1e-3) * TICK_PERIOD);
        TMR0IF  = 0;
        Event = true;
    }
}
