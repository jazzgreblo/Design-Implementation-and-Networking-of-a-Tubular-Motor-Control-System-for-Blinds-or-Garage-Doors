/**
 * TMR5 Generated Timer Driver API Header File
 * 
 * @file tmr5.h
 * 
 * @ingroup timerdriver
 * 
 * @brief This file contains API prototypes and other data types for the TMR5 Timer driver.
 *
 * @version TMR5 Timer Driver Version 4.0.0
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

#ifndef TMR5_H
#define TMR5_H

#include <stdint.h>
#include <stdbool.h>
#include "timer_interface.h"

/**
 * @misradeviation{@advisory,2.5}
 * MCC Melody drivers provide macros that can be added to an application. 
 * It depends on the application whether a macro is used or not. 
 */

/**
 * @ingroup timerdriver
 * @brief Defines the maximum count value of the timer.
 */
#define TMR5_MAX_COUNT (65535U)
/**
 * @ingroup timerdriver
 * @brief Defines the timer prescaled clock frequency in hertz.
 */
/* cppcheck-suppress misra-c2012-2.5 */
#define TMR5_CLOCK_FREQ (8000000UL)
/**
 * @ingroup timerdriver
 * @brief Defines the Custom Name for the \ref TMR5_MAX_COUNT.
 */
/* cppcheck-suppress misra-c2012-2.5 */
#define TIMER5_MAX_COUNT TMR5_MAX_COUNT
/**
 * @ingroup timerdriver
 * @brief Defines the Custom Name for the \ref TMR5_CLOCK_FREQ.
 */
/* cppcheck-suppress misra-c2012-2.5 */
#define TIMER5_CLOCK_FREQ TMR5_CLOCK_FREQ
/**
 * @ingroup timerdriver
 * @brief Defines the Custom Name for the \ref TMR5_Initialize API.
 */
/* cppcheck-suppress misra-c2012-2.5 */
#define Timer5_Initialize TMR5_Initialize
/**
 * @ingroup timerdriver
 * @brief Defines the Custom Name for the \ref TMR5_Deinitialize API.
 */
/* cppcheck-suppress misra-c2012-2.5 */
#define Timer5_Deinitialize TMR5_Deinitialize
/**
 * @ingroup timerdriver
 * @brief Defines the Custom Name for the \ref TMR5_Start API.
 */
/* cppcheck-suppress misra-c2012-2.5 */
#define Timer5_Start TMR5_Start
/**
 * @ingroup timerdriver
 * @brief Defines the Custom Name for the \ref TMR5_Stop API.
 */
/* cppcheck-suppress misra-c2012-2.5 */
#define Timer5_Stop TMR5_Stop
/**
 * @ingroup timerdriver
 * @brief Defines the Custom Name for the \ref TMR5_CounterGet API.
 */
/* cppcheck-suppress misra-c2012-2.5 */
#define Timer5_CounterGet TMR5_CounterGet
/**
 * @ingroup timerdriver
 * @brief Defines the Custom Name for the \ref TMR5_CounterSet API.
 */
/* cppcheck-suppress misra-c2012-2.5 */
#define Timer5_CounterSet TMR5_CounterSet
/**
 * @ingroup timerdriver
 * @brief Defines the Custom Name for the \ref TMR5_PeriodSet API.
 */
/* cppcheck-suppress misra-c2012-2.5 */
#define Timer5_PeriodSet TMR5_PeriodSet
/**
 * @ingroup timerdriver
 * @brief Defines the Custom Name for the \ref TMR5_PeriodGet API.
 */
/* cppcheck-suppress misra-c2012-2.5 */
#define Timer5_PeriodGet TMR5_PeriodGet
/**
 * @ingroup timerdriver
 * @brief Defines the Custom Name for the \ref TMR5_MaxCountGet API.
 */
/* cppcheck-suppress misra-c2012-2.5 */
#define Timer5_MaxCountGet TMR5_MaxCountGet
/**
 * @ingroup timerdriver
 * @brief Defines the Custom Name for the \ref TMR5_OverflowISR API.
 */
/* cppcheck-suppress misra-c2012-2.5 */
#define Timer5_OverflowISR TMR5_OverflowISR
/**
 * @ingroup timerdriver
 * @brief Defines the Custom Name for the \ref TMR5_OverflowCallbackRegister API.
 */
/* cppcheck-suppress misra-c2012-2.5 */
#define Timer5_OverflowCallbackRegister TMR5_OverflowCallbackRegister

/**
 @ingroup timerdriver
 @struct TIMER_INTERFACE
 @brief Declares an instance of TIMER_INTERFACE for the TMR5 module.
 */
extern const struct TIMER_INTERFACE Timer5;

/**
 * @ingroup timerdriver
 * @brief Initializes the Timer5 (TMR5) module.
 *        This routine must be called before any other TMR5 routines.
 * @param None.
 * @return None.
 */
void TMR5_Initialize(void);

/**
 * @ingroup timerdriver
 * @brief Deinitializes the TMR5 module.
 * @param None.
 * @return None.
 */
void TMR5_Deinitialize(void);

/**
 * @ingroup timerdriver
 * @brief Starts the TMR5 timer.
 * @pre Initialize TMR5 with TMR5_Initialize() before calling this API.
 * @param None.
 * @return None.
 */
void TMR5_Start(void);

/**
 * @ingroup timerdriver
 * @brief Stops the TMR5 timer.
 * @pre Initialize TMR5 with TMR5_Initialize() before calling this API.
 * @param None.
 * @return None.
 */
void TMR5_Stop(void);

/**
 * @ingroup timerdriver
 * @brief Returns the current counter value.
 * @pre Initialize TMR5 with TMR5_Initialize() before calling this API.
 * @param None.
 * @return Counter value from the TMR5 register
 */
uint32_t TMR5_CounterGet(void);

/**
 * @ingroup timerdriver
 * @brief Sets the counter value.
 * @pre Initialize TMR5 with TMR5_Initialize() before calling this API.
 * @param timerVal - Counter value to be written to the TMR5 register
 * @return None.
 */
void TMR5_CounterSet(uint32_t timerVal);

/**
 * @ingroup timerdriver
 * @brief Sets the period count value.
 * @pre Initialize TMR5 with TMR5_Initialize() before calling this API. 
 * @param periodVal - Period count value
 * @return None.
 */
void TMR5_PeriodSet(uint32_t periodVal);

/**
 * @ingroup timerdriver
 * @brief Returns the current period value.
 * @param None.
 * @return Period count value
 */
uint32_t TMR5_PeriodGet(void);

/**
 * @ingroup timerdriver
 * @brief Returns the maximum count value of timer.
 * @param None.
 * @return Maximum count value of timer
 */
uint32_t TMR5_MaxCountGet(void);

/**
 * @ingroup timerdriver
 * @brief Interrupt Service Routine (ISR) for the TMR5 overflow interrupt.
 * @param None.
 * @return None.
 */
void TMR5_OverflowISR(void);

/**
 * @ingroup timerdriver
 * @brief Registers a callback function for the TMR5 overflow event.
 * @param CallbackHandler - Address of the custom callback function
 * @return None.
 */
 void TMR5_OverflowCallbackRegister(void (* CallbackHandler)(void));
#endif // TMR5_H