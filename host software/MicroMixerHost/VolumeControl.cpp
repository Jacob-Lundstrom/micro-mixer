#include "VolumeControl.h"
#include <iostream>
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audiopolicy.h>
#include <Psapi.h>
#include <corecrt_wstring.h>
#include <atlstr.h>
#include <string>

HRESULT hr;
IMMDeviceEnumerator* pEnumerator;
IMMDevice* pDevice;
IAudioSessionManager2* pSessionManager;
IAudioSessionEnumerator* pSessionEnumerator;
int pSessionCount;
IAudioSessionControl* audioSession;
DWORD processId;
IAudioSessionControl2* sessionControl2;
HANDLE processHandle;
TCHAR processPath[MAX_PATH];
ISimpleAudioVolume* appVolume = NULL;

std::wstring wPath;
std::string sPath;
std::string name;
std::size_t lastSlashPosition;

int debug = 0;

void volContInit() {
    // Initialize the COM Library
    hr = CoInitialize(NULL);

    // Create an instance of the audio device enumerator
    pEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID*)&pEnumerator);

    // Get the default audio endpoint device
    pDevice = NULL;
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDevice);

    // Activate an instance of the audio session manager
    hr = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (LPVOID*)&pSessionManager);
}

void setVolume(char fileToChangeVolume[], float newVolumeLevel, bool muted) {
    // Get the audio session enumerator
    hr = pSessionManager->GetSessionEnumerator(&pSessionEnumerator); // 6kb increase
    

    // Get the number of audio sessions
    pSessionCount = 0;
    hr = pSessionEnumerator->GetCount(&pSessionCount);

    // Volume change operation
    for (int i = 0; i < pSessionCount; i++) {
        // Get the i-th audio session
        audioSession = NULL;
        hr = pSessionEnumerator->GetSession(i, &audioSession);

        // Get the process ID of the audio session
        processId = 0;
        sessionControl2 = NULL;
        hr = audioSession->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&sessionControl2);
        if (SUCCEEDED(hr)) {
            sessionControl2->GetProcessId(&processId);
        }

        // Get a handle to the process that created the audio session
        processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
        if (processHandle != NULL) {
            // Get the path of the process executable file
            
            if (GetModuleFileNameEx(processHandle, NULL, processPath, MAX_PATH) != 0 || true) {
                // Convert the path to a string
                wPath = processPath;
                sPath = std::string(wPath.begin(), wPath.end());

                // Extract the name of the executable file
                
                lastSlashPosition = sPath.find_last_of('\\');
                if (debug >= 3) {
                    std::cout << sPath << std::endl;
                }
                if (lastSlashPosition != std::string::npos) {
                    name = sPath.substr(lastSlashPosition + 1);
                    if (debug >= 2) {
                        std::cout << name << std::endl;
                    }
                }
                else {
                    name = nullptr;
                }

                // Check if the name matches the target file name
                if (fileToChangeVolume == name) {
                    // Get the audio volume control for the audio session
                    hr = sessionControl2->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&appVolume);
                    appVolume->SetMasterVolume(newVolumeLevel, NULL);
                    appVolume->SetMute(muted, NULL);
                    appVolume->Release();
                }
            }
            
            CloseHandle(processHandle);

        }
        sessionControl2->Release();
        audioSession->Release();
    }

    // Release everything created in the loop
    pSessionEnumerator->Release();
    
    if (debug >= 1) {
        std::cout << std::endl;
    }
}

void volContDeinit() {
    // Release the session manager
    pSessionManager->Release();

    // Uninitialize COM Library
    CoUninitialize();
}
