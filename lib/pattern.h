#pragma once
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <TlHelp32.h>

namespace pattern {
  // Signature/pattern types
  enum {
    // normal: normal
    // read: read memory at pattern
    // subtract: subtract module base
    ST_NORMAL = 0x0,
    ST_READ = 0x1,
    ST_SUBTRACT = 0x2
  };

  uintptr_t findPattern(HANDLE handle, MODULEENTRY32 module, const char* pattern, short sigType, uintptr_t patternOffset, uintptr_t addressOffset);
  bool compareBytes(const unsigned char* bytes, const char* pattern);
}
