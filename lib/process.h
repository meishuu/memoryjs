#pragma once
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <TlHelp32.h>
#include <vector>

namespace process {
struct Pair {
  HANDLE handle;
  PROCESSENTRY32 process;
};

Pair openProcess(const char* processName, char** errorMessage);
Pair openProcess(DWORD processId, char** errorMessage);
void closeProcess(HANDLE hProcess);
std::vector<PROCESSENTRY32> getProcesses(char** errorMessage);
}  // namespace process
