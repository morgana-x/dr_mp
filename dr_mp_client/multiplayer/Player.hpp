#include <cstring>
#include <chrono>
#include <queue>

struct Frame
{
	std::chrono::steady_clock::time_point Time;
	float Pos[3];

	Frame(std::chrono::steady_clock::time_point t, float x, float y, float z)
	{
		Time = t;
		Pos[0] = x;
		Pos[1] = y;
		Pos[2] = z;
	}

	Frame(std::chrono::steady_clock::time_point t, float pos[3])
	{
		Time = t;
		for (int i = 0; i < 3; i++)
			Pos[i] = pos[i];
	}
};

struct Player
{
	bool Active;
	bool IsClient;

	int ID;
	char Name[32];

	float Pos[3];
	float OldPos[3];

	std::deque<Frame> Frames;

	int Map;
	int CamType;

	int CharID;
	int ExpID;

	void SetName(const char* name)
	{
		int len = strlen(name);
		if (len > 31)
			len = 31;

		for (int i = 0; i < len; i++)
			Name[i] = name[i];

		Name[len] = '\0';
	}
};