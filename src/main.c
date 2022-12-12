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

__code uint16_t __at(_CONFIG) __configword = _INTRC_OSC_NOCLKOUT & _WDTE_ON & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _BOREN_ON & _FCMEN_OFF & _IESO_OFF;

#define ADC_MIN      130
#define ADC_MAX      220
#define RMS_SAMPLE_COUNT 256

#define ADC_PIN         GP4
#define ADC_TRIS        TRISIO4

#define PWM_PIN         GP2
#define PWM_TRIS        TRISIO2

#define LED_PIN         GP5
#define LED_TRIS        TRISIO5

#define TICK_PERIOD       1e-3   /*1mS  */
#define ADC_PERIOD        5e-3   /*5mS */
#define RMS_PERIOD        250e-3 /*250mS*/

typedef enum
{
    ADC = 0,
    RMS,
};

const uint8_t PeriodTable[] =
{
    ADC_PERIOD / TICK_PERIOD,
    RMS_PERIOD / TICK_PERIOD
};
uint8_t TimeTable[sizeof(PeriodTable)] = {0};
bool Event = false;
uint32_t ADC_Value = 0;
uint16_t ADC_Raw;
uint32_t ADC_Sum = ADC_MAX * ADC_MAX * RMS_SAMPLE_COUNT;
/**********************************/
/**********************************/
/**********************************/
void Read_ADC(void)
{
    //--------
    while(GO_DONE);
    ADC_Raw = ADRESH;
    ADC_Raw <<= 8;
    ADC_Raw |= ADRESL;
    GO_DONE = 1;
    //---------
    ADC_Sum -= ADC_Sum / RMS_SAMPLE_COUNT;
    ADC_Sum += (uint32_t)ADC_Raw * (uint32_t)ADC_Raw;
}
/**********************************/
void RMS_Calc(void)
{
    if(ADC_Value == 0)
    {
        ADC_Value = 1;
    }
    ADC_Value = (ADC_Value + ((ADC_Sum / RMS_SAMPLE_COUNT) / ADC_Value)) / 2;

    if(ADC_Value > ADC_MAX)
    {
        LED_PIN = 0;
        TMR2ON = 0; // PWM stop
        CCP1CON = 0;
        PWM_PIN = 0;
    }
    else if(ADC_Value <= ADC_MIN)
    {
        LED_PIN = 1;
        TMR2ON = 0; // PWM stop
        CCP1CON = 0;
        PWM_PIN = 1;
    }
    else
    {
        CCPR1L = 255 - ((ADC_MAX - ADC_MIN) - (ADC_Value - ADC_MIN));
        //CCPR1L = (((ADC_Value - ADC_MIN) * 256UL) / (ADC_Value - ADC_MIN)) - 1;
        LED_PIN = !LED_PIN;
        CCP1CON = 0xF;
        TMR2ON = 1; // PWM start
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
    CCP1CON = 0x0F;
}
/**********************************/
void InitTMR0(void)
{
    T0CS   = 0;
    PSA    = 0;
    PS0    = 1;
    PS1    = 0;
    PS2    = 1;
    TMR0IF = 0;
    TMR0IE = 1;
}
/**********************************/
void InitADC(void)
{
    ANSEL  = 0x38;
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
            Event = false;
            __asm CLRWDT __endasm;
            if(--TimeTable[ADC] == 0)
            {
                TimeTable[ADC] = PeriodTable[ADC];
                Read_ADC();
            }
            if(--TimeTable[RMS] == 0)
            {
                TimeTable[RMS] = PeriodTable[RMS];
                RMS_Calc();
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
