/**
  * TMR1 Generated Timer Driver File
  *
  * @file tmr1.c
  *
  * @ingroup timerdriver
  *
  * @brief Driver implementation for the TMR1 Timer driver
  *
  * @version TMR1 Timer Driver Version 4.0.0
  * 
  * @version Package Version 5.1.0
  */
/*
© [2025] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/

/**
 * Section: Included Files
*/

#include <xc.h>
#include "../tmr1.h"

const struct TIMER_INTERFACE Timer1 = {
    .Initialize = TMR1_Initialize,
    .Deinitialize = TMR1_Deinitialize,
    .Start = TMR1_Start,
    .Stop = TMR1_Stop,
    .PeriodSet = TMR1_PeriodSet,
    .PeriodGet = TMR1_PeriodGet,
    .CounterGet = TMR1_CounterGet,
    .CounterSet = TMR1_CounterSet,
    .MaxCountGet = TMR1_MaxCountGet,
    .TimeoutCallbackRegister = TMR1_OverflowCallbackRegister,
    .Tasks = NULL
};

static volatile uint16_t timer1ReloadVal;
static void TMR1_DefaultOverflowCallback(void);
static void (*TMR1_OverflowCallback)(void);

void TMR1_Initialize(void)
{
    T1CONbits.TMR1ON = 0;             // TMRON disabled

    T1GCON = (0 << _T1GCON_T1GGO_POSN)   // T1GGO done
        | (0 << _T1GCON_T1GSPM_POSN)   // T1GSPM disabled
        | (0 << _T1GCON_T1GTM_POSN)   // T1GTM disabled
        | (0 << _T1GCON_T1GPOL_POSN)   // T1GPOL low
        | (0 << _T1GCON_T1GE_POSN);  // T1GE disabled

    T1GATE = (0 << _T1GATE_GSS_POSN);  // GSS T1G_pin

    T1CLK = (1 << _T1CLK_CS_POSN);  // CS Fosc/4 -----> mora bit zbog CCP-a (tako pise) i mora bit syncronize upaljen dole u T1CON

    TMR1H = 0xFF;              // Timer clock 4000000 Hz
    TMR1L = 0xFF;

    timer1ReloadVal=((uint16_t)TMR1H << 8) | TMR1L;
    TMR1_OverflowCallback = TMR1_DefaultOverflowCallback;

    PIR4bits.TMR1IF = 0;   
    //PIE4bits.TMR1IE = 1;  tu opce necu koristit ovaj interrupt od tmr1 jer mi on sluzi samo za ccp1 da kad on izbroji neku vrijednost da okine
  
    T1CON = (0 << _T1CON_TMR1ON_POSN)   // TMR1ON disabled
        | (1 << _T1CON_T1RD16_POSN)   // T1RD16 enabled
        | (0 << _T1CON_nT1SYNC_POSN)   // nT1SYNC synchronize
        | (2 << _T1CON_CKPS_POSN);  // CKPS 1:4
}

void TMR1_Deinitialize(void)
{
    T1CONbits.TMR1ON = 0;             // TMRON disabled
    
    T1CON = 0x0;
    T1GCON = 0x0;
    T1GATE = 0x0;
    T1CLK = 0x0;
    TMR1H = 0x0;
    TMR1L = 0x0;

    PIR4bits.TMR1IF = 0;
    PIE4bits.TMR1IE = 0;

    PIR5bits.TMR1GIF = 0;
    PIE5bits.TMR1GIE = 0;
}

void TMR1_Start(void)
{   
    T1CONbits.TMR1ON = 1;
}

void TMR1_Stop(void)
{ 
    T1CONbits.TMR1ON = 0;
}

uint32_t TMR1_CounterGet(void)
{
    uint16_t readVal;
    uint8_t readValHigh;
    uint8_t readValLow;
    	
    readValLow = TMR1L;
    readValHigh = TMR1H;
    
    readVal = ((uint16_t)readValHigh << 8) | readValLow;

    return (uint32_t)readVal;
}

void TMR1_CounterSet(uint32_t timerVal)
{
    if(1U == T1CONbits.nT1SYNC)
    {
        bool onState = T1CONbits.TMR1ON;

        T1CONbits.TMR1ON = 0;      
        TMR1H = (uint8_t)(timerVal >> 8);
        TMR1L = (uint8_t)timerVal;       
        T1CONbits.TMR1ON = onState;
    }
    else
    {      
        TMR1H = (uint8_t)(timerVal >> 8);
        TMR1L = (uint8_t)timerVal;
    }
}

void TMR1_PeriodSet(uint32_t periodVal)
{
    timer1ReloadVal =  TMR1_MAX_COUNT - (uint16_t)periodVal;
    /* cppcheck-suppress misra-c2012-8.7 */
    TMR1_CounterSet(timer1ReloadVal);
}

uint32_t TMR1_PeriodGet(void)
{
    return ((uint32_t)TMR1_MAX_COUNT - timer1ReloadVal);
}

uint32_t TMR1_MaxCountGet(void)
{
    return (uint32_t)TMR1_MAX_COUNT;
}

void TMR1_OverflowISR(void)
{
    /* cppcheck-suppress misra-c2012-8.7 */
    TMR1_CounterSet(timer1ReloadVal);

    // The ticker is set to 1 -> The callback function gets called every time this ISR executes.
    if(NULL != TMR1_OverflowCallback)
    {
        TMR1_OverflowCallback();
    }
    PIR4bits.TMR1IF = 0;
}

void TMR1_OverflowCallbackRegister(void (* CallbackHandler)(void))
{
    TMR1_OverflowCallback = CallbackHandler;
}

static void TMR1_DefaultOverflowCallback(void)
{
    // Default interrupt handler
}
/**
  End of File
*/
