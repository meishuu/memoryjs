const memoryjs = require('./build/Release/memoryjs');

module.exports = {
  // data type constants
  BYTE: 'byte',
  INT: 'int',
  INT32: 'int32',
  UINT32: 'uint32',
  INT64: 'int64',
  UINT64: 'uint64',
  DWORD: 'dword',
  SHORT: 'short',
  LONG: 'long',
  FLOAT: 'float',
  DOUBLE: 'double',
  BOOL: 'bool',
  BOOLEAN: 'boolean',
  PTR: 'ptr',
  POINTER: 'pointer',
  STR: 'str',
  STRING: 'string',
  VEC3: 'vec3',
  VECTOR3: 'vector3',
  VEC4: 'vec4',
  VECTOR4: 'vector4',

  // signature type constants
  NORMAL: 0x0,
  READ: 0x1,
  SUBTRACT: 0x2,

  // function data type constants
  T_VOID: 0x0,
  T_STRING: 0x1,
  T_CHAR: 0x2,
  T_BOOL: 0x3,
  T_INT: 0x4,
  T_DOUBLE: 0x5,
  T_FLOAT: 0x6,

  // Memory access types.
  // See: https://docs.microsoft.com/en-gb/windows/desktop/Memory/memory-protection-constants
  PAGE_NOACCESS: 0x01,
  PAGE_READONLY: 0x02,
  PAGE_READWRITE: 0x04,
  PAGE_WRITECOPY: 0x08,
  PAGE_EXECUTE: 0x10,
  PAGE_EXECUTE_READ: 0x20,
  PAGE_EXECUTE_READWRITE: 0x40,
  PAGE_EXECUTE_WRITECOPY: 0x80,
  PAGE_GUARD: 0x100,
  PAGE_NOCACHE: 0x200,
  PAGE_WRITECOMBINE: 0x400,
  PAGE_ENCLAVE_UNVALIDATED: 0x20000000,
  PAGE_TARGETS_NO_UPDATE: 0x40000000,
  PAGE_TARGETS_INVALID: 0x40000000,
  PAGE_ENCLAVE_THREAD_CONTROL: 0x80000000,

  // Memory allocation types.
  // See: https://docs.microsoft.com/en-us/windows/desktop/api/memoryapi/nf-memoryapi-virtualallocex
  MEM_COMMIT: 0x00001000,
  MEM_RESERVE: 0x00002000,
  MEM_RESET: 0x00080000,
  MEM_TOP_DOWN: 0x00100000,
  MEM_RESET_UNDO: 0x1000000,
  MEM_LARGE_PAGES: 0x20000000,
  MEM_PHYSICAL: 0x00400000,

  // Page types.
  // See: https://docs.microsoft.com/en-us/windows/desktop/api/winnt/ns-winnt-_memory_basic_information
  MEM_PRIVATE: 0x20000,
  MEM_MAPPED: 0x40000,
  MEM_IMAGE: 0x1000000,

  openProcess(processIdentifier, callback) {
    if (arguments.length === 1) {
      return memoryjs.openProcess(processIdentifier);
    }

    memoryjs.openProcess(processIdentifier, callback);
  },

  getProcesses(callback) {
    if (arguments.length === 0) {
      return memoryjs.getProcesses();
    }

    memoryjs.getProcesses(callback);
  },

  findModule(moduleName, processId, callback) {
    if (arguments.length === 2) {
      return memoryjs.findModule(moduleName, processId);
    }

    memoryjs.findModule(moduleName, processId, callback);
  },

  getModules(processId, callback) {
    if (arguments.length === 1) {
      return memoryjs.getModules(processId);
    }

    memoryjs.getModules(processId, callback);
  },

  readMemory(handle, address, dataType, callback) {
    if (arguments.length === 3) {
      return memoryjs.readMemory(handle, address, dataType.toLowerCase());
    }

    memoryjs.readMemory(handle, address, dataType.toLowerCase(), callback);
  },

  readBuffer(handle, address, size, callback) {
    if (arguments.length === 3) {
      return memoryjs.readBuffer(handle, address, size);
    }

    memoryjs.readBuffer(handle, address, size, callback);
  },

  writeMemory(handle, address, value, dataType) {
    if (dataType === 'str' || dataType === 'string') {
      value += '\0'; // add terminator
    }

    return memoryjs.writeMemory(handle, address, value, dataType.toLowerCase());
  },

  writeBuffer(handle, address, buffer) {
    return memoryjs.writeBuffer(handle, address, buffer);
  },

  // eslint-disable-next-line
  findPattern(handle, moduleName, signature, signatureType, patternOffset, addressOffset, callback) {
    if (arguments.length === 6) {
      return memoryjs.findPattern(
        handle,
        moduleName,
        signature,
        signatureType,
        patternOffset,
        addressOffset,
      );
    }

    memoryjs.findPattern(
      handle,
      moduleName,
      signature,
      signatureType,
      patternOffset,
      addressOffset,
      callback,
    );
  },

  callFunction(handle, args, returnType, address, callback) {
    if (arguments.length === 4) {
      return memoryjs.callFunction(handle, args, returnType, address);
    }

    memoryjs.callFunction(handle, args, returnType, address, callback);
  },

  virtualAllocEx(handle, address, size, allocationType, protection, callback) {
    if (arguments.length === 5) {
      return memoryjs.virtualAllocEx(handle, address, size, allocationType, protection);
    }

    memoryjs.virtualAllocEx(handle, address, size, allocationType, protection, callback);
  },

  virtualProtectEx(handle, address, size, protection, callback) {
    if (arguments.length === 4) {
      return memoryjs.virtualProtectEx(handle, address, size, protection);
    }

    memoryjs.virtualProtectEx(handle, address, size, protection, callback);
  },

  getRegions(handle, getOffsets, callback) {
    if (arguments.length === 1) {
      return memoryjs.getRegions(handle);
    }

    memoryjs.getRegions(handle, callback);
  },

  virtualQueryEx(handle, address, callback) {
    if (arguments.length === 2) {
      return memoryjs.virtualQueryEx(handle, address);
    }

    memoryjs.virtualQueryEx(handle, address, callback);
  },

  closeProcess: memoryjs.closeProcess,
};
