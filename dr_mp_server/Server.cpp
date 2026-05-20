#pragma once
#include "Server.hpp"
#include <fcntl.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")
//https://jameshfisher.com/2017/04/05/set_socket_nonblocking/
// https://www.tutorialspoint.com/cplusplus/cpp_socket_programming.htm

u_long iMode = 1;

void Server::Start(int port)
{
	Server::Close();

	WSADATA wsaData;
	int wsaerr;
	WORD wVersionRequested = MAKEWORD(2, 2);
	wsaerr = WSAStartup(wVersionRequested, &wsaData);

	socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_fd == 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	
	ioctlsocket(socket_fd, FIONBIO, &iMode);

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	// Bind
	if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		Players[i].Active = false;
		Players[i].ID = i;
		Players[i].SetName("????");
		Players[i].Map = -1;
	}

	// Listen
	if (listen(socket_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	std::cout << "Server started at port " << port << "\n";
}

void Server::Close()
{
	BroadcastPacket(PacketKick("Server closed"));
}

void Server::Tick()
{
	Server::ReceiveConnections();
	Server::ReceivePackets();
}


int new_socket;
char namebuffer[32];

void Server::ReceiveConnections()
{
	int addrlen = sizeof(address);
	// Accept a connection
	new_socket = accept(socket_fd, (struct sockaddr*)&address, &addrlen);

	if (new_socket < 0)
		return;
	std::cout << "Received new connection request\n";
	int playerID = FindFreeID();

	if (playerID == -1)
	{
		SendSocket(new_socket, PacketKick("Player limit reached!"));
		closesocket(new_socket);
		std::cout << "Couldn't find free iD!\n";
		return;
	}

	Players[playerID].StartConnect(new_socket);
	SendPacket(playerID, PacketInit(playerID));
	std::cout << "Waiting for player connect confirmation from pid: " << playerID << "\n";
}


const int packetBufferLen = 256;
char packetBuffer[packetBufferLen];


void Server::ReceivePacket(int playerId)
{
	if (!Players[playerId].Active)
		return;

	int numbytes = recv(Players[playerId].Socket, packetBuffer, 1, 0);

	if (numbytes == -1)
		return;

	if (numbytes == 0)
	{
		PlayerDisconnect(playerId);
		return;
	}

	int id = packetBuffer[0];

	if (id < 0 || id >= P_LENGTH)
	{
		PlayerKick(playerId, "Received invalid packet ID!");
		return;
	}


	int attempts = 0;
	int length = PacketLengths[id];
	while (numbytes < length && attempts < 20)
	{
		int n = recv(Players[playerId].Socket, (packetBuffer + numbytes), length - numbytes, 0);

		if (n == 0)
		{
			PlayerDisconnect(playerId);
			return;
		}

		if (n > 0)
			numbytes += n;
		attempts++;
	}

	if (numbytes != length)
	{
		PlayerKick(playerId, "Invalid size received for packet");
		std::cout << "Invalid length received for id  " << id << ": " << numbytes << "!=" << length << "!\n";
		return;
	}

	switch (id)
	{
		case P_Connect:
		{
			if (Players[playerId].FinCon)
				break;

			auto packet = (PacketConnect*)(packetBuffer);

			Players[playerId].SetName(packet->pl_name);
			Players[playerId].CharID = FindFreeCharID();
			Players[playerId].ExpID = 0;

			for (int i = 0; i < MAX_PLAYERS; i++)
			{
				if (!Players[i].Active || !Players[i].FinCon)
					continue;

				SendPacket(playerId, PacketConnect(i, Players[i].Name));
				SendPacket(playerId, PacketChr(i, Players[i].CharID, Players[i].ExpID));
				SendPacket(playerId, PacketPos(i, Players[i].Pos));
				SendPacket(playerId, PacketCamType(i, Players[i].CamType));
				SendPacket(playerId, PacketMap(i, Players[i].Map));
			}

			BroadcastPacketExcludePlayer(PacketConnect(playerId, Players[playerId].Name), playerId);
			BroadcastPacket(PacketChr(playerId, Players[playerId].CharID, Players[playerId].ExpID));
			Players[playerId].FinCon = true;

			std::stringstream ss;
			ss << "| + " << Players[playerId].Name << " has connected!";
			const auto tmp = ss.str();
			ChatBroadcast(tmp);
			break;
		}
		case P_Disconnect:
		{
			PlayerDisconnect(playerId);
			break;
		}
		case P_Map:
		{
			auto packet = (PacketMap*)(packetBuffer);

			Players[playerId].Map = packet->pl_map;
			if (Players[playerId].Map > 9999)
				Players[playerId].Map = -1;

			BroadcastPacketExcludePlayer(PacketMap(playerId, Players[playerId].Map), playerId);
			std::cout << Players[playerId].Name << " joined map " << Players[playerId].Map << "\n";
			break;
		}
		case P_CamType:
		{
			auto packet = (PacketCamType*)(packetBuffer);
			Players[playerId].CamType = packet->pl_cam;
			BroadcastPacketExcludePlayer(PacketCamType(playerId, Players[playerId].CamType), playerId);
			break;
		}
		case P_Pos:
		{
			auto packet = (PacketPos*)(packetBuffer);
			for (int i = 0; i < 3; i++)
				Players[playerId].Pos[i] = packet->pl_pos[i];

			BroadcastPacketExcludePlayer(PacketPos(playerId, Players[playerId].Pos), playerId);
			break;
		}
		case P_Message:
		{
			auto packet = (PacketMsg*)(packetBuffer);
			if (strlen(packet->pl_msg) == 0)
				break;

			std::stringstream str;
			str << (Players[playerId].Name) << ": " << packet->pl_msg;
			const std::string tmp = str.str();
			ChatBroadcast(tmp);
			break;
		}
		case P_Chr:
		{
			auto packet = (PacketChr*)(packetBuffer);

			if (packet->pl_char == Players[playerId].CharID && packet->pl_exp == Players[playerId].ExpID)
			{
				ChatSend(playerId, "You already have this model!");
				break;
			}

			if (!IsCharIDFree(packet->pl_char, playerId))
			{
				SendPacket(playerId, PacketChr(playerId, Players[playerId].CharID, Players[playerId].ExpID));
				ChatSend(playerId, "That character id is taken!!!");
				break;
			}

			if (!ValidExpression(packet->pl_char, packet->pl_exp))
			{
				ChatSend(playerId, "Invalid expresion id / expression id too high for that character");
				SendPacket(playerId, PacketChr(playerId, Players[playerId].CharID, Players[playerId].ExpID));
				break;
			}
			Players[playerId].CharID = packet->pl_char;
			Players[playerId].ExpID = packet->pl_exp;
			BroadcastPacket(PacketChr(playerId, Players[playerId].CharID, Players[playerId].ExpID));
			break;
		}
	}

}

void Server::ReceivePackets()
{
	for (int i = 0; i < MAX_PLAYERS; i++)
		if (Players[i].Active)
			ReceivePacket(i);
}