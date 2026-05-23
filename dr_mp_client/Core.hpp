#pragma once
#include "multiplayer/Client.hpp"
namespace Core
{
	enum DrGame
	{
		None,
		DR1,
		DR2
	};

	inline DrGame Game;
	inline extern void Init(HMODULE hModule);
	inline extern void Uninit();
	inline extern void Tick();
	inline dr_mp::Client Client;
}