#include <stdio.h>
#include <Windows.h>
#include "module.h"

HINSTANCE hMainModule;

typedef void* (*f_CreateInterface)(const char* pName, int* pReturnCode);

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
    auto f_createInterface = reinterpret_cast<f_CreateInterface>(GetProcAddress(hMainModule, "CreateInterface"));
    if (!f_createInterface) {
        printf("Couldn't get hammer entrypoint function!\n");
        return 1;
    }

    // ACTION!!
    int returnCode;
    void* hammerInterface = f_createInterface("Hammer001", &returnCode);
    // TODO: I have no clue what to do from here lmao

    return 0;
}
