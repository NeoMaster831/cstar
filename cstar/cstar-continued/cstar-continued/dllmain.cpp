// dllmain.cpp : DLL 애플리케이션의 진입점을 정의합니다.
#include "pch.h"
#include <Psapi.h>

using namespace std;
#define LEN 6

DWORD ebpValue, jmpBack;

void __declspec(naked) get_edi() {
    __asm {
        // original instructions
        mov [ebp-0xC], eax
        mov eax, [ebp-0xC]

        // get edi
        mov ebpValue, ebp

        // jmp back
        jmp [jmpBack]
    }
}

DWORD WINAPI MainThread(HMODULE hModule) {
    
    AllocConsole(); FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    cout << hex << (DWORD)get_edi << '\n';

    DWORD query = 0x401491;

    DWORD dummy; VirtualProtect((LPVOID)query, LEN, PAGE_EXECUTE_READWRITE, &dummy);
    memset((void*)query, 0x90, LEN);
    *(BYTE*)query = 0xE9;

    *(DWORD*)(query + 1) = (DWORD)get_edi - query - 5;

    cout << *(DWORD*)(query + 1) << '\n';
    jmpBack = query + LEN;

    VirtualProtect((LPVOID)query, LEN, dummy, &dummy);

    while (1) {
        cout << ebpValue << '\n';
        Sleep(400);
    }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, NULL, 0, NULL));
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

/*

Address of signature = mscorlib.ni.dll + 0x00AD7D60
"\xA5\xA5\x33\xD2\x89\x55\x00\xC7\x45\xDC\x00\x00\x00\x00\xC7\x45\xDC\x00\x00\x00\x00\xC7\x45\xE8\x00\x00\x00\x00\xC7\x45\xEC\x00\x00\x00\x00\x68\x00\x00\x00\x00\xEB\x00\x81\x7D\xDC\x00\x00\x00\x00\x7F\x00\x81\x7D\xDC\x00\x00\x00\x00\x7E\x00\x8D\x4D\x00\xFF\x15\x00\x00\x00\x00\x66\xC7\x45\xCC\x00\x00\x83\x7D\xE0", "xxxxxx?xxx????xxx????xxx????xxx????x????x?xxx????x?xxx????x?xx?xx????xxxx??xxx"
"A5 A5 33 D2 89 55 ? C7 45 DC ? ? ? ? C7 45 DC ? ? ? ? C7 45 E8 ? ? ? ? C7 45 EC ? ? ? ? 68 ? ? ? ? EB ? 81 7D DC ? ? ? ? 7F ? 81 7D DC ? ? ? ? 7E ? 8D 4D ? FF 15 ? ? ? ? 66 C7 45 CC ? ? 83 7D E0"
*/