#pragma once

#include "module.h"

namespace Patcher {

uintptr_t FindModule() {
    return (uintptr_t)GetModuleHandleA("hammer_dll.dll");
}

void* GetEntrypoint(uintptr_t modulePtr) {
    #if WIN32
    return reinterpret_cast<void*>(modulePtr + 0x18f860);
    #endif
    return nullptr;
}

ReturnCode_t Patch() {
    printf("Patching DLL...\n");

    // Init MinHook
    MH_STATUS mhStatus = MH_Initialize();
    if (mhStatus != MH_STATUS::MH_OK) {
        printf("Failed to initialize MinHook! Error code: %i\n", mhStatus);
        return ReturnCode_t::MinHookInitFail;
    }

    // Get a pointer to the main hammer DLL
    uintptr_t pModule = FindModule();
    if (!pModule) {
        printf("Failed to get Hammer DLL handle!\n");
        return ReturnCode_t::DllGetFail;
    }

    // From there, we can find the address of the texture justify function
    auto pEntrypoint = static_cast<JustifyTextureFunc>(GetEntrypoint(pModule));
    if (!pEntrypoint) {
        printf("Patch entrypoint is not known for your platform!\n");
        return ReturnCode_t::GetEntryFail;
    }

    JustifyTextureFunc f_originalCallback;

    MH_STATUS hookStatus = MH_CreateHook(pEntrypoint, JustifyTexturePatched, reinterpret_cast<LPVOID*>(&f_originalCallback));
    if (hookStatus != MH_STATUS::MH_OK) {
        printf("Failed to hook function. The pointer is probably incorrect! Error code: %i\n", hookStatus);
        return ReturnCode_t::HookFail;
    }

    SetOriginalCallback(f_originalCallback);

    if (MH_EnableHook(pEntrypoint) != MH_STATUS::MH_OK) {
        printf("Failed to enable hook!\n");
        return ReturnCode_t::HookFail;
    }

    return ReturnCode_t::Success;
}

ReturnCode_t Unpatch() {
    printf("Disabling hooks...\n");

    if (MH_DisableHook(MH_ALL_HOOKS) != MH_STATUS::MH_OK) {
        return ReturnCode_t::HookFail;
    }

    return ReturnCode_t::Success;
}

} // namespace Patcher
