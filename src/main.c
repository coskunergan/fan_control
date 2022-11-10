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

#define ADC_MIN      128
#define ADC_MAX      250
#define ADC_SAMPLE_COUNT 32

#define ADC_PIN         GP4
#define ADC_TRIS        TRISIO4

#define PWM_PIN         GP2
#define PWM_TRIS        TRISIO2

#define LED_PIN         GP5
#define LED_TRIS        TRISIO5

#define UART_TX_TRIS    TRISIO0
#define UART_TX         GP0

#define TICK_PERIOD       1e-3   /*1mS period must be capable of resolution all procces. */
#define ADC_PERIOD        5e-3   /*10mS*/
#define RMS_PERIOD        250e-3 /*50mS*/
#define LED_PERIOD        2e-3   /*2mS*/
#define UART_BIT_PERIOD   2e-3   /*2mS(~490baudrate)*/
#define PRINT_PERIOD      250e-3 /*250mS*/

typedef enum
{
    SUCCES = 0,
    ERROR
} State_t;

typedef enum
{
    ADC = 0,
    RMS,
    LED,
    UART_BIT,
    PRINT
} Procces_t; // sizeof ??

const uint8_t PeriodTable[] = // sizeof ??
{
    ADC_PERIOD / TICK_PERIOD,
    RMS_PERIOD / TICK_PERIOD,
    LED_PERIOD / TICK_PERIOD,
    UART_BIT_PERIOD / TICK_PERIOD,
    PRINT_PERIOD / TICK_PERIOD
};

bool Event = false;
Procces_t Procces;
uint8_t TimeTable[ASIZE(PeriodTable)] = {};
char UartBuffer[16];
uint8_t Uart_Gone_Index = 0;
uint8_t Uart_Going_Index = 0;
State_t State = SUCCES;
uint16_t ADC_Value = 0;
uint32_t ADC_Sum = 0;
uint8_t PWM_Value = 0;
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
// void Led_State(State_t state)
// {
//     static uint8_t dimmer = 1;
//     static bool flip = false;
//     static uint8_t count = 0;
//     static uint8_t step = 0;
// #define LED_SPEED 15
//     if(state == ERROR)
//     {
//         if(++count == 50)
//         {
//             count = 0;
//             LED_PIN = !LED_PIN;
//         }
//     }
//     else
//     {
//         if(++count == 10)
//         {
//             LED_PIN = 1;
//             count = 0;
//             step += dimmer;
//             if(step > LED_SPEED)
//             {
//                 step = 0;
//                 if(flip)
//                 {
//                     if(--dimmer == 1)
//                     {
//                         flip = false;
//                     }
//                 }
//                 else
//                 {
//                     if(++dimmer == 10)
//                     {
//                         flip = true;
//                     }
//                 }
//             }
//         }
//         if(count == dimmer)
//         {
//             LED_PIN = 0;
//         }
//     }
// }
/**********************************/
// void Convert_DecToASCI(uint16_t val, char *ptr)
// {
//     *ptr = (val / 1000);
//     if(*ptr)
//     {
//         *ptr += '0';
//         ptr++;
//     }
//     *ptr = (val / 100) % 10;
//     *ptr += '0';
//     ptr++;
//     *ptr = (val / 10) % 10;
//     *ptr += '0';
//     ptr++;
//     *ptr = val % 10;
//     *ptr += '0';
//     ptr++;
//     *ptr = '\0';
// }
/**********************************/
void RMS_Calc(void)
{
    ADC_Value = (ADC_Value + ((ADC_Sum / ADC_SAMPLE_COUNT) / ADC_Value)) / 2;
    if(ADC_Value > ADC_MAX)
    {
        PWM_Value = 100;
        CCPR1L = 255;
    }
    else if(ADC_Value < ADC_MIN)
    {
        PWM_Value = 0;
        CCPR1L = 0;
    }
    else
    {
        PWM_Value = ADC_Value / ((ADC_MAX - ADC_MIN) / 100);
        CCPR1L = ADC_Value / ((ADC_MAX - ADC_MIN) / 255);
    }
}
/**********************************/
void Adc_Read(void)
{
    uint16_t adc;
    //--------
    while(GO_DONE);
    adc = ADRESH;
    adc <<= 8;
    adc |= ADRESL;
    GO_DONE = 1;
    //---------
    adc >>= 2;
    ADC_Sum -= ADC_Sum / ADC_SAMPLE_COUNT;
    ADC_Sum += (uint16_t)adc * adc;
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
    PWM_TRIS = 0;
    PWM_PIN = 0;
}
/**********************************/
void InitTMR2Pwm(void)
{
    T2CKPS0 = 0;
    T2CKPS1 = 0;
    TOUTPS0 = 0;
    TOUTPS1 = 0;
    TOUTPS2 = 0;
    TOUTPS3 = 0;
    TMR2ON = 1;
    PR2 = 255;
    CCP1CON = 0x0C;
    CCPR1L = 128;
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
    ADCON0 = 0x8D;
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
    InitTMR2Pwm();
    InitADC();

    PEIE = 1;
    GIE = 1;

    while(1)
    {
        if(Event)
        {
            for(; Procces < ASIZE(PeriodTable); Procces++)
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
                    Adc_Read();
                    break;
                case RMS:
                    RMS_Calc();
                    break;
                case LED:
                    //Led_State(State);
                    break;
                case UART_BIT:
                    Transmit_Bit();
                    break;
                case PRINT:
                    static bool toggle_print = false;
                    toggle_print = !toggle_print;
                    if(toggle_print)
                    {
                        // Transmit_Uart("ADC:");
                        char buffer[5];
                        // Convert_DecToASCI(ADC_Value, buffer);
                        // Transmit_Uart(buffer);
                        // Transmit_Uart("\r\n");

                        buffer[0] = ADC_Value;
                        buffer[1] = PWM_Value;
                        buffer[2] = 0;
                        Transmit_Uart(buffer);
                        Transmit_Uart("\r\n");

                    }
                    else
                    {
                        // // Transmit_Uart("PWM:%");
                        // char buffer[4];
                        // // Convert_DecToASCI(PWM_Value, buffer);
                        // // Transmit_Uart(buffer);
                        // // Transmit_Uart("\r\n");
                        // buffer[0] = PWM_Value;
                        // buffer[1] = 0;
                        // Transmit_Uart(buffer);
                        // Transmit_Uart("\r\n");
                    }
                    break;
                default:
                    Procces = 0;
                    Event = false;
                    break;
            }
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
