#include <xc.h>
#include "../tmr3.h"

const struct TIMER_INTERFACE Timer3 = {
    .Initialize = TMR3_Initialize,
    .Deinitialize = TMR3_Deinitialize,
    .Start = TMR3_Start,
    .Stop = TMR3_Stop,
    .PeriodSet = TMR3_PeriodSet,
    .PeriodGet = TMR3_PeriodGet,
    .CounterGet = TMR3_CounterGet,
    .CounterSet = TMR3_CounterSet,
    .MaxCountGet = TMR3_MaxCountGet,
    .TimeoutCallbackRegister = TMR3_OverflowCallbackRegister,
    .Tasks = NULL
};

static volatile uint16_t timer3ReloadVal;
static void TMR3_DefaultOverflowCallback(void);
static void (*TMR3_OverflowCallback)(void);

void TMR3_Initialize(void)
{
    T3CONbits.TMR3ON = 0;             // TMRON disabled

    T3GCON = (0 << _T3GCON_T3GGO_POSN)   // T3GGO done
        | (0 << _T3GCON_T3GSPM_POSN)   // T3GSPM disabled
        | (0 << _T3GCON_T3GTM_POSN)   // T3GTM disabled
        | (0 << _T3GCON_T3GPOL_POSN)   // T3GPOL low
        | (0 << _T3GCON_T3GE_POSN);  // T3GE disabled

    T3GATE = (0 << _T3GATE_GSS_POSN);  // GSS T3G_pin

    T3CLK = (3 << _T3CLK_CS_POSN);  // CS HFINTOSC (Fosc = HFINTOSC)

    TMR3H = 0xEB;              // Period 80us; Timer clock 64000000 Hz
    TMR3L = 0xFF;
    
    timer3ReloadVal=((uint16_t)TMR3H << 8) | TMR3L;
    TMR3_OverflowCallback = TMR3_DefaultOverflowCallback;

    PIR4bits.TMR3IF = 0;   
    PIE4bits.TMR3IE = 1;
  
    T3CON = (0 << _T3CON_TMR3ON_POSN)   // TMR3ON disabled tako da se pokre?e tek kad se pozove u kodu
        | (0 << _T3CON_T3RD16_POSN)   // T3RD16 disabled
        | (1 << _T3CON_nT3SYNC_POSN)   // nT3SYNC do_not_synchronize
        | (0 << _T3CON_CKPS_POSN);  // CKPS 1:1
}

void TMR3_Deinitialize(void)
{
    T3CONbits.TMR3ON = 0;             // TMRON disabled
    
    T3CON = 0x0;
    T3GCON = 0x0;
    T3GATE = 0x0;
    T3CLK = 0x0;
    TMR3H = 0x0;
    TMR3L = 0x0;

    PIR4bits.TMR3IF = 0;
    PIE4bits.TMR3IE = 0;

    PIR5bits.TMR3GIF = 0;
    PIE5bits.TMR3GIE = 0;
}

void TMR3_Start(void)
{   
    T3CONbits.TMR3ON = 1;
}

void TMR3_Stop(void)
{ 
    T3CONbits.TMR3ON = 0;
}

uint32_t TMR3_CounterGet(void)
{
    uint16_t readVal;
    uint8_t readValHigh;
    uint8_t readValLow;
    	
    readValLow = TMR3L;
    readValHigh = TMR3H;
    
    readVal = ((uint16_t)readValHigh << 8) | readValLow;

    return (uint32_t)readVal;
}

void TMR3_CounterSet(uint32_t timerVal)
{
    if(1U == T3CONbits.nT3SYNC)
    {
        bool onState = T3CONbits.TMR3ON;

        T3CONbits.TMR3ON = 0;      
        TMR3H = (uint8_t)(timerVal >> 8);
        TMR3L = (uint8_t)timerVal;       
        T3CONbits.TMR3ON = onState;
    }
    else
    {      
        TMR3H = (uint8_t)(timerVal >> 8);
        TMR3L = (uint8_t)timerVal;
    }
}

void TMR3_PeriodSet(uint32_t periodVal)
{
    timer3ReloadVal =  TMR3_MAX_COUNT - (uint16_t)periodVal;
    /* cppcheck-suppress misra-c2012-8.7 */
    TMR3_CounterSet(timer3ReloadVal);
}

uint32_t TMR3_PeriodGet(void)
{
    return ((uint32_t)TMR3_MAX_COUNT - timer3ReloadVal);
}

uint32_t TMR3_MaxCountGet(void)
{
    return (uint32_t)TMR3_MAX_COUNT;
}

void TMR3_OverflowISR(void)
{
    /* cppcheck-suppress misra-c2012-8.7 */
    T3CONbits.TMR3ON = 0; //automatiski se timer zaustavlja svaki put i onda pokre?e u funkciji setTriacHigh 
    TMR3_CounterSet(timer3ReloadVal); //timer se reloada tako da je spreman slijede?i put kada se pozove start

    // The ticker is set to 1 -> The callback function gets called every time this ISR executes.
    if(NULL != TMR3_OverflowCallback)
    {
        TMR3_OverflowCallback();
    }
    PIR4bits.TMR3IF = 0;
}

void TMR3_OverflowCallbackRegister(void (* CallbackHandler)(void))
{
    TMR3_OverflowCallback = CallbackHandler;
}

static void TMR3_DefaultOverflowCallback(void)
{
    // Default interrupt handler
}
/**
  End of File
*/
