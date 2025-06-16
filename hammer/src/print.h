#pragma once

#include <Windows.h>
#include <stdio.h>
#include "logging.h"

template<typename... Args> void DebugPrintF(const char* fmt, Args... args) {
    LoggingSystem_Log(0, 0, fmt, args...);
    // char buf[256];
    // sprintf_s(buf, fmt, args...);
    // OutputDebugStringA(buf);
}
