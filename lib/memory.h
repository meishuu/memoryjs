#pragma once
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <vector>

namespace memory {
std::vector<MEMORY_BASIC_INFORMATION> getRegions(HANDLE hProcess);
char* readBuffer(HANDLE hProcess, DWORD64 address, SIZE_T size);

template <class T>
T readMemory(HANDLE hProcess, DWORD64 address) {
  T cRead;
  ReadProcessMemory(hProcess, (LPVOID)address, &cRead, sizeof(T), NULL);
  return cRead;
}
}  // namespace memory
