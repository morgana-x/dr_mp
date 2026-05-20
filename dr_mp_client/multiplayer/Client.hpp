#pragma once
#include "Player.hpp"
#include "Packet.hpp"
#include "../pch.h"
#include <iostream>
#include "Chat.hpp"

namespace dr_mp
{
	const int MAX_PLAYERS = 17;
	class Client
	{
		public:
			int sock;


			Player Players[MAX_PLAYERS];

			Client()
			{
				client_id = -1;
				connected = false;
				verified = false;
				for (int i = 0; i < MAX_PLAYERS; i++)
				{
					Players[i].ID = i;
					Players[i].Map = -1;
					Players[i].Active = false;
					Players[i].IsClient = false;
				}

				chat = Chat();
				Init();
			};

			int client_id;

			char name[32];

			bool connected;
			bool verified;

			Chat chat;

			void Init();

			int Connect(const char* addr, int port = 64355);

			void Tick();

			void Disconnect();

			void ReceiveMessage();

			void SendChat(char* message);

			void RequestCharaChange(int c, int e);

			void SetName(char* n)
			{
				int len = strlen(n);
				if (len > 32)
					len = 32;
				for (int i = 0; i < len; i++)
					name[i] = n[i];
				name[len] = '\0';
			}

			void SetName(const char* n)
			{
				int len = strlen(n);
				if (len > 32)
					len = 32;
				for (int i = 0; i < 32; i++)
					name[i] = n[i];
			}

			template<typename T>
			void SendPacket(T packet)
			{
				int length = sizeof(packet);
				int numbytes = 0;
				int attempts = 0;

				T copy = packet;
				while (attempts < 20 && numbytes < length)
				{
					attempts++;

					int n = send(sock, (char*)(&copy + numbytes), length - numbytes, 0);
					if (n == 0)
					{
						Disconnect();
						return;
					}
					if (n > 0)
						numbytes += n;

				}

				if (numbytes != length)
				{
					std::cout << "Failed to send packet " << (int)((char*)(&packet))[0] << " length mistmatch: " << numbytes << " != " << length << "\n";
					Disconnect();
				}
			};

			sockaddr_in serv_addr;

	private:
		void TickSend();
	};
}