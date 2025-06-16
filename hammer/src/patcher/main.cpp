#include <Windows.h>
#include "module.h"
#include "print.h"

// This module needs to be injected with Frida or similar
// var myModule = Module.load("...");

using Patcher::ReturnCode_t;

bool WINAPI DllMain( HINSTANCE hModule, DWORD loadReason, LPVOID lpReserved ) {
    DebugPrintF("DllMain called with arg %i\n", loadReason);

    if (loadReason == DLL_PROCESS_ATTACH) {
        DebugPrintF("Library attaching...\n");
        ReturnCode_t returnCode = Patcher::Patch();
        if (returnCode != ReturnCode_t::Success) return false;
        DebugPrintF("Success!\n");
    }

    if (loadReason == DLL_PROCESS_DETACH) {
        DebugPrintF("Library detatching...\n");
        Patcher::Unpatch();
    }

    return true;
}
