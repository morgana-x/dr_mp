#include <iostream>
#include "Server.hpp"
#include <thread>
#include <stdio.h>
#include <stdlib.h>
extern "C" {
#include "ext/args/args.h"
}

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

int main(int argc, char** argv)
{
    int port = 64355;

    // Instantiate a new ArgParser instance.
    ArgParser* parser = ap_new_parser();
    if (!parser) {
        exit(1);
    }

    // Register the program's helptext and version number.
    ap_set_helptext(parser, "Usage: dr_mp_server.exe -p 64355");
    ap_set_version(parser, "1.0");

    ap_add_int_opt(parser, "port p", port);

    // Parse the command line arguments.
    if (!ap_parse(parser, argc, argv)) {
        exit(1);
    }

    port = ap_get_int_value(parser, "port");

    std::cout << "Starting server using port " << port << "...\n";

    server.Start(port);

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


