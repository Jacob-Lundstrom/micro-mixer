#include <stdio.h>
#include <iostream>
#include <Windows.h>
#include <string>
#include "VolumeControl.h"
#include "mixerUSB.h"
#include "MediaControl.h"


float aIn1;
float aIn2;
float aIn3;
float aIn4;
bool mute1 = false;
bool mute2 = false;
bool mute3 = false;
bool mute4 = false;
bool previousMediaStates[3] = {false, false, false};

int main(int argc, char** argv)
{
    bool disconnected = true;
    volContInit();
    unsigned char* deviceData;

    char program1[] = "chrome.exe";
    char program2[] = "Spotify.exe";
    char program3[] = "Zoom.exe";
    char program4[] = "VALORANT-Win64-Shipping.exe";

    bool lastPreviousTrack = 0;
    bool lastPlayPause = 0;
    bool lastNextTrack = 0;

    uint32_t mediaKeys[3] = {VK_MEDIA_PREV_TRACK, VK_MEDIA_PLAY_PAUSE, VK_MEDIA_NEXT_TRACK};

    while (1) {

        while (disconnected) {
            int e = mixerUSBInit();
            if (e != -1) {
                disconnected = false;
            }
        }

        deviceData = mixerUSBReadAllData();
        if (deviceData == nullptr) {
            mixerUSBDeInit();
            disconnected = true;
        }
        else {
            aIn1 = 1.0f - (deviceData[1] / (float)0xFF);
            aIn2 = 1.0f - (deviceData[2] / (float)0xFF);
            aIn3 = 1.0f - (deviceData[3] / (float)0xFF);
            aIn4 = 1.0f - (deviceData[4] / (float)0xFF);

            mute1 = deviceData[5] == 0xFF;
            mute2 = deviceData[6] == 0xFF;
            mute3 = deviceData[7] == 0xFF;
            mute4 = deviceData[8] == 0xFF;

            setVolume(program1, aIn1, mute1);
            setVolume(program2, aIn2, mute2);
            setVolume(program3, aIn3, mute3);
            setVolume(program4, aIn4, mute4);

            for (int i = 9; i <= 11; i++) {
                if (deviceData[i] == 0xFF) {
                    if (!previousMediaStates[i - 9]) {
                        sendMediaKey(mediaKeys[i - 9], 1);
                        previousMediaStates[i - 9] = true;
                    }
                }
                else {
                    previousMediaStates[i - 9] = false;
                }
            }
        }

        Sleep(100);
    }

    volContDeinit();
    mixerUSBDeInit();
    return 0;
}
