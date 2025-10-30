#include "mcc_generated_files/system/system.h"
#include "main.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pic18f47q10.h>
#include <inttypes.h>

#define MAX_COMMAND_LEN         (8U)
#define LINEFEED_CHAR           ((uint8_t)'\n')
#define CARRIAGERETURN_CHAR     ((uint8_t)'\r')

#define MAX_COUNTER_DELAY 39000u    // maksimalano kasnjenje kuta okidanja trijaka, f_osc_timer1 = 4MHz, 
                                    //T_osc_timer1 = 250ns, period/2 =10ms -- 10ms/250ns = 40000
                                    //ALI san oduzeo 800(200us) zbog toga jer trijaku gate mora neko vrijeme bit na 
                                    //na 0 da on ne provede u slijede?oj poluperiodi
                                    //to je za unutarnji integrirani trijak

// MSB 2 bita = adresa, mora biti 01 
#define UART_ADDR_MASK   0xC0u   // bit7..6
#define UART_ADDR        0x01u   // 
#define UART_CMD_MASK    0x3Fu   // donjih 6 bitova

// 6-bit naredbe (moraju odgovarati PC-u)
#define CMD_STOP   0b111111
#define CMD_UP     0b101010
#define CMD_DOWN   0b010101
#define CMD_DBG    0b111000

// 6-bit status stanja za slanje prema PC-u
#define CMD_SOFT_STOP_START  5
#define CMD_BOTTOM_POS       4
#define CMD_TOP_POS          3
#define CMD_GOING_UP         2
#define CMD_GOING_DOWN       1
#define CMD_CURRENT          0

volatile unsigned char SystemState = 0;
volatile unsigned char DesiredSystemState = 0;

volatile unsigned char errorStates = 0;

volatile unsigned char statusState = (UART_ADDR << 6); // za slanje prema PC-u

volatile uint8_t prev_current = 0;

volatile bool currentDetected_flag = false; //for calling controlManager if change has happened
volatile uint8_t direction = 0; // 1 = UP, 2 = DOWN 
volatile bool debug_flag = false;
volatile bool current_flag = false;

//volatile uint16_t last_tmr5 = 0; //za ZCD debouncer
//volatile uint16_t now_tmr5 = 0; //za ZCD debouncer
//ovo ne koristim vise

volatile unsigned char InputCommand = 0;
volatile unsigned char LastCommand = 0;

volatile bool softStop_over_flag = 0;

//UART variables
static char command[MAX_COMMAND_LEN]; //8 baytes fit in this array
static uint8_t index = 0;
static uint8_t readMessage;

//Soft start & ZCR variables
volatile bool zcdTriggered = 0;
volatile uint16_t delayCounter = MAX_COUNTER_DELAY;                // vrijednost delaya za slanje impulsa na trijak, tu je stavljena po?etna 
//At a phase control angle close to 180° the driver?s turn on pulse at the trailing edge of the AC sine wave must
//be limited to end 200us before AC zero cross as shown in - MOC3073M datasheet 

uint16_t counterStep = 780; //vrijemeKontroleFaznogKuta /period;  // Step to decrease/increase delay
//mora bit djeljivo sa MAX_COUNTER_STEP

/* Po?alji ASCII "0"/"1" bitove MSB?LSB za zadani bajt */
static void sendBits(uint8_t value)
{
    for (int8_t i = 7; i >= 0; i--)
    {
        uint8_t bit = (value >> i) & 0x01;
        EUSART1_Write(bit ? '1' : '0');
    }
}

void UART_SendBinary(const char *text, void* value, uint8_t is_uint16){
    // Print the variable name (text)
    while (*text) {
        EUSART1_Write(*text++);
    }

    // Print colon and space
    EUSART1_Write(':');
    EUSART1_Write(' ');

    // Determine the type based on is_uint16 flag
    if (is_uint16) {
        uint16_t val = *((uint16_t*)value);  // Cast to uint16_t
        for (int i = 15; i >= 0; i--) {  // 16 bits for uint16_t
            EUSART1_Write((val & (1 << i)) ? '1' : '0');
        }
    } else {
        uint8_t val = *((uint8_t*)value);  // Cast to uint8_t
        for (int i = 7; i >= 0; i--) {  // 8 bits for uint8_t
            EUSART1_Write((val & (1 << i)) ? '1' : '0');
        }
    }

    // Optional newline
    EUSART1_Write('\r');
    EUSART1_Write('\n');
}

void UART_ProcessCommand(void)
{
    // Pro?itaj jedan bajt iz kru?nog buffera (ISR ga je ve? napunio)
    uint8_t b = EUSART1_Read();

    // Provjera adrese u MSB bitovima (01xxxxxx)
    if ((b >> 6) != UART_ADDR) {
        return; // nije za ovaj ure?aj
    }
    
    LastCommand = InputCommand; 
    
    // Izdvoji 6-bit komandu
    uint8_t cmd6 = (b & UART_CMD_MASK);

    if(cmd6 == CMD_STOP) { 
        InputCommand = ROLLER_STOP; 
    }else if (cmd6 == CMD_UP) { 
        InputCommand = ROLLER_UP;
    }else if (cmd6 == CMD_DOWN) { 
        InputCommand = ROLLER_DOWN; 
    }else if (cmd6 == CMD_DBG)  {
//        UART_SendBinary("DesiredSystemState",(void*)&DesiredSystemState,0);
//        UART_SendBinary("SystemState",(void*)&SystemState,0); 
//         za ovo koristit treba u eusart1 promijenit RX buffer size na 64
//         i python promijenit jer sada hvata 8 bitova i printa ih na odre?eni na?in i azurira ledice
        sendBits(100);
        return;
    }else{
        return;
    }
    
    if (LastCommand == InputCommand){
        //(void)printf("Repeated command.\r\n");
        return; //exit ExecuteCommand function and don't run inputManager because the input same as the current input command
    }
    
    inputManager();
}

//ne koristim -staro
void UART_ExecuteCommand_old(char *command){
    
    if (command == NULL) return;
    
    LastCommand = InputCommand;
    
    if(strcmp(command, "0") == 0){
        InputCommand = ROLLER_STOP; // 0 = Stop command 
        //IO_D2_SetHigh(); 
    }else if (strcmp(command, "1") == 0){
        InputCommand = ROLLER_UP; // 1 = UP command
        //IO_D3_SetHigh();
    }else if (strcmp(command, "2") == 0){
        InputCommand = ROLLER_DOWN;  // 2 = DOWN command
        //IO_D4_SetHigh();
    }else if (strcmp(command, "3") == 0){ //3 = GET_DEBUG_INFO command
        //IO_D2_SetHigh();
        UART_SendBinary("DesiredSystemState",(void*)&DesiredSystemState,0);
        UART_SendBinary("SystemState",(void*)&SystemState,0);
        return;
    }else{
        //(void)printf("Incorrect command.\r\n");
        return; //exit ExecuteCommand function and don't run inputManager because the input is not valid
    }
    if (LastCommand == InputCommand){
        //(void)printf("Repeated command.\r\n");
        return; //exit ExecuteCommand function and don't run inputManager because the input same as the current input command
    }
    inputManager();
}

//ne koristim -staro
void UART_ProcessCommand_old(void){
    //IO_D3_Toggle();
    if(UART.IsRxReady()) {
        readMessage = UART.Read();
        if ( (readMessage != LINEFEED_CHAR) && (readMessage != CARRIAGERETURN_CHAR) ) {
            command[index++] = readMessage;
            if (index > MAX_COMMAND_LEN) {
                (index) = 0;
                
            }
        }
           if (readMessage == CARRIAGERETURN_CHAR) {
                command[index] = '\0';
                index = 0;
                UART_ExecuteCommand(command);
                //IO_D5_Toggle();
            }
    }
}

void statusUpdate(void){
    if((TEST_BIT(SystemState, SOFT_START)) || (TEST_BIT(SystemState, SOFT_START))){
        SET_BIT(statusState, CMD_SOFT_STOP_START);
    }else{
        CLEAR_BIT(statusState, CMD_SOFT_STOP_START);
    } // BIT 5
    if(TEST_BIT(SystemState, BOTTOM_POSITION)){
        SET_BIT(statusState, CMD_BOTTOM_POS);
    }else{
        CLEAR_BIT(statusState, CMD_BOTTOM_POS);
    }//BIT 4
    if(TEST_BIT(SystemState, TOP_POSITION)){
        SET_BIT(statusState, CMD_TOP_POS);
    }else{
        CLEAR_BIT(statusState, CMD_TOP_POS);
    } //BIT 3
    if(TEST_BIT(SystemState, GOING_UP)){
        SET_BIT(statusState, CMD_GOING_UP);
    }else{
        CLEAR_BIT(statusState, CMD_GOING_UP);
    } //BIT 2
    if(TEST_BIT(SystemState, GOING_DOWN)){
        SET_BIT(statusState, CMD_GOING_DOWN);
    }else{
        CLEAR_BIT(statusState, CMD_GOING_DOWN);
    } //BIT 1
    if(TEST_BIT(SystemState, CURRENT_DETECTED)){
        SET_BIT(statusState, CMD_CURRENT);
    }else{
        CLEAR_BIT(statusState, CMD_CURRENT);
    } //BIT 0
}

static inline void errorManager(void){
    if(TEST_BIT(SystemState, TOP_POSITION) && TEST_BIT(SystemState, BOTTOM_POSITION)){
        SET_BIT(errorStates, UNABLE_TO_START);
    }
}

void errorHandler(void){
    if(TEST_BIT(errorStates, UNABLE_TO_START)){
        printf("Unable to start");
        CLEAR_BIT(errorStates, UNABLE_TO_START); 
        CLEAR_BIT(SystemState, BOTTOM_POSITION); 
        CLEAR_BIT(SystemState, TOP_POSITION); 
    }
}

void inputManager(void){
    if(InputCommand == ROLLER_STOP){
        CLEAR_BIT(DesiredSystemState, MOTOR_RUNNING);
        CLEAR_BIT(DesiredSystemState, GOING_UP);
        CLEAR_BIT(DesiredSystemState, GOING_DOWN);
    }
    if((InputCommand == ROLLER_UP) && !(TEST_BIT(SystemState, TOP_POSITION))){ //tu je uvijet da ako stisnemo gore 
                                                                               // i vec smo u top position nece ni mijenjat zeljena stanja
        SET_BIT(DesiredSystemState, MOTOR_RUNNING);
        SET_BIT(DesiredSystemState, GOING_UP);
        CLEAR_BIT(DesiredSystemState, GOING_DOWN);
        //CLEAR_BIT(DesiredSystemState, SOFT_STOP);
    }
    if(InputCommand == ROLLER_DOWN && !(TEST_BIT(SystemState, BOTTOM_POSITION))){
        SET_BIT(DesiredSystemState, MOTOR_RUNNING);
        SET_BIT(DesiredSystemState, GOING_DOWN);
        CLEAR_BIT(DesiredSystemState, GOING_UP);
        //CLEAR_BIT(DesiredSystemState, SOFT_STOP);
    }
    
    //check for errors
    sendBits(statusState);
    controlManager();
}

// Returns true if the motor is running,
// when the desired and actual direction is the same (up or down),
// and both states indicate we're in SOFT_STOP ? i.e., time to switch to SOFT_START.
bool controlStopStartChange(void)
{
    bool running =
        TEST_BIT(DesiredSystemState, MOTOR_RUNNING) &&
        TEST_BIT(SystemState,        MOTOR_RUNNING);

    bool same_dir =
        (TEST_BIT(DesiredSystemState, GOING_UP)   && TEST_BIT(SystemState, GOING_UP)) ||
        (TEST_BIT(DesiredSystemState, GOING_DOWN) && TEST_BIT(SystemState, GOING_DOWN));

    bool system_in_soft_stop =
        //TEST_BIT(DesiredSystemState, SOFT_STOP) &&
        TEST_BIT(SystemState,        SOFT_STOP);

    return running && same_dir && system_in_soft_stop;
}

//Returns true when motor is running 
//and desired direction and actual is not the same
//and we are in softStart - time to switch to softStart
bool controlMismatchDirectionSoftStart(void)
{
    bool running =
        TEST_BIT(DesiredSystemState, MOTOR_RUNNING) &&
        TEST_BIT(SystemState,        MOTOR_RUNNING);

    bool not_same_dir =
        (TEST_BIT(DesiredSystemState, GOING_UP)   && TEST_BIT(SystemState, GOING_DOWN)) ||
        (TEST_BIT(DesiredSystemState, GOING_DOWN) && TEST_BIT(SystemState, GOING_UP));
    
    bool in_soft_start =
        TEST_BIT(DesiredSystemState, SOFT_START) &&
        TEST_BIT(SystemState,        SOFT_START);

    return running && not_same_dir && in_soft_start;
}

void controlManager(void){
    if((!TEST_BIT(DesiredSystemState, MOTOR_RUNNING)) && TEST_BIT(SystemState, MOTOR_RUNNING)){ //i ako zelimo da motor ne radi i trenutno radi
        CLEAR_BIT(DesiredSystemState, SOFT_START); //3. 
        SET_BIT(DesiredSystemState, SOFT_STOP); //3. 
        
    }else if(!(TEST_BIT(SystemState, CURRENT_DETECTED))){ //ako nema struje 
        if(TEST_BIT(DesiredSystemState, MOTOR_RUNNING) && (!TEST_BIT(SystemState, MOTOR_RUNNING))
                && TEST_BIT(DesiredSystemState, GOING_UP) && (!TEST_BIT(SystemState, TOP_POSITION))){ //i ako zelimo da motor radi i trenutno ne radi
            SET_BIT(DesiredSystemState, SOFT_START); //1.
            
        }else if(TEST_BIT(DesiredSystemState, MOTOR_RUNNING) && (!TEST_BIT(SystemState, MOTOR_RUNNING))
                && TEST_BIT(DesiredSystemState, GOING_DOWN) && (!TEST_BIT(SystemState, BOTTOM_POSITION))){ //i ako zelimo da motor radi i trenutno ne radi
            SET_BIT(DesiredSystemState, SOFT_START); //2.
        }else if(controlStopStartChange()){
            //kad sam krenuo u soft_stop ali se predomislim i kliknem opet gore npr TREBAT ce napravit i za kontra kad idem soft stop npr up i onda 
            // kliknem da idem gore dok sam vec bio u soft stopu
            CLEAR_BIT(DesiredSystemState, SOFT_STOP); 
            //CLEAR_BIT(SystemState, SOFT_STOP);
            SET_BIT(DesiredSystemState, SOFT_START);     
          
                //ovo malo provjerit nisan siguran dal je okej
                // i tu san stao za opis u diplomskom
            
        }else if(controlMismatchDirectionSoftStart()){
            //kad mijenjam smijer dok sam u softStartu i nema struje
            CLEAR_BIT(DesiredSystemState, SOFT_START);             
            SET_BIT(DesiredSystemState, SOFT_STOP);
            CLEAR_BIT(SystemState, SOFT_START); 
        
        //jedna ideja ako ovo bude zezalo da se nekad pali i gasi currentDetected bit
        //dodati jos jedan else if granu - kad stuje nema neko vrijeme da se izbjegne slucajni ulazak u top ili botom position 
        //- u kojem palimo timer nakon sta nestane struje i nakon nez 200ms ako nije se oper upalila stuja onda tek stavljamo zastavicu
        //koju pregledavamo u tom zadnjem else if uvjetu i ove 6. i 7. tamo stavljam 
        
        }else if((TEST_BIT(DesiredSystemState, MOTOR_RUNNING)) && (TEST_BIT(SystemState, MOTOR_RUNNING))  //6.
            && (TEST_BIT(DesiredSystemState, GOING_UP) && (TEST_BIT(SystemState, GOING_UP)))
                && (!TEST_BIT(DesiredSystemState, SOFT_START)) && (!TEST_BIT(SystemState, SOFT_START))){
            SET_BIT(SystemState, TOP_POSITION); //ako smo htijeli pokrenut motor i poceli ga u soft startu (jer se tamo posavlja MOTOR_RUNNING za systemState)
                                                // i ako nema struje znaci da ga nismo uspjeli pokrenut i da je u gornjoj ili donjoj poziciji
            delayCounter = MAX_COUNTER_DELAY; // jer cemo morat u soft_start nakon sta motor stane gore ili dole 
            CLEAR_BIT(SystemState, MOTOR_RUNNING);
            CLEAR_BIT(DesiredSystemState, MOTOR_RUNNING);
            CLEAR_BIT(SystemState, GOING_UP);
            CLEAR_BIT(DesiredSystemState, GOING_UP);
        }else if((TEST_BIT(DesiredSystemState, MOTOR_RUNNING)) && (TEST_BIT(SystemState, MOTOR_RUNNING))  //7.
            && (TEST_BIT(DesiredSystemState, GOING_DOWN) && (TEST_BIT(SystemState, GOING_DOWN)))
                && (!TEST_BIT(DesiredSystemState, SOFT_START)) && (!TEST_BIT(SystemState, SOFT_START))){
            SET_BIT(SystemState, BOTTOM_POSITION); //ako smo htijeli pokrenut motor i poceli ga u soft startu (jer se tamo posavlja MOTOR_RUNNING za systemState)
                                                   // i ako nema struje znaci da ga nismo uspjeli pokrenut i da je u gornjoj ili donjoj poziciji
            delayCounter = MAX_COUNTER_DELAY; // jer cemo morat u soft_start nakon sta motor stane gore ili dole
            CLEAR_BIT(SystemState, MOTOR_RUNNING);
            CLEAR_BIT(DesiredSystemState, MOTOR_RUNNING);
            CLEAR_BIT(SystemState, GOING_DOWN);
            CLEAR_BIT(DesiredSystemState, GOING_DOWN);
        }

    }else if(TEST_BIT(SystemState, CURRENT_DETECTED)){    //ako ima struje
        if((TEST_BIT(DesiredSystemState, MOTOR_RUNNING)) && (TEST_BIT(SystemState, MOTOR_RUNNING)) //4. i 5. - mismatch of desired and actual direction
                && ( ((TEST_BIT(DesiredSystemState, GOING_UP)) && (TEST_BIT(SystemState, GOING_DOWN))) 
                || ((TEST_BIT(DesiredSystemState, GOING_DOWN)) && (TEST_BIT(SystemState, GOING_UP))) ) ){
            SET_BIT(DesiredSystemState, SOFT_STOP);
            CLEAR_BIT(DesiredSystemState, SOFT_START);
            CLEAR_BIT(SystemState, SOFT_START);
        }else if(controlStopStartChange()){ //10. i 11.
            //kad sam krenuo u soft_stop ali se predomislim i kliknem opet gore npr TREBAT ce napravit i za kontra kad idem soft stop npr up i onda 
            // kliknem da idem gore dok sam vec bio u soft stopu
            CLEAR_BIT(DesiredSystemState, SOFT_STOP); 
            SET_BIT(DesiredSystemState, SOFT_START);
            CLEAR_BIT(SystemState, SOFT_STOP);
        }else if(softStop_over_flag){
            softStop_over_flag = 0;
            SET_BIT(DesiredSystemState, SOFT_START);
        }
        if((TEST_BIT(DesiredSystemState, MOTOR_RUNNING)) && (TEST_BIT(SystemState, MOTOR_RUNNING)) //9.1
               && TEST_BIT(DesiredSystemState, GOING_UP) && TEST_BIT(SystemState, GOING_UP)){
            CLEAR_BIT(SystemState, BOTTOM_POSITION);
        }
        if((TEST_BIT(DesiredSystemState, MOTOR_RUNNING)) && (TEST_BIT(SystemState, MOTOR_RUNNING)) //8.1
               && TEST_BIT(DesiredSystemState, GOING_DOWN) && TEST_BIT(SystemState, GOING_DOWN)){
            CLEAR_BIT(SystemState, TOP_POSITION);
        }
    }
    errorManager();

}


static inline void CCP2_ArmOneShot(uint16_t ticks){
    //static inline = ?A tiny, private helper function in this file that the 
    //compiler should paste directly into where it?s used, instead of making a real function call.?
               
    
    // Never schedule 0 ticks (some PICs won?t match at 0)
    uint16_t c = (ticks == 0u) ? 1u : ticks;
  
    // Stop & reset TMR1
    Timer1.Stop();
    Timer1.CounterSet(0x0000);

    // Write compare: HIGH then LOW
    CCPR2H = (uint8_t)(c >> 8);
    CCPR2L = (uint8_t)(c & 0xFF);

    // Arm interrupt cleanly
    PIR6bits.CCP2IF = 0;   // clear any stale flag
    PIE6bits.CCP2IE = 1;   // enable for this one shot
  
    // Go
    Timer1.Start();
}

void ZCD_Handler(void){
    //IO_D5_Toggle();
    if(TEST_BIT(DesiredSystemState, SOFT_STOP)){
        //IO_D3_Toggle();
        SET_BIT(SystemState, SOFT_STOP);  

        if(delayCounter > MAX_COUNTER_DELAY + 10000){ //ovih 10000 san prozvoljno uzeo
            delayCounter = counterStep;
        } //to je za ako se desi underflow (0-step = 65635-step)
        
        if (delayCounter < MAX_COUNTER_DELAY){ 
            delayCounter += counterStep;
            CCP2_ArmOneShot(delayCounter); // tu nutra san stavio jer kad je npr counter = step onda bi 2 puta na isti delay opalilo, ovako ne
            //IO_D4_Toggle();
        }else{
            CLEAR_BIT(DesiredSystemState, SOFT_STOP);
            CLEAR_BIT(SystemState, SOFT_STOP);
            CLEAR_BIT(SystemState, MOTOR_RUNNING);
            CLEAR_BIT(SystemState, GOING_UP);
            CLEAR_BIT(SystemState, GOING_DOWN); 
            softStop_over_flag = 1;
            controlManager();
        }
        
    }else if(TEST_BIT(DesiredSystemState, SOFT_START)){
        //IO_D4_Toggle();
        SET_BIT(SystemState, SOFT_START);
        if (delayCounter > MAX_COUNTER_DELAY) delayCounter = MAX_COUNTER_DELAY; //U SLUCaju da smo nekkao presli max odnosno 10ms
        
        if (delayCounter > counterStep){
            //printf("%u\n", delayCounter);
            delayCounter -= counterStep;
            CCP2_ArmOneShot(delayCounter); // tu nutra san stavio jer kad je npr counter = step onda bi 2 puta na isti delay opalilo, ovako ne
            //IO_D4_Toggle(); //100
        }else{
            //IO_D5_SetHigh();
            CLEAR_BIT(DesiredSystemState, SOFT_START);
            CLEAR_BIT(SystemState, SOFT_START);
            CCP2_ArmOneShot(counterStep); //ovaj zadnji put inace ne okine pa imam prekid u naponu jednoj periodi
            controlManager();
        }
    }else if(TEST_BIT(DesiredSystemState, MOTOR_RUNNING)){
        CCP2_ArmOneShot(1600); // 800 znaci 250us 960 je 0.3ms 1600 je 0.5ms u praksi ispadne 0.3ms delay jer ZCD nije tocan za tih cca 0.2ms u prosjeku
    }else {
//        CLEAR_BIT(SystemState, SOFT_START);
//        CLEAR_BIT(SystemState, SOFT_STOP);
    }

    sendBits(statusState); //svakih 10ms ce slat poruku (slanje traje 20us sta je dosta brzo)
}

void CCP2_CompareISR(void){
    
    PIE6bits.CCP2IE = 0;   // mask first (prevents re-entry)
    PIR6bits.CCP2IF = 0;   // clear the CCP2 interrupt flag 

    Timer1.Stop();
    Timer1.CounterSet(0x0000);
    
    // Start the 80 µs gate pulse (Timer3 does the width, ISR will end it)
    setTriacHigh();
}

void setTriacHigh(void) {
    direction = 0; //zasto postoji ovaj dumbus: jer kad damo naredbu stop jedino sta se postavi je da zelimo uc u soft stop i u desiredSyState 
                   //ne zelimo ic ni UP ni DOWN pa moramo gledat u systemState koji je bio zadnji smjer pa prema tome odredimo koji triac
                   //ce se upravljati po soft stopu
    if (TEST_BIT(SystemState, SOFT_STOP)) {
        if (TEST_BIT(SystemState, GOING_UP)) {
            direction = 1;
        } else if (TEST_BIT(SystemState, GOING_DOWN)) {
            direction = 2;
        }
    } else {
        if (TEST_BIT(DesiredSystemState, GOING_UP)) {
            direction = 1;
        } else if (TEST_BIT(DesiredSystemState, GOING_DOWN)) {
            direction = 2;
        }
    }

    if (direction == 1) {
        // fire triac no.1
        SET_BIT(SystemState, MOTOR_RUNNING);
        SET_BIT(SystemState, GOING_UP);
        CLEAR_BIT(SystemState, GOING_DOWN);
        //IO_D3_SetHigh();
        IO_RD3_SetHigh();
        Timer3.Start();
    } else if (direction == 2) {
        // fire triac no.2
        SET_BIT(SystemState, MOTOR_RUNNING);
        SET_BIT(SystemState, GOING_DOWN);
        CLEAR_BIT(SystemState, GOING_UP);
        //IO_D2_SetHigh();
        IO_RD2_SetHigh();
        Timer3.Start();
    }
}//after 80us timer is stopped and reloaded (in TMR3_ISR) and setTriacLow is called 

void setTriacLow (void){
    //stavim oba na 0 jer nikad ne upravljam paralelno sa 2 nego uvijek samo resetiram tu
    
    //reset triac no.1
    //IO_D3_SetLow();
    IO_RD3_SetLow();
    
    //reset triac no.2
    //IO_D2_SetLow();
    IO_RD2_SetLow();
    
    Timer3.Stop();              //zaustavim timer i postavim ga opet na tih 80us za drugi put kad se pokrene
    Timer3.CounterSet(0xEBFF);
}

void checkCurrentDetection(void)
{
    //statusSend();
    
    static uint8_t prev_sample = 0; // zadnja ocitana vrijednost brojaca
    uint8_t sample, delta;

    // snapshot IOC_counter (uint8_t je atomski, ne treba disable GIE)
    sample = IOC_counter;
    
    // unsigned aritmetika radi modulo 2^N (ovdje N=8 ? mod 256).
    // (uint8_t)(now - prev) daje tocan broj koraka cak i preko overflowa.
    // Primjer: prev=250, now=5 ? delta = (5 - 250) mod 256 = 11
    // (-245) mod 256 = 11 (two's complement).

    // izracun delta mod 256 (unsigned aritmetika automatski radi modulo)
    delta = (uint8_t)(sample - prev_sample);
    prev_sample = sample; // spremi za iduci put

    // provjera praga za detekciju
    if (delta >= 5u) { // >=5 doga?aja u 100 ms
        SET_BIT(SystemState, CURRENT_DETECTED);
        IO_D4_SetHigh();
    } else {
        CLEAR_BIT(SystemState, CURRENT_DETECTED);
        IO_D4_SetLow();
    }

    // reagiraj samo na promjenu stanja
    static uint8_t last_flag = 0;
    uint8_t cur_flag = TEST_BIT(SystemState, CURRENT_DETECTED) ? 1 : 0;
    if (last_flag != cur_flag) {
        last_flag = cur_flag;
        controlManager();
        statusUpdate(); //svaki put kada se promijeni vrijednost struje IMA/NEMA
        sendBits(statusState);
        //current_flag = 1;
    }
}

//void toggle(void){
//    IO_D5_Toggle();
//}

void enableCMP1(void){
    PIR2bits.C1IF = 0;    //clear any leftover interrupt flags
    PIE2bits.C1IE = 1;    //enable CMP1 interrupts
}

void CMP1_ISR(void) 
{
    // Clear the CMP1 interrupt flag
    PIR2bits.C1IF = 0;
    
    //IO_D5_Toggle(); //ovo je ZCD flag ubiti
    zcdTriggered = 1;
    
    PIE2bits.C1IE = 0; //disable CM1 until tmr5 overflows (3ms)
    T5CONbits.TMR5ON = 1; //start timer5
}

int main(void){
    SYSTEM_Initialize();
    __delay_ms(2000);    
    //OSCTUNEbits.TUN = +27; 
    Timer0.TimeoutCallbackRegister(checkCurrentDetection); //svakih 100ms se gleda da li ima struje pomocu sklopa za detekciju
    UART.RxCompleteCallbackRegister(UART_ProcessCommand); // kada do?e poruka i interrupt se generira i o?itana vrijednost se spremi u buffer ovo se pokrene    
    Timer3.TimeoutCallbackRegister(setTriacLow); //pomocu timer 3 se kontrolira duljina pulsa za triac 
    Timer5.TimeoutCallbackRegister(enableCMP1); //pomocu timer 5 se pale opet interrupti od CMP1 

    IO_RC2_SetHigh(); //driver enable HIGH -> enables driver outputs
    IO_RD0_SetLow();  //receiver enable LOW -> enables receiver inputs

    INTERRUPT_GlobalInterruptEnable(); 
    INTERRUPT_PeripheralInterruptEnable(); 
    
    while(1){
        if(zcdTriggered){
            IO_D5_Toggle();
            zcdTriggered = 0;
            ZCD_Handler();
        }
        errorHandler();
        statusUpdate();
    } 
    
    
}
 