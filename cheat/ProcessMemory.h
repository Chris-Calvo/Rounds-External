#pragma once
#ifndef PROCESS_MEMORY_H
#define PROCESS_MEMORY_H

#include <windows.h>
#include <vector>
#include <string>

class ProcessMemory {
public:
    DWORD Pid = 0;
    HANDLE hProcess = nullptr;

    bool GetProcessHandle(const std::wstring& processName);
    uintptr_t GetModuleBaseAddress(const std::wstring& moduleName);
    uintptr_t ResolvePointer(uintptr_t baseAddress, const std::vector<uintptr_t>& offsets);

    template<typename T>
    bool ReadMemory(uintptr_t address, T& value);

    template<typename T>
    bool WriteMemory(uintptr_t address, T& value);

    ~ProcessMemory();
};

template<typename T>
bool ProcessMemory::ReadMemory(uintptr_t address, T& value) {
    if (!hProcess || address == 0) return false;
    return ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), nullptr);
}

template<typename T>
bool ProcessMemory::WriteMemory(uintptr_t address, T& value) {
    if (!hProcess || address == 0) return false;
    return WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(address), &value, sizeof(T), nullptr);
}

#endif