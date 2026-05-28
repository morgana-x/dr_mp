#include "Hud.hpp"
#include "../ext/minhook/include/MinHook.h"
#include "../ext/drlib/Dr1.h"
#include "../Core.hpp"

DrLib::Dr1::Funcs::HUD::RenderChapterFunc* RenderChapterOriginal;

template <typename T>
inline MH_STATUS MH_CreateHookEx(LPVOID pTarget, LPVOID pDetour, T** ppOriginal)
{
	return MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}

void renderchat()
{
	/*DrLib::Dr1::Funcs::HUD::RenderRect(0, 0, 480.0, 272.0, 0,
		255, 100, 100, 200,
		100, 100, 100, 200,
		50, 50, 50, 200,
		50, 50, 50, 200,
		0x4e2a, 2, 1, 0xFF);

	for (int i = Core::Client.chat.Length() - 1; i >= 0; i--)
	{
		if (Core::Client.chat.ChatHistory[i].length() == 0)
			continue;
		auto msg = Core::Client.chat.ChatHistory[i].c_str();

		//DrLib::Dr1::Funcs::HUD::PrintString(0x04, 100, 100, msg, 1);
	}

	//DrLib::Dr1::Funcs::HUD::PrintString(0x04, 100, 100, "Hello!", 1);*/
}

void Render()
{
	//renderchat();

	RenderChapterOriginal();
}

namespace dr_mp
{
	void HudInitialise()
	{
		MH_CreateHookEx(DrLib::Dr1::Funcs::HUD::RenderChapter, &Render, &RenderChapterOriginal);
		MH_EnableHook(DrLib::Dr1::Funcs::HUD::RenderChapter);
	}

	void HudUninitialise()
	{
		MH_DisableHook(DrLib::Dr1::Funcs::HUD::RenderChapter);
	}
}