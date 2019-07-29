#pragma once
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <TlHelp32.h>
#include <vector>

namespace module {
std::vector<MODULEENTRY32> getModules(DWORD processId, char** errorMessage);
MODULEENTRY32 findModule(const char* moduleName, DWORD processId, char** errorMessage);
DWORD64 getBaseAddress(const char* processName, DWORD processId);
}  // namespace module
