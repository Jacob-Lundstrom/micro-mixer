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
void MixerInitialize(void);

/*********************************************************************
* Function: void MixerStart(void);
*
* Overview: Starts running the MicroMixer
*
* PreCondition: The device should be configured into the configuration
*   that contains the custom HID interface.  The MixerInitialize()
*   function should also have been called before calling this function.
*
* Input: None
*
* Output: None
*
********************************************************************/
void MixerStart(void);

/*********************************************************************
* Function: void MixerTasks(void);
*
* Overview: Keeps the MicroMixer running.
*
* PreCondition: The demo should have been initialized and started via
*   the MixerInitialize() and MixerStart() demos
*   respectively.
*
* Input: None
*
* Output: None
*
********************************************************************/
void MixerTasks(void);

void readSliders(uint8_t* output);
void scanRows(void);
void toggleMutes(void);