// dllmain.cpp : DLL 애플리케이션의 진입점을 정의합니다.
#include "pch.h"

using namespace std;
using namespace filesystem;
#define LEGIT_OFFSET 5 // MS

char* ScanBasic(char* pattern, char* mask, char* begin, intptr_t size)
{
    intptr_t patternLen = strlen(mask);

    for (int i = 0; i < size; i++)
    {
        bool found = true;
        for (int j = 0; j < patternLen; j++)
        {
            if (mask[j] != '?' && pattern[j] != *(char*)((intptr_t)begin + i + j))
            {
                found = false;
                break;
            }
        }
        if (found)
        {
            return (begin + i);
        }
    }
    return nullptr;
}

char* ScanInternal(char* pattern, char* mask, char* begin, intptr_t size)
{
    char* match{ nullptr };
    MEMORY_BASIC_INFORMATION mbi{};

    for (char* curr = begin; curr < begin + size; curr += mbi.RegionSize)
    {
        if (!VirtualQuery(curr, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT || mbi.Protect & PAGE_GUARD || mbi.Protect & PAGE_NOACCESS) continue;
        //cout << hex << (DWORD)curr << ' ' << mbi.RegionSize << '\n';
        match = ScanBasic(pattern, mask, curr, mbi.RegionSize);

        if (match != nullptr)
        {
            break;
        }
    }
    return match;
}

char* _CUR_BEATMAP_A = (char*)"\x8B\x0D\x00\x00\x00\x00\xBA\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xF8";
char* _CUR_BEATMAP_M = (char*)"xx????x????x????xx";

char* _PSTATE_A = (char*)"\x83\x3D\x00\x00\x00\x00\x02\x75\x18\x80\x3D";
char* _PSTATE_M = (char*)"xx????xxxxx";

char* _TIMESTMP_A = (char*)"\xA3\x00\x00\x00\x00\xEB\x0A\xA1";
char* _TIMESTMP_M = (char*)"x????xxx";

map<string, string> avName_Path;

DWORD WINAPI CelestialStar(HMODULE hModule) {

    AllocConsole();
    FILE* STDOUT; freopen_s(&STDOUT, "CONOUT$", "w", stdout);
    FILE* STDIN; freopen_s(&STDIN, "CONIN$", "r", stdin);

    PState state;
    MODULEINFO mInfo; HMODULE osuModule = GetModuleHandle(L"osu!.exe");
    if (!osuModule) return 0;
    GetModuleInformation(GetCurrentProcess(), osuModule, &mInfo, sizeof(MODULEINFO));
    char* start = (char*)mInfo.lpBaseOfDll;
    DWORD sz = 0x7FFFFFFF - (DWORD)start;

    cout << hex;
    cout << (DWORD)start << ' ' << sz << '\n';

    char* CUR_BEATMAP = ScanInternal(_CUR_BEATMAP_A, _CUR_BEATMAP_M, start, sz);
    DWORD* curBeatmapPtr = (DWORD*)*(DWORD*)(CUR_BEATMAP + 2);
    cout << "Got Current Beatmap Ptr: " << (DWORD)curBeatmapPtr << '\n';

    char* PSTATE = ScanInternal(_PSTATE_A, _PSTATE_M, start, sz);
    BYTE* pStatePtr = (BYTE*)*(DWORD*)(PSTATE + 2);
    cout << "Got Player State Ptr: " << (DWORD)pStatePtr << '\n';

    char* TIMESTMP = ScanInternal(_TIMESTMP_A, _TIMESTMP_M, start, sz);
    int* timeStmpPtr = (int*)*(DWORD*)(TIMESTMP + 1);
    cout << "Got Time Stamp Ptr: " << (DWORD)timeStmpPtr << '\n';
    
    string osuSongsPath = "C:\\Users\\last_\\AppData\\Local\\osu!\\Songs";
    for (const auto& filesEntry : directory_iterator(osuSongsPath)) {
        for (const auto entry : directory_iterator(filesEntry)) {
            if (entry.path().extension().string() == ".osu") {
                avName_Path.insert({ entry.path().filename().string(), entry.path().string() });
            }
        }
    }
    /*
    for (auto iter = avName_Path.begin(); iter != avName_Path.end(); iter++) {
        cout << "File Name: " << iter->first << "\nDir: " << iter->second << '\n';
    }
    */
    cout << "Got Beatmap Files" << '\n';

    Beatmap btmp;
    queue<pair<int, pair<double, int> > > hq; // time, length
    queue<TimingPoint> tq;
    int sleepAmount = 200, curBeatLength = 0;
    double curSV = 1.0, slMul = 1.0;
    while (1) {
        Sleep(sleepAmount);
        state.pState = *pStatePtr;
        state.timestamp = *timeStmpPtr;
        if (state.pState != PlayState::Play) {
            sleepAmount = 200; curBeatLength = 0; curSV = 0.0; slMul = 0.0;
            while (!hq.empty()) hq.pop();
            while (!tq.empty()) tq.pop();
            btmp = Beatmap();
            continue;
        }
        if (btmp.Base == NULL) {
            cout << 1 << '\n';
            btmp = Beatmap(*curBeatmapPtr);
            cout << 1 << '\n';
            btmp.Parse(avName_Path);
            cout << 1 << '\n';
            for (int i = 0; i < btmp.hitObjects.size(); i++) {
                HitObject ho = btmp.hitObjects[i];
                if (ho.type & (1 << 0)) {
                    hq.push({ ho.time, { -1.0, 0 } });
                }
                else if (ho.type & (1 << 1)) {
                    hq.push({ ho.time, { ho.extra.length, ho.extra.slides } });
                }
            }
            for (int i = 0; i < btmp.timingPoints.size(); i++) tq.push(btmp.timingPoints[i]);
            sleepAmount = 1;
            slMul = btmp.sliderMultiplier;
        }


        if (hq.empty()) continue;
        pair<int, pair<double, int> > ch = hq.front();
        TimingPoint ct = tq.empty() ? TimingPoint() : tq.front();

        // gtfu
        while (ct.time <= state.timestamp + LEGIT_OFFSET) {
            cout << dec << "TimingPoint: " << ct.time << '\n';
            if (ct.uninherited) curBeatLength = ct.beatLength;
            else curSV = -ct.beatLength;
            tq.pop(); ct = tq.front();
        }

        // Update first
        if (ct.uninherited) curBeatLength = ct.beatLength;
        else curSV = -ct.beatLength;

        while (ch.first < state.timestamp - LEGIT_OFFSET) {
            hq.pop(); ch = hq.front();
        }

        if (ch.first <= state.timestamp + LEGIT_OFFSET && ch.first >= state.timestamp - LEGIT_OFFSET) {

            double holdTime;
            if (ch.second.first < 0.0) holdTime = 17.5; // single tap
            else {
                holdTime = ch.second.first / (slMul * 100.0 * (100.0 / curSV)) * (double)curBeatLength; // slider
                holdTime *= (double)ch.second.second;
            }
            int inMilli = (int)(holdTime);

            INPUT ip[2];
            ip[0].type = INPUT_KEYBOARD; ip[0].ki.wVk = 'S'; ip[0].ki.dwFlags = 0;
            ip[1].type = INPUT_KEYBOARD; ip[1].ki.wVk = 'S'; ip[1].ki.dwFlags = KEYEVENTF_KEYUP;
            cout << "CurBL: " << curBeatLength << ", CurSV: " << curSV << ", SM: "<< slMul << ", HoldTime: " << inMilli << '\n';

            SendInput(1, ip, sizeof(INPUT));
            Sleep(inMilli);
            SendInput(1, ip + 1, sizeof(INPUT));

            hq.pop(); if (!hq.empty()) ch = hq.front();

        }
    }

    FreeConsole();
    if (STDOUT) fclose(STDOUT); if (STDIN) fclose(STDIN);
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CelestialStar, hModule, 0, 0));
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

