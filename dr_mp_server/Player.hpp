
struct Player
{
	bool Active;
	bool FinCon;

	int ID;
	char Name[32];

	float Pos[3];
	int Map;
	int CamType;

	int CharID;
	int ExpID;

	int Socket;

	void SetName(const char* name)
	{
		int len = strlen(name);
		if (len > 31)
			len = 31;

		for (int i = 0; i < len; i++)
			Name[i] = name[i];

		Name[len] = '\0';
	}

	void CloseConnection()
	{
		Active = false;
		closesocket(Socket);
		FinCon = false;
	};


	void StartConnect(int socket)
	{
		FinCon = false;
		Active = true;
		SetName("???");
		Map = -1;
		Socket = socket;
	};
};