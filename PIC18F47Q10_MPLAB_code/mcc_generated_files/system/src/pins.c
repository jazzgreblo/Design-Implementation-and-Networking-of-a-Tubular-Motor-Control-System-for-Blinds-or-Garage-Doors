#include "../pins.h"
#include "../../../../Diplomski_rad.X/main.h"
#include "../system.h"
#include <stdio.h>

volatile uint8_t IOC_counter = 0;
volatile uint8_t IOC_counter_last_check = 0;
volatile uint16_t Timer0_last_counter = 0;
volatile bool IOC_flag = 0;

void (*IO_RC1_InterruptHandler)(void);

void PIN_MANAGER_Initialize(void)
{
   /**
    LATx registers
    */
    LATA = 0x0;
    LATB = 0x0;
    LATC = 0x0;
    LATD = 0x0;
    LATE = 0x0;
    /**
    ODx registers
    */
    ODCONA = 0x0;
    ODCONB = 0x0;
    ODCONC = 0x0;
    ODCOND = 0x0;
    ODCONE = 0x0;

    /**
    TRISx registers
     * ako je bit == 0 -> taj pin se koristi kao output (izlaz)
     * ako je bit == 1 -> taj pin se koristi kao input (ulaz)  
    */
    TRISA = 0xB;
    TRISB = 0xDF; //RB5 se koristi za INV pin od RS485 isolator clicka stavit na HIGH
    TRISC = 0xBB; // RC1 je input za detekciju struje
                  // RC2 je output od CCP1 (da vidim kad okida)
                  // RC5 je postavljen kao ulaz pin za tipku S1
                  //provjerit jos da li je to tako
                  // RC7 je postavljen kao input pin (koristi se za UART RX) sa TRISB = 0xFF; (sedmi bit je 1)
                  // RC6 je postavljen kao output pin za UART TX (6 bit je 0)
    TRISD = 0xF0; //RD0 i RD1 su outputi za debuging
                  // RD3 triac UP output (za trigerat LED u optotiacu)
                  // RD2 triac DOWN output (za trigerat LED u optotiacu)
    TRISE = 0x7;

    /**
    ANSELx registers
     * ako je bit == 0 -> taj pin se koristi kao digitalni  
     * ako je bit == 1 -> taj pin se koristi kao analogni  
    */
    //mislim da RC6 nije skuzio da treba bit za uart isto
    ANSELA = 0xF;
    ANSELB = 0xCF; // RB4 je postavljen kao digitalni pin za tipku S1
    ANSELC = 0x19; // RC1 se koristi kao digitalni pin za IOC detekciju struje
                   // RC2 se koristi kao digitalni pin za izlaz CCP1 
                   // RC5 je postavljen kao digitalni pin za tipku S1
                   // RB7 je postavljen kao digitalni pin (koristi se za UART RX) sa ANSELB = 0x7F (sedmi bit = 0)
                   // RB6 je postavljen kao digitalni pin (6 bit je 0)
    ANSELD = 0xF0; //RD0 i RD1 su digitalni pinovi za debuging 
                   //RD3 i RD2 su digitalni pinovi za trigerat LED u optotiacu 
    ANSELE = 0x7;

    /**
    WPUx registers
    */
    WPUA = 0x0;
    WPUB = 0x0;
    WPUC = 0x2; //maknut pullup na ovom pinu kad budem spojio bas sklop za detekciju struje jer bi s time mjenjao VL i VH ! -- RADI OVAKO
    WPUD = 0x0;
    WPUE = 0x0;


    /**
    SLRCONx registers
    */
    SLRCONA = 0xFF;
    SLRCONB = 0xFF;
    SLRCONC = 0xFF;
    SLRCOND = 0xFF;
    SLRCONE = 0x7;

    /**
    INLVLx registers
    */
    INLVLA = 0xFF;
    INLVLB = 0xFF;
    INLVLC = 0xFF;
    INLVLD = 0xFF;
    INLVLE = 0xF;

    /**
    RxyI2C | RxyFEAT registers   
    */
    /**
    PPS registers
    */
    RX1PPS = 0x17;  //RC7->EUSART1:RX1;
    RC6PPS = 0x09;  //RC6->EUSART1:TX1;
    
    //staro
    //RX2PPS = 0xF;   //RB7->EUSART2:RX2;
    //RB6PPS = 0x0B;  //RB6->EUSART2:TX2;
    
   /**
    IOCx registers 
    */
    IOCAP = 0x0;
    IOCAN = 0x0;
    IOCAF = 0x0;
    IOCBP = 0x0;
    IOCBN = 0x0;
    IOCBF = 0x0;
    IOCCP = 0x2; //RC1 positive edge detection enabled
    IOCCN = 0x2; //RC1 negative edge detection enabled
    IOCCF = 0x0;
    IOCEP = 0x0;
    IOCEN = 0x0;
    IOCEF = 0x0;

    IO_RC1_SetInterruptHandler(IO_RC1_DefaultInterruptHandler);

    // Enable PIE0bits.IOCIE interrupt 
    PIE0bits.IOCIE = 1; 
}
  
void PIN_MANAGER_IOC(void)
{
    // interrupt on change for pin IO_RC1
    if(IOCCFbits.IOCCF1 == 1)
    {
        IO_RC1_ISR();  
    }
}
   
/**
   IO_RC1 Interrupt Service Routine
*/
void IO_RC1_ISR(void) { 
    volatile uint8_t dummy = PORTC; (void)dummy; // 1) clear mismatch by read
    // procita trenutne vrijednosti svih pinova PORTC; i izbjegne warning da se 'dummy' ne koristi

    uint16_t now = Timer0.CounterGet();          // 2) uzmi vrijeme
    if ((uint16_t)(now - Timer0_last_counter) >= 3000u) { // ~6 ms debounce (2us * 3000)
        IOC_counter++; // uint8_t: autom. overflow 0..255 (ok)
                       // uint16_t: isto automatski overflow msm?
        Timer0_last_counter = now;
    }

    IOCCFbits.IOCCF1 = 0; // 3) clear flag NA KRAJU
}

/**
  Allows selecting an interrupt handler for IO_RC1 at application runtime
*/
void IO_RC1_SetInterruptHandler(void (* InterruptHandler)(void)){
    IO_RC1_InterruptHandler = InterruptHandler;
}

/**
  Default interrupt handler for IO_RC1
*/
void IO_RC1_DefaultInterruptHandler(void){
    // add your IO_RC1 interrupt custom code
    // or set custom function using IO_RC1_SetInterruptHandler()
}
/**
 End of File
*/