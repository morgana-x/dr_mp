// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "Core.hpp"
#include "Draw.hpp"
#include "stdio.h"

void AllocateConsole()
{
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:

        AllocateConsole();

        DisableThreadLibraryCalls(hModule);

        Core::Init(hModule);
        break;
    case DLL_PROCESS_DETACH:
        Core::Uninit();
        break;
    }
    return TRUE;
}

/*
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
    Draw::Init();
    return 0;
}*/