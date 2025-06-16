#include <Windows.h>
#include <stdio.h>
#include "module.h"

// Usage:
// 1. Rename hammer_dll to hammer_dll_original
// 2. Rename this dll to hammer_dll and place in the same folder. KEEP THE ORIGINAL!
// 3. Enjoy hotspotting!

typedef void* (*f_CreateInterface)(const char* pName, int* pReturnCode);


// https://stackoverflow.com/a/538179
#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

// Set on load
HMODULE hMainModule = NULL;

bool WINAPI DllMain( HINSTANCE hModule, DWORD loadReason, LPVOID lpReserved ) {
    printf("DllMain called with arg %i\n", loadReason);

    
    if (loadReason == DLL_PROCESS_ATTACH) {
        // hMainModule = LoadLibraryA("hammer_dll_original");

        printf("!! Library attaching...\n");
        Patcher::Patch();
    }

    if (loadReason == DLL_PROCESS_DETACH) {
        // if (hMainModule) FreeLibrary(hMainModule);

        printf("!! Library detatching...\n");
        Patcher::Unpatch();
    }
}

// EXTERN_DLL_EXPORT void* CreateInterface(const char* pName, int* pReturnCode) {
//     printf("CREATING INTERFACE %s\n", pName);
//     if (!hMainModule) {
//         printf("Attempted to call CreateInterface before DLL was loaded!\n");
//         *pReturnCode = -1;
//         return NULL;
//     }

//     f_CreateInterface interfaceOriginal = (f_CreateInterface)GetProcAddress(hMainModule, "CreateInterface");

//     if (!interfaceOriginal) {
//         printf("Could not locate CreateInterface in DLL!\n");
//         *pReturnCode = -1;
//         return NULL;
//     }
//     return interfaceOriginal(pName, pReturnCode);
// }
