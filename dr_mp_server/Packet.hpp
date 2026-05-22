#pragma once
#include <cstring>
enum Packets
{
	P_Init,
	P_Connect,
	P_Disconnect,
	P_Kick,
	P_Pos,
	P_Map,
	P_Message,
	P_CamType,
	P_Chr,
	P_Ping,
	P_LENGTH,
};


struct Packet
{
	char pk_id;
};

struct PacketInit : Packet
{
	int pl_id;

	PacketInit(int id)
	{
		pk_id = P_Init;
		pl_id = id;
	}
};

struct PacketConnect : Packet
{
	int pl_id;
	char pl_name[32];

	PacketConnect(int id, const char name[32])
	{
		pk_id = P_Connect;
		pl_id = id;

		for (int i = 0; i < 32; i++)
			pl_name[i] = name[i];


	}
};

struct PacketDisconnect : Packet
{
	int pl_id;

	PacketDisconnect(int id)
	{
		pk_id = P_Disconnect;
		pl_id = id;
	}
};

struct PacketKick : Packet
{
	char k_msg[128];
	PacketKick(const char* msg)
	{
		pk_id = P_Kick;

		int len = strlen(msg);
		if (len > 127)
			len = 127;

		for (int i = 0; i < len; i++)
			k_msg[i] = msg[i];
	}
};

struct PacketPos : Packet
{
	int pl_id;
	float pl_pos[3];

	PacketPos(int id, float pos[3])
	{
		pk_id = P_Pos;
		pl_id = id;

		for (int i = 0; i < 3; i++)
			pl_pos[i] = pos[i];
	}

	PacketPos(int id, float x, float y, float z)
	{
		pk_id = P_Pos;
		pl_id = id;

		pl_pos[0] = x;
		pl_pos[1] = y;
		pl_pos[2] = z;
	}
};

struct PacketMap : Packet
{
	int pl_id;
	int pl_map;

	PacketMap(int id, int map)
	{
		pk_id = P_Map;
		pl_id = id;
		pl_map = map;
	}
};

struct PacketMsg : Packet
{
	int pl_id;
	char pl_msg[128];

	PacketMsg(int id, const char* msg)
	{
		pk_id = P_Message;
		pl_id = id;

		int length = strlen(msg);
		if (length > 127)
			length = 127;

		for (int i = 0; i < length; i++)
			pl_msg[i] = msg[i];

		pl_msg[length] = '\0';
	}
};

struct PacketCamType : Packet
{
	int pl_id;
	int pl_cam;

	PacketCamType(int id, int cam)
	{
		pk_id = P_CamType;
		pl_id = id;
		pl_cam = cam;
	}
};

struct PacketChr : Packet
{
	int pl_id;
	int pl_char;
	int pl_exp;

	PacketChr(int id, int chr, int exp)
	{
		pk_id = P_Chr;
		pl_id = id;
		pl_char = chr;
		pl_exp = exp;
	}
};

struct PacketPing : Packet
{
	PacketPing()
	{
		pk_id = P_Ping;
	}
};

inline int PacketLengths[P_LENGTH] = {
	sizeof(PacketInit),
	sizeof(PacketConnect),
	sizeof(PacketDisconnect),
	sizeof(PacketKick),
	sizeof(PacketPos),
	sizeof(PacketMap),
	sizeof(PacketMsg),
	sizeof(PacketCamType),
	sizeof(PacketChr),
	sizeof(PacketPing)
};