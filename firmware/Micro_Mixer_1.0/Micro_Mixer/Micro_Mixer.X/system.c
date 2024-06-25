/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

To request to license the code under the MLA license (www.microchip.com/mla_license), 
please contact mla_licensing@microchip.com
*******************************************************************************/

#include <xc.h>
#include "system.h"
#include "usb.h"

/** CONFIGURATION Bits **********************************************/

// PIC18F24K50 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1L
#pragma config PLLSEL = PLL3X   // PLL Selection (3x clock multiplier)
#pragma config CFGPLLEN = OFF   // PLL Enable Configuration bit (PLL Disabled (firmware controlled))
#pragma config CPUDIV = NOCLKDIV// CPU System Clock Postscaler (CPU uses system clock (no divide))
#pragma config LS48MHZ = SYS48X8// Low Speed USB mode with 48 MHz system clock (System clock at 48 MHz, USB clock divider is set to 8)

// CONFIG1H
#pragma config FOSC = INTOSCIO  // Oscillator Selection (Internal oscillator)
#pragma config PCLKEN = OFF     // Primary Oscillator Shutdown (Primary oscillator shutdown firmware controlled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config nPWRTEN = OFF    // Power-up Timer Enable (Power up timer disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable (BOR enabled in hardware (SBOREN is ignored))
#pragma config BORV = 190       // Brown-out Reset Voltage (BOR set to 1.9V nominal)
#pragma config nLPBOR = ON      // Low-Power Brown-out Reset (Low-Power Brown-out Reset enabled)

// CONFIG2H
#pragma config WDTEN = SWON     // Watchdog Timer Enable bits (WDT controlled by firmware (SWDTEN enabled))
#pragma config WDTPS = 32768    // Watchdog Timer Postscaler (1:32768)

// CONFIG3H
#pragma config CCP2MX = RC1     // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<5:0> pins are configured as analog input channels on Reset)
#pragma config T3CMX = RC0      // Timer3 Clock Input MUX bit (T3CKI function is on RC0)
#pragma config SDOMX = RB3      // SDO Output MUX bit (SDO function is on RB3)
#pragma config MCLRE = ON       // Master Clear Reset Pin Enable (MCLR pin enabled; RE3 input disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset (Stack full/underflow will cause Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config ICPRT = OFF      // Dedicated In-Circuit Debug/Programming Port Enable (ICPORT disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled)

// CONFIG5L
#pragma config CP0 = OFF        // Block 0 Code Protect (Block 0 is not code-protected)
#pragma config CP1 = OFF        // Block 1 Code Protect (Block 1 is not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protect (Boot block is not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protect (Data EEPROM is not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Block 0 Write Protect (Block 0 (0800-1FFFh) is not write-protected)
#pragma config WRT1 = OFF       // Block 1 Write Protect (Block 1 (2000-3FFFh) is not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Registers Write Protect (Configuration registers (300000-3000FFh) are not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protect (Boot block (0000-7FFh) is not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protect (Data EEPROM is not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Block 0 Table Read Protect (Block 0 is not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Block 1 Table Read Protect (Block 1 is not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protect (Boot block is not protected from table reads executed in other blocks)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
 
/*********************************************************************
* Function: void SYSTEM_Initialize( SYSTEM_STATE state )
*
* Overview: Initializes the system.
*
* PreCondition: None
*
* Input:  SYSTEM_STATE - the state to initialize the system into
*
* Output: None
*
********************************************************************/
void SYSTEM_Initialize( SYSTEM_STATE state )
{
    switch(state)
    {
        case SYSTEM_STATE_USB_START:
            
            //Configure oscillator settings for clock settings compatible with USB
            //operation.  Note: Proper settings depends on USB speed (full or low).
            #if(USB_SPEED_OPTION == USB_FULL_SPEED)
                OSCTUNE = 0x80; //3X PLL ratio mode selected
                OSCCON = 0x70;  //Switch to 16MHz HFINTOSC
                OSCCON2 = 0x10; //Enable PLL, SOSC, PRI OSC drivers turned off
                while(OSCCON2bits.PLLRDY != 1);   //Wait for PLL lock
                ACTCON = 0x90;  //Enable active clock tuning for USB operation
            #endif
            
            // Pin setups
            TRISA &= ~(1 << 4 | 1 << 5 | 1 << 6 | 1 << 7);  // LED0, LED1, LED2, LED3 outputs
            TRISA |= 1 << 0 | 1 << 1;                       // AIN1, AIN2 inputs
            TRISB |= 1 << 2 | 1 << 3 | 1 << 4 | 1 << 5;     // COL0, COL1, COL2, COL3 inputs
            TRISC &= ~(1 << 1 | 1 << 2);                    // ROW0, ROW1 outputs
            TRISC |= 1 << 6 | 1 << 7;                       // AIN3, AIN4 inputs
			             
            // Setup the ADC
            ADCON0bits.ADON  = 0b1;                     // Turns the ADC on
            ADCON1bits.PVCFG = 0b00;                    // Sets the positive voltage reference to VDD
            ADCON1bits.NVCFG = 0b00;                    // Sets the negative voltage reference to VSS
            ADCON2bits.ADFM  = 0b0;                     // Sets the ADC to be left justified
            ADCON2bits.ACQT  = 0b010;                   // Turns on manual acquisition
            ADCON2bits.ADCS  = 0b110;                   // Sets the A/D Conversion clock to Fosc/64
            
            
            break;
            
        case SYSTEM_STATE_USB_SUSPEND: 
            break;
            
        case SYSTEM_STATE_USB_RESUME:
            break;
    }
}


void SYSTEM_Tasks(void) {
    
}


#if(__XC8_VERSION < 2000)
    #define INTERRUPT interrupt
#else
    #define INTERRUPT __interrupt()
#endif

void INTERRUPT SYS_InterruptHigh(void)
{
    #if defined(USB_INTERRUPT)
        USBDeviceTasks();
    #endif
}
