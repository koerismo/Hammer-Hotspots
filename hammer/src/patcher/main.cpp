#include <Windows.h>
#include <stdio.h>
#include "module.h"

// This module needs to be injected with Frida or similar
// var myModule = Module.load("...");

bool WINAPI DllMain( HINSTANCE hModule, DWORD loadReason, LPVOID lpReserved ) {
    printf("DllMain called with arg %i\n", loadReason);

    if (loadReason == DLL_PROCESS_ATTACH) {
        printf("Library loaded!\n");
        Patcher::Patch();
    }

    if (loadReason == DLL_PROCESS_DETACH) {
        printf("Library detatching...\n");
        Patcher::Unpatch();
    }
}
