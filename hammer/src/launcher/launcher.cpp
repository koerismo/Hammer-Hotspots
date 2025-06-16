#include <stdio.h>
#include <Windows.h>
#include "module.h"

HINSTANCE hMainModule;

typedef int (__stdcall *f_hammerMain)();

int main(int vargs, const char* args) {
    printf("Initializing...\n");

    // Load main dll
    hMainModule = LoadLibrary("hammer_dll");
    if (!hMainModule) {
        printf("Failed to load Hammer DLL!");
        return 1;
    }

    // Hook 'fit' button
    if (Patcher::Patch() != Patcher::ReturnCode_t::Success) {
        return 1;
    }

    printf("Patched! Launching...\n");

    // Get DLL entrypoint function
    auto hammerEntry = GetProcAddress(hMainModule, "entry");
    if (!hammerEntry) {
        printf("Couldn't get hammer entrypoint function!\n");
        return 1;
    }

    // ACTION!!
    hammerEntry();

    return 0;
}
