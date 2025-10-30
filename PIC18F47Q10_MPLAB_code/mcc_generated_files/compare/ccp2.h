/**
 * CCP2 Generated Driver API Header File.
 * 
 * @file ccp2.h
 * 
 * @defgroup compare2 COMPARE2
 * 
 * @brief This file contains the API prototypes and other data types for the CCP2 module.
 *
 * @version CCP2 Driver Version 2.0.3
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

#ifndef CCP2_H
#define CCP2_H

 /**
   Section: Included Files
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>


/** 
   Section: Data Type Definition
*/

/**
 * @ingroup compare2
 * @union CCPR2_PERIOD_REG_T
 * @brief Custom data type to hold the low byte, high byte, and 16-bit values of the period register.
 */
  /**
 * @misradeviation{@advisory,19.2}
 * The CCP register values necessitates to store and accessing the register values within the group byte therefore the use of a union is essential.
 */
 /* cppcheck-suppress misra-c2012-19.2 */
typedef union CCPR2Reg_tag
{
   struct
   {
      uint8_t ccpr2l;
      uint8_t ccpr2h;
   };
   struct
   {
      uint16_t ccpr2_16Bit;
   };
} CCPR2_PERIOD_REG_T ;

/**
  Section: Compare Module APIs
*/

/**
 * @ingroup compare2
 * @brief Initializes the CCP2 module. This is called only once before calling other CCP2 APIs.
 * @param None.
 * @return None.
 */
void CCP2_Initialize(void);

/**
 * @ingroup compare2
 * @brief Sets the 16-bit Compare value.
 * @pre CCP2_Initialize() is already called.
 * @param compareCount - 16-bit unsigned value.
 * @return None.
 */
void CCP2_SetCompareCount(uint16_t compareCount);

/**
 * @ingroup compare2
 * @brief Returns the Compare output status.
 * @pre CCP2_Initialize() is already called.
 * @param None.
 * @retval True - Compare output is high.
 * @retval False - Compare output is low.
 */
bool CCP2_OutputStatusGet(void);

/**
 * @ingroup compare2
 * @brief Implements the Interrupt Service Routine (ISR) for the compare interrupt.
 * @param None.
 * @return None.
 */
void CCP2_CompareISR(void);

#endif //CCP2_H
/**
 End of File
*/