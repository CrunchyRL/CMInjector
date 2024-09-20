#include <Windows.h>
#include <iostream>
#include <psapi.h>
#include <cstdio>
#include <string>

#pragma comment(lib, "Wininet.lib")
#pragma warning(disable : 4996)
using namespace std;

DWORD GetProcessIdByName(const string& name) {
    DWORD pid = 0, priority = 0;
    DWORD processes[1024], needed;

    if (!EnumProcesses(processes, sizeof(processes), &needed)) return 0;

    DWORD numProcesses = needed / sizeof(DWORD);

    for (DWORD i = 0; i < numProcesses; i++) {
        if (processes[i] != 0) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
            if (hProcess) {
                char processName[MAX_PATH]{};
                if (GetModuleFileNameExA(hProcess, NULL, processName, MAX_PATH)) {
                    string fileName = strrchr(processName, '\\') + 1;
                    if (_stricmp(fileName.c_str(), name.c_str()) == 0) {
                        DWORD processPriority = GetPriorityClass(hProcess);
                        if (processPriority > priority) {
                            pid = processes[i];
                            priority = processPriority;
                        }
                    }
                }
                CloseHandle(hProcess);
            }
        }
    }

    return pid;
}

bool LoadFile(const string& src, const string& dest) {
    FILE* sourceFile = fopen(src.c_str(), "rb");
    if (!sourceFile) return false;

    FILE* destFile = fopen(dest.c_str(), "wb");
    if (!destFile) {
        fclose(sourceFile);
        return false;
    }

    const size_t bufferSize = 4096;
    char buffer[bufferSize];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, bufferSize, sourceFile)) > 0) {
        fwrite(buffer, 1, bytesRead, destFile);
    }

    fclose(sourceFile);
    fclose(destFile);
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <Process Name> <DLL Filename>\n";
        return 1;
    }

    DWORD pid = GetProcessIdByName(argv[1]);
    if (!pid) {
        cout << "Error: Process " << argv[1] << " not found\n";
        return 1;
    }

    HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) {
        cout << "Error: Could not open process " << argv[1] << '\n';
        return 1;
    }

    char dllPath[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, dllPath);
    strcat_s(dllPath, "\\");
    strcat_s(dllPath, argv[2]);

    string tempDir = getenv("TEMP");
    string tempDllPath = tempDir + "\\" + argv[2];

    if (!LoadFile(dllPath, tempDllPath)) {
        cout << "Error: Could not load DLL file from " << dllPath << '\n';
        return 1;
    }

    char fullPath[MAX_PATH];
    GetFullPathNameA(dllPath, MAX_PATH, fullPath, nullptr);

    LPVOID remotePath = VirtualAllocEx(hProcess, nullptr, strlen(fullPath), MEM_COMMIT, PAGE_READWRITE);
    if (!remotePath) {
        cout << "Error: Could not allocate memory in the target process\n";
        return 1;
    }

    if (!WriteProcessMemory(hProcess, remotePath, fullPath, strlen(fullPath), nullptr)) {
        cout << "Error: Could not write to target process memory\n";
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        return 1;
    }

    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    if (!kernel32) {
        cout << "Error: Could not load kernel32.dll\n";
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        return 1;
    }

    LPVOID loadLibraryAddr = (LPVOID)GetProcAddress(kernel32, "LoadLibraryA");
    if (!loadLibraryAddr) {
        cout << "Error: Could not find LoadLibraryA function in kernel32.dll\n";
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        return 1;
    }

    HANDLE hRemoteThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, remotePath, 0, nullptr);
    if (!hRemoteThread) {
        cout << "Error: Could not create remote thread in target process\n";
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        return 1;
    }

    cout << "DLL injected successfully!\n";

    WaitForSingleObject(hRemoteThread, INFINITE);
    VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
    CloseHandle(hRemoteThread);
    CloseHandle(hProcess);

    return 0;
}
