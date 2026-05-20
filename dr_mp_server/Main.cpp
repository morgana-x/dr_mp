// dr_mp_server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Server.hpp"
#include <thread>

Server server;

std::thread thread_s;

void tick_server()
{
    while (true)
    {
        server.Tick();
        Sleep(10);
    }
}

void close_server()
{
    std::cout << "Closing server...\n";
    thread_s.detach();
    server.Close();
}

int main()
{
    int port = 64355;

    std::cout << "Starting server using port " << port << "...\n";

    server.Start(port);

    std::cout << "Starting server tick thread..." << "\n";

    thread_s = std::thread(tick_server);

    char line[64];
    while (1)
    {
        std::cin.getline(line, sizeof(line));
        if (line == "close")
            break;
        server.ChatBroadcast(line);
    }

    close_server();
}

void atexit()
{
    close_server();
}


