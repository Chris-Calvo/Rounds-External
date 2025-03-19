#include "ProcessMemory.h"
#include <tlhelp32.h>
#include <windows.h>

#include <sstream>
#include <vector>
#include <string>

bool ProcessMemory::GetProcessHandle(const std::wstring& processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        MessageBox(nullptr, L"Failed to create process snapshot.", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (processName == pe32.szExeFile) {
                Pid = pe32.th32ProcessID;
                hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Pid);
                CloseHandle(hSnapshot);
                if (!hProcess) {
                    MessageBox(nullptr, L"Failed to open process.", L"Error", MB_OK | MB_ICONERROR);
                }
                return hProcess != nullptr;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    //MessageBox(nullptr, L"Process not found.", L"Error", MB_OK | MB_ICONERROR);
    return false;
}

uintptr_t ProcessMemory::GetModuleBaseAddress(const std::wstring& moduleName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, Pid);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        MessageBox(nullptr, L"Failed to create module snapshot.", L"Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    MODULEENTRY32 me32;
    me32.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hSnapshot, &me32)) {
        do {
            if (moduleName == me32.szModule) {
                CloseHandle(hSnapshot);
                return reinterpret_cast<uintptr_t>(me32.modBaseAddr);
            }
        } while (Module32Next(hSnapshot, &me32));
    }

    CloseHandle(hSnapshot);
    MessageBox(nullptr, L"Module not found.", L"Error", MB_OK | MB_ICONERROR);
    return 0;
}


uintptr_t ProcessMemory::ResolvePointer(uintptr_t baseAddress, const std::vector<uintptr_t>& offsets) {
    uintptr_t address = baseAddress;

    for (size_t i = 0; i < offsets.size() - 1; i++) {

        uintptr_t nextAddress = 0;
        uintptr_t readAddress = address + offsets[i];

        if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(readAddress), &nextAddress, sizeof(nextAddress), nullptr)) {
            return -1;
        }

        address = nextAddress;  
    }

    // The final offset is NOT dereferenced, just added
    address += offsets.back();

    return address;
}


//uintptr_t ProcessMemory::ResolvePointer(HANDLE hProc, uintptr_t baseAddress, std::vector<unsigned int> offsets)
//{
//    uintptr_t addr = baseAddress;
//    for (unsigned int i = 0; i < offsets.size(); ++i)
//    {
//        ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
//        addr += offsets[i];
//    }
//    return addr;
//}


ProcessMemory::~ProcessMemory() {
    if (hProcess) {
        CloseHandle(hProcess);
    }
}
