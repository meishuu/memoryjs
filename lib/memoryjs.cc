#include <windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <napi.h>
#include <psapi.h>
#include <string>
#include <thread>
#include <vector>
#include "memory.h"
#include "module.h"
#include "pattern.h"
#include "process.h"

#pragma comment(lib, "psapi.lib")

struct Vector3 {
  float x, y, z;
};

struct Vector4 {
  float w, x, y, z;
};

namespace memoryjs {
static void throwError(Napi::Env env, char* error) {
  Napi::TypeError::New(env, Napi::String::New(env, error)).ThrowAsJavaScriptException();
}
}  // namespace memoryjs

Napi::Value openProcess(const Napi::CallbackInfo& args) {
  auto env = args.Env();

  if (args.Length() != 1 && args.Length() != 2) {
    memoryjs::throwError(env, "requires 1 argument, or 2 arguments if a callback is being used");
    return env.Null();
  }

  if (!args[0].IsString() && !args[0].IsNumber()) {
    memoryjs::throwError(env, "first argument must be a string or a number");
    return env.Null();
  }

  if (args.Length() == 2 && !args[1].IsFunction()) {
    memoryjs::throwError(env, "second argument must be a function");
    return env.Null();
  }

  // Define error message that may be set by the function that opens the process
  char* errorMessage = "";

  process::Pair pair;

  if (args[0].IsString()) {
    std::string processName(args[0].As<Napi::String>().Utf8Value());
    pair = process::openProcess(processName.c_str(), &errorMessage);

    // In case it failed to open, let's keep retrying
    // while(!strcmp(process.szExeFile, "")) {
    //   process = process::openProcess((char*) *(processName), &errorMessage);
    // };
  }

  if (args[0].IsNumber()) {
    uint32_t processId = args[0].As<Napi::Number>().Uint32Value();
    pair = process::openProcess(processId, &errorMessage);

    // In case it failed to open, let's keep retrying
    // while(!strcmp(process.szExeFile, "")) {
    //   process = process::openProcess(args[0]->Uint32Value(), &errorMessage);
    // };
  }

  // If an error message was returned from the function that opens the process, throw the error.
  // Only throw an error if there is no callback (if there's a callback, the error is passed there).
  if (strcmp(errorMessage, "") && args.Length() != 2) {
    memoryjs::throwError(env, errorMessage);
    return env.Null();
  }

  // Create a v8 Object (JSON) to store the process information
  Napi::Object processInfo = Napi::Object::New(env);

  processInfo.Set("dwSize", Napi::Number::New(env, (int)pair.process.dwSize));
  processInfo.Set("th32ProcessID", Napi::Number::New(env, (int)pair.process.th32ProcessID));
  processInfo.Set("cntThreads", Napi::Number::New(env, (int)pair.process.cntThreads));
  processInfo.Set("th32ParentProcessID", Napi::Number::New(env, (int)pair.process.th32ParentProcessID));
  processInfo.Set("pcPriClassBase", Napi::Number::New(env, (int)pair.process.pcPriClassBase));
  processInfo.Set("szExeFile", Napi::String::New(env, pair.process.szExeFile));
  processInfo.Set("handle", Napi::Number::New(env, (int)pair.handle));

  DWORD64 base = module::getBaseAddress(pair.process.szExeFile, pair.process.th32ProcessID);
  processInfo.Set("modBaseAddr", Napi::Number::New(env, (uintptr_t)base));

  // openProcess can either take one argument or can take
  // two arguments for asychronous use (second argument is the callback)
  if (args.Length() == 2) {
    // Callback to let the user handle with the information
    Napi::Function callback = args[1].As<Napi::Function>();
    callback.Call({Napi::String::New(env, errorMessage), processInfo});
    return env.Null();
  }

  // return JSON
  return processInfo;
}

void closeProcess(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();

  if (args.Length() != 1) {
    memoryjs::throwError(env, "requires 1 argument");
    return;
  }

  if (!args[0].IsNumber()) {
    memoryjs::throwError(env, "first argument must be a number");
    return;
  }

  int32_t hProcess = args[0].As<Napi::Number>().Int32Value();
  process::closeProcess((HANDLE)hProcess);
}

Napi::Value getProcesses(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();

  if (args.Length() > 1) {
    memoryjs::throwError(env, "requires either 0 arguments or 1 argument if a callback is being used");
    return env.Null();
  }

  if (args.Length() == 1 && !args[0].IsFunction()) {
    memoryjs::throwError(env, "first argument must be a function");
    return env.Null();
  }

  // Define error message that may be set by the function that gets the processes
  char* errorMessage = "";

  std::vector<PROCESSENTRY32> processEntries = process::getProcesses(&errorMessage);

  // If an error message was returned from the function that gets the processes, throw the error.
  // Only throw an error if there is no callback (if there's a callback, the error is passed there).
  if (strcmp(errorMessage, "") && args.Length() != 1) {
    memoryjs::throwError(env, errorMessage);
    return env.Null();
  }

  // Creates v8 array with the size being that of the processEntries vector processes is an array of JavaScript objects
  Napi::Array processes = Napi::Array::New(env, processEntries.size());

  // Loop over all processes found
  for (std::vector<PROCESSENTRY32>::size_type i = 0; i != processEntries.size(); i++) {
    // Create a v8 object to store the current process' information
    Napi::Object process = Napi::Object::New(env);

    process.Set("cntThreads", Napi::Number::New(env, (int)processEntries[i].cntThreads));
    process.Set("szExeFile", Napi::String::New(env, processEntries[i].szExeFile));
    process.Set("th32ProcessID", Napi::Number::New(env, (int)processEntries[i].th32ProcessID));
    process.Set("th32ParentProcessID", Napi::Number::New(env, (int)processEntries[i].th32ParentProcessID));
    process.Set("pcPriClassBase", Napi::Number::New(env, (int)processEntries[i].pcPriClassBase));

    // Push the object to the array
    processes.Set(i, process);
  }

  /* getProcesses can either take no arguments or one argument
     one argument is for asychronous use (the callback) */
  if (args.Length() == 1) {
    // Callback to let the user handle with the information
    Napi::Function callback = args[0].As<Napi::Function>();
    callback.Call({Napi::String::New(env, errorMessage), processes});
    return env.Null();
  }

  // return JSON
  return processes;
}

Napi::Value getModules(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();

  if (args.Length() != 1 && args.Length() != 2) {
    memoryjs::throwError(env, "requires 1 argument, or 2 arguments if a callback is being used");
    return env.Null();
  }

  if (!args[0].IsNumber()) {
    memoryjs::throwError(env, "first argument must be a number");
    return env.Null();
  }

  if (args.Length() == 2 && !args[1].IsFunction()) {
    memoryjs::throwError(env, "first argument must be a number, second argument must be a function");
    return env.Null();
  }

  // Define error message that may be set by the function that gets the modules
  char* errorMessage = "";

  int32_t processId = args[0].As<Napi::Number>().Int32Value();
  std::vector<MODULEENTRY32> moduleEntries = module::getModules(processId, &errorMessage);

  // If an error message was returned from the function getting the modules, throw the error.
  // Only throw an error if there is no callback (if there's a callback, the error is passed there).
  if (strcmp(errorMessage, "") && args.Length() != 2) {
    memoryjs::throwError(env, errorMessage);
    return env.Null();
  }

  // Creates v8 array with the size being that of the moduleEntries vector
  // modules is an array of JavaScript objects
  Napi::Array modules = Napi::Array::New(env, moduleEntries.size());

  // Loop over all modules found
  for (std::vector<MODULEENTRY32>::size_type i = 0; i != moduleEntries.size(); i++) {
    //  Create a v8 object to store the current module's information
    Napi::Object module = Napi::Object::New(env);

    module.Set("modBaseAddr", Napi::Number::New(env, (uintptr_t)moduleEntries[i].modBaseAddr));
    module.Set("modBaseSize", Napi::Number::New(env, (int)moduleEntries[i].modBaseSize));
    module.Set("szExePath", Napi::String::New(env, moduleEntries[i].szExePath));
    module.Set("szModule", Napi::String::New(env, moduleEntries[i].szModule));
    module.Set("th32ModuleID", Napi::Number::New(env, (int)moduleEntries[i].th32ProcessID));

    // Push the object to the array
    modules.Set(i, module);
  }

  // getModules can either take one argument or two arguments
  // one/two arguments is for asychronous use (the callback)
  if (args.Length() == 2) {
    // Callback to let the user handle with the information
    Napi::Function callback = args[1].As<Napi::Function>();
    callback.Call({Napi::String::New(env, errorMessage), modules});
    return env.Null();
  }

  // return JSON
  return modules;
}

Napi::Value readMemory(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();

  if (args.Length() != 3 && args.Length() != 4) {
    memoryjs::throwError(env, "requires 3 arguments, or 4 arguments if a callback is being used");
    return env.Null();
  }

  if (!args[0].IsNumber() && !args[1].IsNumber() && !args[2].IsString()) {
    memoryjs::throwError(env, "first and second argument must be a number, third argument must be a string");
    return env.Null();
  }

  if (args.Length() == 4 && !args[3].IsFunction()) {
    memoryjs::throwError(env, "fourth argument must be a function");
    return env.Null();
  }

  std::string dataType(args[2].As<Napi::String>().Utf8Value());

  // Set callback variables in the case the a callback parameter has been passed
  Napi::Function callback = args[3].As<Napi::Function>();
  Napi::Value cbError = env.Null();  // Define the error message that will be set if no data type is recognised
  Napi::Value cbResult;

  HANDLE handle = (HANDLE)args[0].As<Napi::Number>().Int32Value();
  DWORD64 address = args[1].As<Napi::Number>().Int64Value();

  if (dataType == "byte") {
    unsigned char result = memory::readMemory<unsigned char>(handle, address);
    if (args.Length() == 4)
      cbResult = Napi::Number::New(env, result);
    else
      return Napi::Number::New(env, result);

  } else if (dataType == "int") {
    int result = memory::readMemory<int>(handle, address);
    if (args.Length() == 4)
      cbResult = Napi::Number::New(env, result);
    else
      return Napi::Number::New(env, result);

  } else if (dataType == "int32") {
    int32_t result = memory::readMemory<int32_t>(handle, address);
    if (args.Length() == 4)
      cbResult = Napi::Number::New(env, result);
    else
      return Napi::Number::New(env, result);

  } else if (dataType == "uint32") {
    uint32_t result = memory::readMemory<uint32_t>(handle, address);
    if (args.Length() == 4)
      cbResult = Napi::Number::New(env, result);
    else
      return Napi::Number::New(env, result);

  } else if (dataType == "int64") {
    int64_t result = memory::readMemory<int64_t>(handle, address);
    if (args.Length() == 4)
      cbResult = Napi::Number::New(env, result);
    else
      return Napi::Number::New(env, result);

  } else if (dataType == "uint64") {
    uint64_t result = memory::readMemory<uint64_t>(handle, address);
    if (args.Length() == 4)
      cbResult = Napi::Number::New(env, result);
    else
      return Napi::Number::New(env, result);

  } else if (dataType == "dword") {
    DWORD result = memory::readMemory<DWORD>(handle, address);
    if (args.Length() == 4)
      cbResult = Napi::Number::New(env, result);
    else
      return Napi::Number::New(env, result);

  } else if (dataType == "short") {
    short result = memory::readMemory<short>(handle, address);
    if (args.Length() == 4)
      cbResult = Napi::Number::New(env, result);
    else
      return Napi::Number::New(env, result);

  } else if (dataType == "long") {
    long result = memory::readMemory<long>(handle, address);
    if (args.Length() == 4)
      cbResult = Napi::Number::New(env, result);
    else
      return Napi::Number::New(env, result);

  } else if (dataType == "float") {
    float result = memory::readMemory<float>(handle, address);
    if (args.Length() == 4)
      cbResult = Napi::Number::New(env, result);
    else
      return Napi::Number::New(env, result);

  } else if (dataType == "double") {
    double result = memory::readMemory<double>(handle, address);
    if (args.Length() == 4)
      cbResult = Napi::Number::New(env, result);
    else
      return Napi::Number::New(env, result);

  } else if (dataType == "ptr" || dataType == "pointer") {
    intptr_t result = memory::readMemory<intptr_t>(handle, address);
    if (args.Length() == 4)
      cbResult = Napi::Number::New(env, result);
    else
      return Napi::Number::New(env, result);

  } else if (dataType == "bool" || dataType == "boolean") {
    bool result = memory::readMemory<bool>(handle, address);
    if (args.Length() == 4)
      cbResult = Napi::Boolean::New(env, result);
    else
      return Napi::Boolean::New(env, result);

  } else if (dataType == "string" || dataType == "str") {
    std::vector<char> chars;
    int offset = 0x0;
    while (true) {
      char c = memory::readMemory<char>(handle, address + offset);
      chars.push_back(c);

      // break at 1 million chars
      if (offset == (sizeof(char) * 1000000)) {
        chars.clear();
        break;
      }

      // break at terminator (end of string)
      if (c == '\0') {
        break;
      }

      // go to next char
      offset += sizeof(char);
    }

    if (chars.size() == 0) {
      if (args.Length() == 4)
        cbError = Napi::String::New(env, "unable to read string (no null-terminator found after 1 million chars)");
      else {
        memoryjs::throwError(env, "unable to read string (no null-terminator found after 1 million chars)");
        return env.Null();
      }

    } else {
      // vector -> string
      std::string str(chars.begin(), chars.end());

      if (args.Length() == 4)
        cbResult = Napi::String::New(env, str);
      else
        return Napi::String::New(env, str);
    }

  } else if (dataType == "vector3" || dataType == "vec3") {
    Vector3 result = memory::readMemory<Vector3>(handle, address);
    Napi::Object moduleInfo = Napi::Object::New(env);
    moduleInfo.Set("x", Napi::Number::New(env, result.x));
    moduleInfo.Set("y", Napi::Number::New(env, result.y));
    moduleInfo.Set("z", Napi::Number::New(env, result.z));

    if (args.Length() == 4)
      cbResult = moduleInfo;
    else
      return moduleInfo;

  } else if (dataType == "vector4" || dataType == "vec4") {
    Vector4 result = memory::readMemory<Vector4>(handle, address);
    Napi::Object moduleInfo = Napi::Object::New(env);
    moduleInfo.Set("w", Napi::Number::New(env, result.w));
    moduleInfo.Set("x", Napi::Number::New(env, result.x));
    moduleInfo.Set("y", Napi::Number::New(env, result.y));
    moduleInfo.Set("z", Napi::Number::New(env, result.z));

    if (args.Length() == 4)
      cbResult = moduleInfo;
    else
      return moduleInfo;

  } else {
    if (args.Length() == 4)
      cbError = Napi::String::New(env, "unexpected data type");
    else {
      memoryjs::throwError(env, "unexpected data type");
      return env.Null();
    }
  }

  if (args.Length() == 4) callback.Call({cbError, cbResult});

  return env.Null();  // ???
}

Napi::Value readBuffer(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();

  if (args.Length() != 3 && args.Length() != 4) {
    memoryjs::throwError(env, "requires 3 arguments, or 4 arguments if a callback is being used");
    return env.Null();
  }

  if (!args[0].IsNumber() && !args[1].IsNumber() && !args[2].IsNumber()) {
    memoryjs::throwError(env, "first, second and third arguments must be a number");
    return env.Null();
  }

  if (args.Length() == 4 && !args[3].IsFunction()) {
    memoryjs::throwError(env, "fourth argument must be a function");
    return env.Null();
  }

  // Set callback variables in the case the a callback parameter has been passed
  Napi::Function callback = args[3].As<Napi::Function>();
  Napi::Value cbError = env.Null();  // Define the error message that will be set if no data type is recognised
  Napi::Value cbResult;

  HANDLE handle = (HANDLE)args[0].As<Napi::Number>().Int32Value();
  DWORD64 address = args[1].As<Napi::Number>().Int64Value();
  SIZE_T size = args[2].As<Napi::Number>().Uint32Value();
  char* data = memory::readBuffer(handle, address, size);

  Napi::Buffer<char> buffer = Napi::Buffer<char>::Copy(env, data, size);  // TODO copy or new?

  if (args.Length() == 4) {
    cbResult = buffer;
    callback.Call({cbError, cbResult});
    return env.Null();
  }

  return buffer;
}

Napi::Value findPattern(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();

  // if (args.Length() != 5 && args.Length() != 6) {
  //   memoryjs::throwError("requires 5 arguments, or 6 arguments if a callback is being used", isolate);
  //   return;
  // }

  // if (!args[0]->IsNumber() && !args[1]->IsString() && !args[2]->IsNumber() && !args[3]->IsNumber() &&
  // !args[4]->IsNumber()) {
  //   memoryjs::throwError("first argument must be a number, the remaining arguments must be numbers apart from the
  //   callback", isolate); return;
  // }

  // if (args.Length() == 6 && !args[5]->IsFunction()) {
  //   memoryjs::throwError("sixth argument must be a function", isolate);
  //   return;
  // }

  // Address of findPattern result
  uintptr_t address = -1;

  // Define error message that may be set by the function that gets the modules
  char* errorMessage = "";

  HANDLE handle = (HANDLE)args[0].As<Napi::Number>().Int32Value();

  std::vector<MODULEENTRY32> moduleEntries = module::getModules(GetProcessId(handle), &errorMessage);

  // If an error message was returned from the function getting the modules, throw the error.
  // Only throw an error if there is no callback (if there's a callback, the error is passed there).
  if (strcmp(errorMessage, "") && args.Length() != 7) {
    memoryjs::throwError(env, errorMessage);
    return env.Null();
  }

  std::string moduleName(args[1].As<Napi::String>().Utf8Value());
  std::string signature(args[2].As<Napi::String>().Utf8Value());

  for (std::vector<MODULEENTRY32>::size_type i = 0; i != moduleEntries.size(); i++) {
    if (!strcmp(moduleEntries[i].szModule, moduleName.c_str())) {
      // const char* pattern = std::string(*signature).c_str();
      short sigType = args[3].As<Napi::Number>().Uint32Value();
      uint32_t patternOffset = args[4].As<Napi::Number>().Uint32Value();
      uint32_t addressOffset = args[5].As<Napi::Number>().Uint32Value();

      address =
          pattern::findPattern(handle, moduleEntries[i], signature.c_str(), sigType, patternOffset, addressOffset);
      break;
    }
  }

  // If no error was set by getModules and the address is still the value we set it as, it probably means we couldn't
  // find the module
  if (strcmp(errorMessage, "") && address == -1) errorMessage = "unable to find module";

  // If no error was set by getModules and the address is -2 this means there was no match to the pattern
  if (strcmp(errorMessage, "") && address == -2) errorMessage = "no match found";

  // findPattern can be asynchronous
  if (args.Length() == 7) {
    // Callback to let the user handle with the information
    Napi::Function callback = args[6].As<Napi::Function>();
    callback.Call({Napi::String::New(env, errorMessage), Napi::Number::New(env, address)});
    return env.Null();
  }

  // return JSON
  return Napi::Number::New(env, address);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("openProcess", Napi::Function::New(env, openProcess));
  exports.Set("closeProcess", Napi::Function::New(env, closeProcess));
  exports.Set("getProcesses", Napi::Function::New(env, getProcesses));
  exports.Set("getModules", Napi::Function::New(env, getModules));
  exports.Set("readMemory", Napi::Function::New(env, readMemory));
  exports.Set("readBuffer", Napi::Function::New(env, readBuffer));
  exports.Set("findPattern", Napi::Function::New(env, findPattern));
  return exports;
}

NODE_API_MODULE(memoryjs, Init)
