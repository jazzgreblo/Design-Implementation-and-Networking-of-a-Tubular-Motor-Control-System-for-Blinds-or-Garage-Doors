/**
 * UART Control Commands - Polled Example Driver File
 * 
 * @file uart_example.c
 * 
 * @addtogroup uart-example
 * 
 * @brief This is the generated example implementation for the UART Control Commands - Polled driver.
 *
 * @version UART Control Commands - Polled Example Version 1.0.0
*/
/*
� [2025] Microchip Technology Inc. and its subsidiaries.

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

/* Use Case 3 - Polled implementation. Copy this code to your project source, e.g., to main.c  */
/* ------------------------------------------------------------------
#include "mcc_generated_files/system/system.h"
#include <string.h>

#define MAX_COMMAND_LEN         (8U)
#define LINEFEED_CHAR           ((uint8_t)'\n')
#define CARRIAGERETURN_CHAR     ((uint8_t)'\r')

static uint8_t command[MAX_COMMAND_LEN];
static uint8_t index = 0;
static uint8_t readMessage;

void UART_ExecuteCommand(char *command);
void UART_ProcessCommand(void);


void UART_ExecuteCommand(char *command)
{
    if(strcmp(command, "ON") == 0)
    {
        IO_LED_SetLow();
        (void)printf("OK, LED ON.\r\n");
    }
    else if (strcmp(command, "OFF") == 0)
    {
        IO_LED_SetHigh();
        (void)printf("OK, LED OFF.\r\n");
    }
    else
    {
        (void)printf("Incorrect command.\r\n");
    }
}

void UART_ProcessCommand(void)
{
    if(UART.IsRxReady())
    {
        readMessage = UART.Read();
        if ( (readMessage != LINEFEED_CHAR) && (readMessage != CARRIAGERETURN_CHAR) ) 
        {
            command[index++] = readMessage;
            if (index > MAX_COMMAND_LEN) 
            {
                (index) = 0;
            }
        }
    
           if (readMessage == CARRIAGERETURN_CHAR) 
           {
                command[index] = '\0';
                index = 0;
                UART_ExecuteCommand(command);
            }
    }
}

int main(void)
{
    SYSTEM_Initialize();
    
    (void)printf("In the terminal, send 'ON' to turn the LED on, and 'OFF' to turn it off.\r\n");
    (void)printf("Note: commands 'ON' and 'OFF' are case sensitive.\r\n");
    
    while(1)
    {
        UART_ProcessCommand();
    }
}
------------------------------------------------------------------ */
/**
 End of File
*/