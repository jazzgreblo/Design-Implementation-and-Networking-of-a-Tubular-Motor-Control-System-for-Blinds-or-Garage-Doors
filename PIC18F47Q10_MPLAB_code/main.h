/* 
 * File:   main.h
 * Author: jazzg
 *
 * Created on May 14, 2025, 6:47 PM
 */

#ifndef MAIN_H
#define	MAIN_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>    

// Set a bit
#define SET_BIT(var, bit)     ((var) |= (1U << (bit)))

// Clear a bit
#define CLEAR_BIT(var, bit)   ((var) &= ~(1U << (bit)))

// Toggle a bit
#define TOGGLE_BIT(var, bit)  ((var) ^= (1U << (bit)))
    
// Test a bit 
#define TEST_BIT(var, bit)    (((var) >> (bit)) & 1U)
    
    
// Set one or more bits
#define SET_MASK(var, mask)         ((var) |= (mask)) 

// Clear one or more bits
#define CLEAR_MASK(var, mask)       ((var) &= ~(mask))

// Toggle one or more bits
#define TOGGLE_MASK(var, mask)      ((var) ^= (mask))
    
// Check if any bit(s) are set - it returns 1 if any bit in mask is set in var, even if others are not
#define TEST_MASK(var, mask)        (((var) & (mask)) != 0)
    
// Set a certain state and clear all others
#define SET_STATE(var, mask)       ((var) = (mask))
// To change or test multiple bits use | in between multiple masks you want to set/clear/toggle/test

// Motor operations
extern volatile unsigned char SystemState;
extern volatile unsigned char DesiredSystemState;

#define MOTOR_RUNNING       0

#define TOP_POSITION        1
#define BOTTOM_POSITION     2

#define CURRENT_DETECTED    3

#define GOING_UP            4
#define GOING_DOWN          5
#define SOFT_START          6
#define SOFT_STOP           7

#define SOFT_START_UP       0b01010000
#define SOFT_STOP_UP        0b10010000
#define SOFT_START_DOWN     0b01100000
#define SOFT_STOP_DOWN      0b10100000
// Motor operations  

// Input messages
extern volatile unsigned char InputCommand;

#define ROLLER_STOP        0b111111
#define ROLLER_UP          0b101010
#define ROLLER_DOWN        0b010101
#define GET_DEBUG_INFO     0b111000
// Input messages

extern volatile unsigned char errorStates;

#define UNABLE_TO_START 0 //when there is no current after tying to go up AND/OR down 

extern volatile bool zcdTriggered;

// ====== Function Prototypes ======
void UART_ExecuteCommand(char *command);
void UART_ProcessCommand(void);
void inputManager(void);
void controlManager(void);
void checkCurrentDetection(void);
void setTriacHigh(void);
void setTriacLow (void);
void noPhaseAngleControl(void);
void phaseAngleControl(void);
void softStart(void);
void softStop(void);
void softStop_Than_softStart(void);
void ZCD_Handler(void);
void CCP1_CompareISR(void);


#ifdef	__cplusplus
}
#endif

#endif	/* MAIN_H */
