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

/** INCLUDES *******************************************************/
#include "usb.h"
#include "usb_device_hid.h"
#include "app_device_custom_hid.h"

#include <string.h>

#include "system.h"


/** VARIABLES ******************************************************/
/* Some processors have a limited range of RAM addresses where the USB module
 * is able to access.  The following section is for those devices.  This section
 * assigns the buffers that need to be used by the USB module into those
 * specific areas.
 */
#if defined(FIXED_ADDRESS_MEMORY)
    #if defined(COMPILER_MPLAB_C18)
        #pragma udata HID_CUSTOM_OUT_DATA_BUFFER = HID_CUSTOM_OUT_DATA_BUFFER_ADDRESS
        unsigned char ReceivedDataBuffer[64];
        #pragma udata HID_CUSTOM_IN_DATA_BUFFER = HID_CUSTOM_IN_DATA_BUFFER_ADDRESS
        unsigned char ToSendDataBuffer[64];
        #pragma udata

    #elif defined(__XC8)
        unsigned char ReceivedDataBuffer[64] HID_CUSTOM_OUT_DATA_BUFFER_ADDRESS;
        unsigned char ToSendDataBuffer[64] HID_CUSTOM_IN_DATA_BUFFER_ADDRESS;
    #endif
#else
    unsigned char ReceivedDataBuffer[64];
    unsigned char ToSendDataBuffer[64];
    uint8_t analogData = 0;
#endif

#define _XTAL_FREQ 48000000
volatile USB_HANDLE USBOutHandle;    
volatile USB_HANDLE USBInHandle;

char keysPressed = 0b0000000;
char lastMuteAction = 0b00000000;
char muteStates = 0b0000;

/** DEFINITIONS ****************************************************/
typedef enum
{
    COMMAND_RELAY_ALL_INFO = 0x80,
    COMMAND_UPDATE_MUTE_STATUS = 0x81,
    COMMAND_CONNECT = 0x82
} MIXER_COMMANDS;

/** FUNCTIONS ******************************************************/

/*********************************************************************
* Function: void MixerInitialize(void);
*
* Overview: Initializes the MicroMixer
*
* PreCondition: None
*
* Input: None
*
* Output: None
*
********************************************************************/
void MixerInitialize()
{
    //initialize the variable holding the handle for the last
    // transmission
    USBInHandle = 0;

    //enable the HID endpoint
    USBEnableEndpoint(CUSTOM_DEVICE_HID_EP, USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);

    //Re-arm the OUT endpoint for the next packet
    USBOutHandle = (volatile USB_HANDLE)HIDRxPacket(CUSTOM_DEVICE_HID_EP,(uint8_t*)&ReceivedDataBuffer[0],64);
}

/*********************************************************************
* Function: void MixerTasks(void);
*
* Overview: Keeps the MicroMixer running.
*
* PreCondition: The demo should have been initialized and started via
*   the MixerInitialize() and MixerStart()
*   respectively.
*
* Input: None
*
* Output: None
*
********************************************************************/
void MixerTasks()
{   
    
    /* If the USB device isn't configured yet, we can't really do anything
     * else since we don't have a host to talk to.  So jump back to the
     * top of the while loop. */
    if( USBGetDeviceState() < CONFIGURED_STATE )
    {
        return;
    }

    /* If we are currently suspended, then we need to see if we need to
     * issue a remote wakeup.  In either case, we shouldn't process any
     * keyboard commands since we aren't currently communicating to the host
     * thus just continue back to the start of the while loop. */
    if( USBIsDeviceSuspended() == true )
    {
        return;
    }
    
    //Check if we have received an OUT data packet from the host
    if(HIDRxHandleBusy(USBOutHandle) == false)
    {
        for (int i = 0; i < sizeof(ToSendDataBuffer); i++){
            ToSendDataBuffer[i] = 0x00; // Clears the whole buffer
        }
        //We just received a packet of data from the USB host.
        //Check the first uint8_t of the packet to see what command the host
        //application software wants us to fulfill.
        switch(ReceivedDataBuffer[0])				//Look at the data the host sent, to see what kind of application specific command it sent.
        {
            case COMMAND_UPDATE_MUTE_STATUS:
                //Update mute LEDs here
                break;
            case COMMAND_RELAY_ALL_INFO:  
                //Check to make sure the endpoint/buffer is free before we modify the contents
                if(!HIDTxHandleBusy(USBInHandle))
                {
                    ToSendDataBuffer[0] = 0x80;				//Echo back to the host PC the command we are fulfilling in the first uint8_t.
                    
                    readSliders(&ToSendDataBuffer[1]);    //Send the data of the sliders to the PC
//                    scanRows();
//                    toggleMutes();W
                    
//                    for (int i = 0; i < 4; i++){
//                        ToSendDataBuffer[i+5] = ((muteStates & (1 << i)) >> i) * 0xFF;
//                    }
                    
//                    ToSendDataBuffer[9] = ((keysPressed & (1 << 4)) >> 4) * 0xFF;
//                    ToSendDataBuffer[10] = ((keysPressed & (1 << 5)) >> 5) * 0xFF; // FIX THIS. JUST EXAMPLE
//                    ToSendDataBuffer[11] = ((keysPressed & (1 << 6)) >> 6) * 0xFF;
                    
                    //Prepare the USB module to send the data packet to the host
                    USBInHandle = HIDTxPacket(CUSTOM_DEVICE_HID_EP, (uint8_t*)&ToSendDataBuffer[0],sizeof(ToSendDataBuffer));
                }
                break;
        }
        //Re-arm the OUT endpoint, so we can receive the next OUT data packet 
        //that the host may try to send us.
        
        USBOutHandle = HIDRxPacket(CUSTOM_DEVICE_HID_EP, (uint8_t*)&ReceivedDataBuffer[0], 64);
    }
}

void readSliders(uint8_t* output){
    uint8_t sliders[] = {0, 1, 18, 19}; 
    
    for (int i = 0 ; i < sizeof(sliders); i++){
        ADCON0bits.CHS = sliders[i];    // Sets the ADC channel select to the corresponding channel
        ADCON0bits.GO = 1;              // Starts ADC conversion
        while (ADCON0bits.GO);          // Waits for conversion to complete
        output[i] = ADRESH;             // Will use the high side of the 10 bit output
                                        // This causes more stable inputs, as it ignores the lower readings        
    }
}

void scanRows(void){
    PORTC = 1 << 1;
    keysPressed |= (PORTBbits.RB5 << 0) | (PORTBbits.RB4 << 1) | (PORTBbits.RB3 << 2) | (PORTBbits.RB2 << 3); // Mutes
    PORTC = 1 << 2;
    keysPressed |= (PORTBbits.RB5 << 5) | (PORTBbits.RB4 << 6) | (PORTBbits.RB3 << 7); // Media
    PORTC = 0;
}

void toggleMutes(){    
    for (int i = 0; i < 4; i++){
        if ( (keysPressed & (1 << i)) != (lastMuteAction & (1 << i)) ){
            muteStates ^= keysPressed & (1 << i);     // Flips the current state of the mute buttons
//            PORTA ^= keyStates & (1 << (i + 4));    // Toggles indicator LEDs 
                                                    // Add function to send mute command over USB
        }
    }
    lastMuteAction = keysPressed & 0b00001111;
}

