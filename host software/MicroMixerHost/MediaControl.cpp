#include <stdbool.h>
#include <Windows.h>

INPUT simulatedKeyboard[1] = {};

void sendMediaKey(int keyCode, bool state) {
    if (state) {
        simulatedKeyboard[0].type = INPUT_KEYBOARD;
        simulatedKeyboard[0].ki.wVk = keyCode;
        simulatedKeyboard[0].ki.dwFlags = 0;
        SendInput(ARRAYSIZE(simulatedKeyboard), simulatedKeyboard, sizeof(simulatedKeyboard));
        simulatedKeyboard[0].ki.dwFlags = KEYEVENTF_KEYUP;
    }
}