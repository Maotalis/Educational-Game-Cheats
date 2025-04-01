#pragma once

#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include "globals.h"
#include <optional>
using namespace Globals;

DWORD GetProcessIdByName(const std::wstring& processName) {
    // Create a snapshot of all processes
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create process snapshot.\n";
        return 0;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    // Get the first process
    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            // Compare the process name
            if (_wcsicmp(pe32.szExeFile, processName.c_str()) == 0) {
                CloseHandle(hSnapshot);
                return pe32.th32ProcessID; // Return the process ID
            }
        } while (Process32NextW(hSnapshot, &pe32)); // Get the next process
    }
    else {
        std::cerr << "Failed to retrieve process entry.\n";
    }

    CloseHandle(hSnapshot);
    return 0; // Process not found
}

uintptr_t GetBaseModuleAddress(DWORD processId, const std::wstring& moduleName) {
    // Create a snapshot of all modules in the target process
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create module snapshot.\n";
        return 0;
    }

    MODULEENTRY32W me32;
    me32.dwSize = sizeof(MODULEENTRY32W);

    // Enumerate the modules
    if (Module32FirstW(hSnapshot, &me32)) {
        do {
            // Compare the module name
            if (_wcsicmp(me32.szModule, moduleName.c_str()) == 0) {
                CloseHandle(hSnapshot);
                return reinterpret_cast<uintptr_t>(me32.modBaseAddr); // Return the base address
            }
        } while (Module32NextW(hSnapshot, &me32));
    }
    else {
        std::cerr << "Failed to retrieve module entry.\n";
    }

    CloseHandle(hSnapshot);
    return 0; // Module not found
}

uintptr_t GetModuleBaseAddress(uintptr_t procID, const wchar_t* module)
{
    HANDLE handle;
    handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procID);

    MODULEENTRY32 mod;
    mod.dwSize = sizeof(MODULEENTRY32);
    Module32First(handle, &mod);

    do
    {
        if (!wcscmp(module, mod.szModule))
        {
            CloseHandle(handle);
            return (uintptr_t)mod.modBaseAddr;
        }
    } while (Module32Next(handle, &mod));

    CloseHandle(handle);
    return 0;
}


template <class T>
T ReadMemory(uintptr_t addressToRead)
{
    T rpmbuffer;

    ReadProcessMemory(hProcess, (PVOID)addressToRead, &rpmbuffer, sizeof(T), 0);

    return rpmbuffer;
}

// Template function to write memory to another process
template <typename T>
bool WriteMemory(LPVOID address, const T& value) {
    SIZE_T bytesWritten;
    if (WriteProcessMemory(hProcess, address, &value, sizeof(T), &bytesWritten)) {
        return bytesWritten == sizeof(T);
    }
    return false;
}

bool Initialize()
{
    std::wstring targetProcess = L"cs2.exe";
    DWORD processId = GetProcessIdByName(targetProcess);
    hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processId);
    clientDLL = GetModuleBaseAddress(processId, L"client.dll");
    return true;
}