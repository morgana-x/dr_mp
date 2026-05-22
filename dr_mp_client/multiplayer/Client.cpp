
#include "Client.hpp"
#include "stdio.h"
#include <iostream>
#include <chrono>
#include "../Core.hpp"
#include "../drlib/Dr1.h"
#include "../drlib/Dr2.h"

#pragma comment(lib, "Ws2_32.lib")

int mpOldMap = -1;
int mpOldCamType = -1;
Vec3 mpOldPos = Vec3(0, 0, 0);

void createChar(char c, short exp=0, int pos = 0)
{
	if (Core::Game == Core::DR1)
	{
		DrLib::Dr1::Funcs::Character::CreateChar(c);
		DrLib::Dr1::Funcs::Character::LoadStand(c, exp);
		DrLib::Dr1::Funcs::Character::SpawnChar(c, pos);
		return;
	}
	if (Core::Game == Core::DR2)
	{
		DrLib::Dr2::Funcs::Character::CreateChar(c);
		DrLib::Dr2::Funcs::Character::LoadStand(c, exp);
		DrLib::Dr2::Funcs::Character::SpawnChar(c, pos);
		return;
	}
}

void despawnChar(char c)
{
	if (Core::Game == Core::DR1)
	{
		DrLib::Dr1::Funcs::Character::DespawnChar(c);
		return;
	}
	if (Core::Game == Core::DR2)
	{
	//	DrLib::Dr2::Funcs::Character::DespawnChar(c);
		return;
	}
}

void setCharPos(char c, float pos[3])
{
	if (Core::Game == Core::DR1)
	{
		DrLib::Dr1::Funcs::Character::SetPos(c, pos[0], pos[1], pos[2]);
		return;
	}
	if (Core::Game == Core::DR2)
	{
		//DrLib::Dr2::Funcs::Character::SetPos(c, pos[0], pos[1], pos[2]);
		return;
	}
}

DrLib::Types::Common::Vec3* getPos()
{
	if (Core::Game == Core::DR1)
	{
		return DrLib::Dr1::Values::Player::Pos;
	}
	if (Core::Game == Core::DR2)
	{
		//return *DrLib::Dr2::Values::Player::Pos;;
		return nullptr;//DrLib::Types::Common::Vec3(0, 0, 0);
	}

	return nullptr;// DrLib::Types::Common::Vec3(0, 0, 0);
}

int getMap()
{
	if (Core::Game == Core::DR1)
		return *DrLib::Dr1::Values::Map::CurrentMap;
	if (Core::Game == Core::DR2)
		return *DrLib::Dr2::Values::Map::CurrentMap;

	return -1;
}


int getMovementMode()
{
	if (Core::Game == Core::DR1)
		return (int)*DrLib::Dr1::Values::Player::MovementMode;
	//if (Core::Game == Core::DR2)
	//	return (int)DrLib::Dr2::Values::Player::MovementMode;

	return -1;
}

void copyCharaPos(unsigned int chara, float* dest)
{
	if (Core::Game == Core::DR1)
		return DrLib::Dr1::Funcs::Character::CopyPos(chara, dest);
}

void dr_mp::Client::Init()
{
	SetName("morgana");
}

u_long iMode = 1;


struct timeval tv;

int dr_mp::Client::Connect(const char* addr, int port)
{
	std::cout << "Connecting to " << addr << ":" << port << " ...\n";
	WSADATA wsaData;
	int wsaerr;
	WORD wVersionRequested = MAKEWORD(2, 2);
	wsaerr = WSAStartup(wVersionRequested, &wsaData);

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return 1;
    }

	// https://stackoverflow.com/questions/46811842/timeout-issue-for-connecting-via-blocking-nonblocking-tcp-sockets-in-c-c

	int rc = ioctlsocket(sock, FIONBIO, &iMode);
	if (rc == SOCKET_ERROR)
	{
		std::cerr << "Error init sock" << std::endl;
		closesocket(sock);
		return 1;
	}

    serv_addr.sin_family = AF_INET;
   // if (inet_pton(serv_addr.sin_family, addr, &serv_addr.sin_addr)) {
	long result = inet_addr(addr);

	if (result == INADDR_NONE) {
		std::cerr << "Invalid address/ Address not supported" << std::endl;
		return 1;
	}
	serv_addr.sin_addr.s_addr = result;


    serv_addr.sin_port = htons(port);

    // Connect to server

	rc = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    if (rc == 1 || (rc == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)) {

		tv.tv_sec = 10;
		tv.tv_usec = 0;

		fd_set write, except;

		FD_ZERO(&write);
		FD_SET(sock, &write);

		FD_ZERO(&except);
		FD_SET(sock, &except);

		rc = select(NULL, NULL, &write, &except, &tv);
		if (rc == 0)
		{
			WSASetLastError(WSAETIMEDOUT);
			rc = SOCKET_ERROR;
		}
		else if (rc > 0)
		{
			if (FD_ISSET(sock, &except))
			{
				int err = 0;
				int errsize = sizeof(err);
				getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&err, &errsize);
				WSASetLastError(err);
				rc = SOCKET_ERROR;
			}
			else
				rc = 0;
		}
    }

	if (rc == SOCKET_ERROR)
	{
		closesocket(sock);
		std::cerr << "Connection Failed" << std::endl;
		return 1;
	}

	std::cout << "Connection established to " << addr << ":" << port << " ...\n";
	connected = true;
	verified = false;
	return 0;
}

char packetBuffer[256];

float Distance(float p1[3], float p2[3])
{
	float dist = 0;
	for (int i = 0; i < 3; i++)
		dist += (p1[i] - p2[i]) * (p1[i] - p2[i]);
	return sqrtf(dist);
}

void dr_mp::Client::ReceiveMessage()
{
	int result = recv(sock, packetBuffer, 1, 0);

	if (result == 0)
	{
		Disconnect();
		return;
	}

	if (result == -1)
		return;

	int id = packetBuffer[0];

	if (id < 0 || id >= P_LENGTH)
	{
		std::cerr << "Received invalid packet ID! " << id << std::endl;
		return;
	}

	int numbytes = result;

	int attempts = 0;
	int length = PacketLengths[id];
	while (numbytes < length && attempts < 20)
	{
		int n = recv(sock, packetBuffer + numbytes, length - numbytes, 0);
		if (n == 0)
		{
			Disconnect();
			return;
		}
		if (n != -1)
			numbytes += n;
		attempts++;
	}

	if (numbytes < length || numbytes > length)
	{
		std::cerr << "Incorrect size for packet ID " << id << std::endl;
		return;
	}

	switch (id)
	{
		case P_Init:
		{
			auto packet = (PacketInit*)(packetBuffer);
			client_id = packet->pl_id;
			Players[packet->pl_id].IsClient = true;
			Players[packet->pl_id].Active = true;
			Players[packet->pl_id].SetName(name);
			SendPacket(PacketConnect(client_id, name));
			connected = true;
			verified = true;
			std::cout << "Sent confirmation packet to server\n";// established to " << addr << ":" << port << " ...\n";
			break;
		}
		case P_Connect:
		{
			auto packet = (PacketConnect*)(packetBuffer);
			Players[packet->pl_id].SetName(packet->pl_name);
			Players[packet->pl_id].Active = true;
			Players[packet->pl_id].Map = -1;
			std::cout << "received player info packet of " << packet->pl_name << "(" << packet->pl_id << ")\n";

			break;
		}
		case P_Disconnect:
		{
			auto packet = (PacketDisconnect*)(packetBuffer);
			Players[packet->pl_id].Active = false;
			std::cout << "received player disconnect packet of " << Players[packet->pl_id].Name << "(" << packet->pl_id << ")\n";
			if (packet->pl_id == client_id)
				Disconnect();
			else
				if (Players[packet->pl_id].Map == getMap())
					despawnChar(Players[packet->pl_id].CharID);

			break;
		}
		case P_CamType:
		{
			auto packet = (PacketCamType*)(packetBuffer);
			Players[packet->pl_id].CamType = packet->pl_cam;
			break;
		}
		case P_Map:
		{
			auto packet = (PacketMap*)(packetBuffer);
			int oldmap = Players[packet->pl_id].Map;



			Players[packet->pl_id].Map = packet->pl_map;

			auto pl = Players[packet->pl_id];

			if (oldmap != -1 && oldmap != 0 && oldmap == getMap())
				despawnChar(pl.CharID);

			if (packet->pl_map > 0 && packet->pl_map == getMap())
			{
				createChar(pl.CharID, pl.ExpID, FindFreeRoomPos(packet->pl_map));

				if (pl.CamType == MovementMode::Walk)
					setCharPos(pl.CharID, pl.Pos);
			}

			std::cout << "received player map packet of " << Players[packet->pl_id].Name << "(" << packet->pl_id << ") with map " << packet->pl_map << "!\n";
			break;
		}
		case P_Pos:
		{
			auto packet = (PacketPos*)(packetBuffer);

			// Set old pos to current character position
			copyCharaPos(Players[packet->pl_id].CharID, Players[packet->pl_id].OldPos);

			for (int i = 0; i < 3; i++)
				Players[packet->pl_id].Pos[i] = packet->pl_pos[i];

			if (Players[packet->pl_id].CamType == MovementMode::Room)
				break;


			if (Players[packet->pl_id].Map == getMap() && getMovementMode() == MovementMode::Walk && (Distance(Players[packet->pl_id].Pos, Players[packet->pl_id].OldPos) < 250.0))
			{
				Players[packet->pl_id].Lerp = true;
				Players[packet->pl_id].LerpStart = std::chrono::steady_clock::now();
			}
			else
			{
				Players[packet->pl_id].Lerp = false;
				setCharPos(Players[packet->pl_id].CharID, Players[packet->pl_id].Pos);
				std::cout << "big dist, setting pos";
			}
			break;
		}
		case P_Message:
		{
			auto packet = (PacketMsg*)(packetBuffer);
			std::cout << packet->pl_msg << "\n";
			chat.PushMessage(packet->pl_msg);
			break;
		}
		case P_Kick:
		{
			auto packet = (PacketKick*)(packetBuffer);
			std::cout << "Kicked! Reason: " << packet->k_msg << "\n";
			Disconnect();
			break;
		}

		case P_Chr:
		{
			auto packet = (PacketChr*)(packetBuffer);
			int oldChr = Players[packet->pl_id].CharID;
			Players[packet->pl_id].CharID = packet->pl_char;
			Players[packet->pl_id].ExpID = packet->pl_exp;
			if (Players[packet->pl_id].Map == getMap() && Players[packet->pl_id].Map > 0)
			{
				despawnChar(oldChr);
				createChar(Players[packet->pl_id].CharID, Players[packet->pl_id].ExpID);
				setCharPos(Players[packet->pl_id].CharID, Players[packet->pl_id].Pos);
			}
			break;
		}

		case P_Ping:
		{
			SendPacket(PacketPing());
			break;
		}
	}
}


int requestChara = 0;
int requestExp = 0;
bool needRequestChara = false;

char chatBuffer[128];
bool needSendChat;

const int update_milliseconds = 100;

const float lerp_smooth = (float)(update_milliseconds)/1000.0f;

std::chrono::time_point nextPosUpdate = std::chrono::steady_clock::now();
std::chrono::milliseconds nextPosPeriod = std::chrono::milliseconds(update_milliseconds);

void dr_mp::Client::TickSend()
{
	int newMovement = getMovementMode();
	if (newMovement != mpOldCamType)
	{
		mpOldCamType = newMovement;
		SendPacket(PacketCamType(client_id, mpOldCamType));
	}

	int newMap = getMap();
	bool mapChanged = newMap != mpOldMap;
	//	std::cout << "Map " << newMap << "\n";
	if (mapChanged)
	{
		mpOldMap = newMap;
		SendPacket(PacketMap(client_id, newMap));
		std::cout << "Sending map " << newMap << "\n";

		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			if (Players[i].Map == newMap && newMap > 0 && Players[i].Active && !Players[i].IsClient)
			{
				createChar(Players[i].CharID, Players[i].ExpID);
				setCharPos(Players[i].CharID, Players[i].Pos);
			}
		}
	}


	Vec3* newPos = getPos();
	auto now = std::chrono::steady_clock::now();
	if ( (newMovement == MovementMode::Walk) && (now > nextPosUpdate) && (newMap > 0) && (mapChanged || newPos->x != mpOldPos.x || newPos->y != mpOldPos.y || newPos->z != mpOldPos.z))
	{
		nextPosUpdate = now + nextPosPeriod;
		mpOldPos.x = newPos->x;
		mpOldPos.y = newPos->y;
		mpOldPos.z = newPos->z;
		SendPacket(PacketPos(client_id, mpOldPos.x, mpOldPos.y, mpOldPos.z));
	}

	if (needRequestChara)
	{
		needRequestChara = false;
		SendPacket(PacketChr(client_id, requestChara, requestExp));
	}

	if (needSendChat)
	{
		needSendChat = false;
		SendPacket(PacketMsg(client_id, chatBuffer));
	}
}

float newLerpPos[3];

void dr_mp::Client::TickLerp()
{
	if (getMovementMode() != MovementMode::Walk)
		return;

	auto now = std::chrono::steady_clock::now();

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (!Players[i].Active || !Players[i].Lerp)
			continue;

		if (Players[i].Map != getMap())
			continue;

		auto elapsed = std::chrono::duration<float>(now - Players[i].LerpStart).count();
		
		float t = std::clamp(elapsed / lerp_smooth, 0.0f, 1.0f);

		if (t >= 0.99f)
		{
			Players[i].Lerp = false;
			setCharPos(Players[i].CharID, Players[i].Pos);
			continue;
		}

		for (int x = 0; x < 3; x++)
			newLerpPos[x] = Players[i].OldPos[x] + (t *(Players[i].Pos[x] - Players[i].OldPos[x]));

		setCharPos(Players[i].CharID, newLerpPos);
	}
}

void dr_mp::Client::RequestCharaChange(int c, int e)
{
	if (!verified || !connected)
		return;

	requestChara = c;
	requestExp = e;
	needRequestChara = true;
}

void dr_mp::Client::SendChat(char* msg)
{
	if (!verified || !connected)
		return;
	int len = strlen(msg);
	if (len > 127)
		len = 127;

	for (int i = 0; i < len; i++)
		chatBuffer[i] = msg[i];

	chatBuffer[len] = '\0';

	needSendChat = true;
}

void dr_mp::Client::Tick()
{
	if (connected)
	{
		if (verified)
		{
			TickSend();
			TickLerp();
		}

		ReceiveMessage();
	}
}

void dr_mp::Client::Disconnect()
{
	connected = false;
	closesocket(sock);
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		Players[i].Active = false;
		Players[i].IsClient = false;
	}

	std::cout << "Disconnected from server!\n";
}