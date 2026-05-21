#include <cstring>
#include <chrono>

struct Player
{
	bool Active;
	bool IsClient;

	int ID;
	char Name[32];

	float Pos[3];
	float OldPos[3];

	std::chrono::steady_clock::time_point LerpStart;
	bool Lerp;

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