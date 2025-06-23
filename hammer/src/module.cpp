#include "module.h"

#if PASSTHRU
#define ORIG_DLL_NAME "hammer_dll_original.dll"
#else
#define ORIG_DLL_NAME "hammer_dll.dll"
#endif

#if WIN32 && GAME_P2CE
#define PTR_CALC_COORDS         0x01943a0
#define PTR_JUSTIFY_TEXTURE     0x0196a30
#elif WIN32 && GAME_P2
#define PTR_CALC_COORDS         0x012a4e0
#define PTR_JUSTIFY_TEXTURE     0x012bbc0
#else
#error Unsupported game/platform!
#endif

namespace Patcher {

uintptr_t FindModule() {
    return (uintptr_t)GetModuleHandleA(ORIG_DLL_NAME);
}

bool GetEntrypoints(uintptr_t modulePtr, JustifyTextureFunc* out_justifyPtr, CalcTextureCoordsFunc* out_calcCoordsPtr) {
    #if PTR_JUSTIFY_TEXTURE
    *out_justifyPtr    = reinterpret_cast<JustifyTextureFunc>(modulePtr + PTR_JUSTIFY_TEXTURE);
    *out_calcCoordsPtr = reinterpret_cast<CalcTextureCoordsFunc>(modulePtr + PTR_CALC_COORDS);
    return true;
    #endif
    return false;
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

    // TODO: Make this code modular so we don't have to
    // go through this whole dance for every entrypoint

    // Use the main module pointer to find the offsets to our entrypoints
    JustifyTextureFunc f_targetJustifyTexture;
    CalcTextureCoordsFunc f_CalcCoords;
    
    if (!GetEntrypoints(pModule, &f_targetJustifyTexture, &f_CalcCoords)) {
        DebugPrintF("Patch entrypoints are not known for your platform!\n");
        return ReturnCode_t::GetEntryFail;
    }

    // Used to track the original entrypoint
    JustifyTextureFunc f_ogJustifyTexture;

    MH_STATUS hookStatus;
    hookStatus = MH_CreateHook(f_targetJustifyTexture, JustifyTexturePatched, reinterpret_cast<LPVOID*>(&f_ogJustifyTexture));
    if (hookStatus != MH_STATUS::MH_OK) {
        DebugPrintF("Failed to hook function. The pointer is probably incorrect! Error code: %i\n", hookStatus);
        return ReturnCode_t::HookFail;
    }

    // Hand entrypoints off to faceedit module
    SetOriginalCallbacks(f_ogJustifyTexture, f_CalcCoords);

    // Enable JustifyTexture hook
    if (MH_EnableHook(f_targetJustifyTexture) != MH_STATUS::MH_OK) {
        DebugPrintF("Failed to enable hook!\n");
        return ReturnCode_t::HookFail;
    }

    return ReturnCode_t::Success;
}

ReturnCode_t Unpatch() {
    DebugPrintF("Disabling hooks...\n");

    // Disable all hooks
    if (MH_DisableHook(MH_ALL_HOOKS) != MH_STATUS::MH_OK) {
        return ReturnCode_t::HookFail;
    }

    return ReturnCode_t::Success;
}

} // namespace Patcher
