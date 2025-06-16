#pragma once

#include "module.h"

#if PASSTHRU
#define ORIG_DLL_NAME "hammer_dll_original.dll"
#else
#define ORIG_DLL_NAME "hammer_dll.dll"
#endif

namespace Patcher {

uintptr_t FindModule() {
    return (uintptr_t)GetModuleHandleA(ORIG_DLL_NAME);
}

void* GetEntrypoint(uintptr_t modulePtr) {
    #if WIN32 && P2CE
    return reinterpret_cast<void*>(modulePtr + 0x18f860);
    #endif
    return NULL;
}

ReturnCode_t Patch() {
    DebugPrintF("Patching DLL...\n");

    // Init MinHook
    MH_STATUS mhStatus = MH_Initialize();
    if (mhStatus != MH_STATUS::MH_OK) {
        DebugPrintF("Failed to initialize MinHook! Error code: %i\n", mhStatus);
        return ReturnCode_t::MinHookInitFail;
    }

    // Get a pointer to the main hammer DLL
    uintptr_t pModule = FindModule();
    if (!pModule) {
        DebugPrintF("Failed to get Hammer DLL handle!\n");
        return ReturnCode_t::DllGetFail;
    }

    // From there, we can find the address of the texture justify function
    auto pEntrypoint = static_cast<JustifyTextureFunc>(GetEntrypoint(pModule));
    if (!pEntrypoint) {
        DebugPrintF("Patch entrypoint is not known for your platform!\n");
        return ReturnCode_t::GetEntryFail;
    }

    JustifyTextureFunc f_originalCallback;

    MH_STATUS hookStatus = MH_CreateHook(pEntrypoint, JustifyTexturePatched, reinterpret_cast<LPVOID*>(&f_originalCallback));
    if (hookStatus != MH_STATUS::MH_OK) {
        DebugPrintF("Failed to hook function. The pointer is probably incorrect! Error code: %i\n", hookStatus);
        return ReturnCode_t::HookFail;
    }

    SetOriginalCallback(f_originalCallback);

    if (MH_EnableHook(pEntrypoint) != MH_STATUS::MH_OK) {
        DebugPrintF("Failed to enable hook!\n");
        return ReturnCode_t::HookFail;
    }

    return ReturnCode_t::Success;
}

ReturnCode_t Unpatch() {
    DebugPrintF("Disabling hooks...\n");

    if (MH_DisableHook(MH_ALL_HOOKS) != MH_STATUS::MH_OK) {
        return ReturnCode_t::HookFail;
    }

    return ReturnCode_t::Success;
}

} // namespace Patcher
