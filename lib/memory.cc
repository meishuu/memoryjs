#include "memory.h"

#include <windows.h>
#include <vector>

std::vector<MEMORY_BASIC_INFORMATION> memory::getRegions(HANDLE hProcess) {
  std::vector<MEMORY_BASIC_INFORMATION> regions;

  MEMORY_BASIC_INFORMATION region;
  DWORD64 address;

  for (address = 0; VirtualQueryEx(hProcess, (LPVOID)address, &region, sizeof(region)) == sizeof(region);
       address += region.RegionSize) {
    regions.push_back(region);
  }

  return regions;
}

char* memory::readBuffer(HANDLE hProcess, DWORD64 address, SIZE_T size) {
  char* buffer = new char[size];
  ReadProcessMemory(hProcess, (LPVOID)address, buffer, size, NULL);
  return buffer;
}
