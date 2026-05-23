#include "Core.hpp"
#include "Draw.hpp"
#include <string>
#include "Windows.h"
#include "ext/minhook/include/MinHook.h"

HANDLE hTickThread;
HANDLE hDrawThread;

namespace Core
{
    char actualpath[2048];
    void DetectGame()
    {
        TCHAR szEXEPath[2048];

        GetModuleFileName(NULL, szEXEPath, 2048);
        for (int j = 0; szEXEPath[j] != 0; j++)
            actualpath[j] = szEXEPath[j];

        std::string filePath(actualpath);
        std::size_t found = filePath.find_last_of("/\\");
        filePath = filePath.substr(found + 1);

        Core::Game = filePath == "DR1_us.exe" ? DR1 : filePath == "DR2_us.exe" ? DR2 : None;
    }


	void Init(HMODULE hModule)
	{
        DetectGame();

        MH_Initialize();

        Client = dr_mp::Client();

        Draw::hCurrentModule = hModule;

        hTickThread = CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)Core::Tick, nullptr, NULL, nullptr);
        hDrawThread = CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)Draw::Init, nullptr, NULL, nullptr);
	}

    void Uninit()
    {
        MH_Uninitialize();

        TerminateThread(hTickThread, 0);
        TerminateThread(hDrawThread, 0);
    }

    void Tick()
    {
        while (true)
        {
            Client.Tick();
            Sleep(5);
        }
    }
}