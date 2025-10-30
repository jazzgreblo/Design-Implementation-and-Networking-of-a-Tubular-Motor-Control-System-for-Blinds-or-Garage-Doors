/**
 * CCP2 Generated Driver File.
 * 
 * @file ccp2.c
 * 
 * @ingroup compare2
 * 
 * @brief This file contains the API implementation for the CCP2 driver.
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

 /**
   Section: Included Files
 */

#include <xc.h>
#include "../ccp2.h"

/**
  Section: Compare Module APIs
*/

void CCP2_Initialize(void)
{
    // Set the CCP2 to the options selected in the User Interface

    // CCPM Pulseoutput; EN enabled; FMT right_aligned; 
    CCP2CON = 0x8A;
    
    // CCPRH 0; 
    CCPR2H = 0x0;
    
    // CCPRL 0; 
    CCPR2L = 0x0;
    
    // Selecting Timer 1
    CCPTMRSbits.C2TSEL = 0x1;

    // Clear the CCP2 interrupt flag
    PIR6bits.CCP2IF = 0; 

    // Enable the CCP2 interrupt
    PIE6bits.CCP2IE = 1;    
}

void CCP2_SetCompareCount(uint16_t compareCount)
{
    //mozda promjenit na ono jednostavno zapravo ovo nikad ne zoven tako da je senako
   /**
 * @misradeviation{@advisory,19.2}
 * The CCP register values necessitates to store and accessing the register values within the group byte therefore the use of a union is essential.
 */
 /* cppcheck-suppress misra-c2012-19.2 */
  CCPR2_PERIOD_REG_T module;
    
    // Write the 16-bit compare value
    module.ccpr2_16Bit = compareCount;
    
    CCPR2L = module.ccpr2l;
    CCPR2H = module.ccpr2h;
}
bool CCP2_OutputStatusGet(void)
{
    // Returns the output status
    return(CCP2CONbits.OUT);
}

//u mainu funkcija
//void CCP2_CompareISR(void)
//{
//    // Clear the CCP2 interrupt flag
//    PIR6bits.CCP2IF = 0;
//    
//    // Add user code here
//}

/**
 End of File
*/
