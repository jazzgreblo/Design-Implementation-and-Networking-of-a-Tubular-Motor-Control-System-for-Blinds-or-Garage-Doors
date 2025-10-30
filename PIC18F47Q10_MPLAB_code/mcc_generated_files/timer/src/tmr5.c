#include <xc.h>
#include "../tmr5.h"

const struct TIMER_INTERFACE Timer5 = {
    .Initialize = TMR5_Initialize,
    .Deinitialize = TMR5_Deinitialize,
    .Start = TMR5_Start,
    .Stop = TMR5_Stop,
    .PeriodSet = TMR5_PeriodSet,
    .PeriodGet = TMR5_PeriodGet,
    .CounterGet = TMR5_CounterGet,
    .CounterSet = TMR5_CounterSet,
    .MaxCountGet = TMR5_MaxCountGet,
    .TimeoutCallbackRegister = TMR5_OverflowCallbackRegister,
    .Tasks = NULL
};

static volatile uint16_t timer5ReloadVal;
static void TMR5_DefaultOverflowCallback(void);
static void (*TMR5_OverflowCallback)(void);

void TMR5_Initialize(void)
{
    T5CONbits.TMR5ON = 0;             // TMRON disabled

    T5GCON = (0 << _T5GCON_T5GGO_POSN)   // T5GGO done
        | (0 << _T5GCON_T5GSPM_POSN)   // T5GSPM disabled
        | (0 << _T5GCON_T5GTM_POSN)   // T5GTM disabled
        | (0 << _T5GCON_T5GPOL_POSN)   // T5GPOL low
        | (0 << _T5GCON_T5GE_POSN);  // T5GE disabled

    T5GATE = (0 << _T5GATE_GSS_POSN);  // GSS T5G_pin

    T5CLK = (3 << _T5CLK_CS_POSN);  // CS HFINTOSC

    TMR5H = 0xA2;              // period = 3ms (24000tics -> overflow value = 656535-24000 = 41535) Timer clock 8000000 Hz
    TMR5L = 0x3F;

    timer5ReloadVal=((uint16_t)TMR5H << 8) | TMR5L;
    TMR5_OverflowCallback = TMR5_DefaultOverflowCallback;

    PIR4bits.TMR5IF = 0;   
    PIE4bits.TMR5IE = 1;
  
    T5CON = (0 << _T5CON_TMR5ON_POSN)   // TMR5ON disabled jer cu ga palit na CMP_ISR
        | (1 << _T5CON_T5RD16_POSN)   // T5RD16 enabled
        | (1 << _T5CON_nT5SYNC_POSN)   // nT5SYNC do_not_synchronize
        | (3 << _T5CON_CKPS_POSN);  // CKPS 1:8
}

void TMR5_Deinitialize(void)
{
    T5CONbits.TMR5ON = 0;             // TMRON disabled
    
    T5CON = 0x0;
    T5GCON = 0x0;
    T5GATE = 0x0;
    T5CLK = 0x0;
    TMR5H = 0x0;
    TMR5L = 0x0;

    PIR4bits.TMR5IF = 0;
    PIE4bits.TMR5IE = 0;

    PIR5bits.TMR5GIF = 0;
    PIE5bits.TMR5GIE = 0;
}

void TMR5_Start(void)
{   
    T5CONbits.TMR5ON = 1;
}

void TMR5_Stop(void)
{ 
    T5CONbits.TMR5ON = 0;
}

uint32_t TMR5_CounterGet(void)
{
    uint16_t readVal;
    uint8_t readValHigh;
    uint8_t readValLow;
    	
    readValLow = TMR5L;
    readValHigh = TMR5H;
    
    readVal = ((uint16_t)readValHigh << 8) | readValLow;

    return (uint32_t)readVal;
}

void TMR5_CounterSet(uint32_t timerVal)
{
    if(1U == T5CONbits.nT5SYNC)
    {
        bool onState = T5CONbits.TMR5ON;

        T5CONbits.TMR5ON = 0;      
        TMR5H = (uint8_t)(timerVal >> 8);
        TMR5L = (uint8_t)timerVal;       
        T5CONbits.TMR5ON = onState;
    }
    else
    {      
        TMR5H = (uint8_t)(timerVal >> 8);
        TMR5L = (uint8_t)timerVal;
    }
}

void TMR5_PeriodSet(uint32_t periodVal)
{
    timer5ReloadVal =  TMR5_MAX_COUNT - (uint16_t)periodVal;
    /* cppcheck-suppress misra-c2012-8.7 */
    TMR5_CounterSet(timer5ReloadVal);
}

uint32_t TMR5_PeriodGet(void)
{
    return ((uint32_t)TMR5_MAX_COUNT - timer5ReloadVal);
}

uint32_t TMR5_MaxCountGet(void)
{
    return (uint32_t)TMR5_MAX_COUNT;
}

void TMR5_OverflowISR(void)
{
    /* cppcheck-suppress misra-c2012-8.7 */
    TMR5_CounterSet(timer5ReloadVal);
    
    //stop timer5 - pokrece se opet nakon slijedeceg CMP1_ISR
    T5CONbits.TMR5ON = 0;
    
    // The ticker is set to 1 -> The callback function gets called every time this ISR executes.
    if(NULL != TMR5_OverflowCallback)
    {
        TMR5_OverflowCallback();
    }
    PIR4bits.TMR5IF = 0;
}

void TMR5_OverflowCallbackRegister(void (* CallbackHandler)(void))
{
    TMR5_OverflowCallback = CallbackHandler;
}

static void TMR5_DefaultOverflowCallback(void)
{
    // Default interrupt handler
}
/**
  End of File
*/
