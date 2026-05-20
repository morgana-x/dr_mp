#pragma once
#include <winsock2.h>
#include "Packet.hpp"
#include "Player.hpp"
#include <sstream>
#include <string>
#include <iostream>
#include "Chara.hpp"

#pragma comment(lib, "Ws2_32.lib")
const int MAX_PLAYERS = 17;



class Server
{
	public:
		Player Players[MAX_PLAYERS];
		void Start(int port = 65355);
		void Close();
		void Tick();


		template<typename T>
		int SendSocket(SOCKET socket, T packet) {
			int length = sizeof(packet);
			int numbytes = 0;
			int attempts = 0;
			T copy = packet;
			while (attempts < 20 && numbytes < length)
			{
				attempts++;

				int n = send(socket, (char*)(&copy + numbytes), length - numbytes, 0);
				if (n == 0)
					return 1;

				if (n > 0)
					numbytes += n;
			}

			if (numbytes != length)
			{
				std::cout << "Failed to send packet " << (int)((char*)(&packet))[0] << " length mistmatch: " << numbytes  << " != " << length << "\n";
				return 1;
			}

			return 0;
		//	send(socket, (char *)&packet, sizeof(packet), 0);
		};

		template<typename T>
		void SendPacket(int id, T packet) {
			if (Players[id].Active)
				if (SendSocket(Players[id].Socket, packet))
					Players[id].CloseConnection();
		};

		template<typename T>
		void BroadcastPacket(T packet)
		{
			for (int i = 0; i < MAX_PLAYERS; i++)
				SendPacket(i, packet);
		};

		template<typename T>
		void BroadcastPacketExcludePlayer(T packet, int excludedPlayer)
		{
			for (int i = 0; i < MAX_PLAYERS; i++)
				if ( i != excludedPlayer)
					SendPacket(i, packet);
		};

		void ChatSend(int id, std::string msg)
		{
			const char* c_str = msg.c_str();
			SendPacket(id, PacketMsg(0xFF, c_str));
		}

		void ChatBroadcast(std::string msg)
		{
			const char* c_str = msg.c_str();
			auto packet = PacketMsg(0xFF, c_str);
			BroadcastPacket(packet);

			std::cout << msg << "\n";
		}

		void PlayerDisconnect(int id)
		{
			SendPacket(id, PacketDisconnect(id));
			Players[id].CloseConnection();
			BroadcastPacket(PacketDisconnect(id));

			std::stringstream ss;
			ss << "| - " << Players[id].Name << " has disconnected!";
			const auto tmp = ss.str();
			ChatBroadcast(tmp);
		}

		void PlayerKick(int id, const char* reason)
		{
			SendPacket(id, PacketKick(reason));
			PlayerDisconnect(id);
		}
	
	private: 
		sockaddr_in address;
		int socket_fd;

		void ReceivePacket(int id);

		void ReceiveConnections();
		void ReceivePackets();

		int FindFreeID()
		{
			for (int i = 0; i < MAX_PLAYERS; i++)
				if (!Players[i].Active)
					return i;
			return -1;
		}

		int IsCharIDFree(int chara, int forPlayer = -1)
		{
			for (int x = 0; x < MAX_PLAYERS; x++)
			{
				if (x == forPlayer)
					continue;
				if (!Players[x].Active)
					continue;
				if (Players[x].CharID == chara)
					return false;
			}
			return true;
		}

		int FindFreeCharID()
		{
			for (int i = 0; i <= MAX_PLAYERS; i++)
			{
				if (IsCharIDFree(i))
					return i;
			}
			return -1;
		}

		int ValidExpression(int chara, int exp)
		{
			if (exp > MaxExpressions[chara])
				return 0;
			return 1;
		}
};