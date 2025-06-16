#pragma once

#include <Windows.h>
#include "MinHook.h"
#include "faceedit.h"
#include "print.h"

namespace Patcher {

enum class ReturnCode_t {
    Success,
    MinHookInitFail,
    DllGetFail,
    GetEntryFail,
    HookFail,
};

/// Get the JustifyTexture entrypoint with the specified module pointer
bool GetEntrypoints(uintptr_t modulePtr, JustifyTextureFunc* out_justifyPtr, CalcTextureCoordsFunc* out_calcCoordsPtr);

/// Patches the justify tool
ReturnCode_t Patch();

/// Un-patches the justify tool
ReturnCode_t Unpatch();

} // namespace Patcher
