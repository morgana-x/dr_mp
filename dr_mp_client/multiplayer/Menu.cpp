#include "Menu.hpp"
#include "../Core.hpp"
#include "imgui.h"
#include "string.h"
#include <sstream>
char menu_connect_ip[256] = "127.0.0.1";
int menu_connect_port = 64355;

char client_name[32] = "Makoto Naegi";


int menu_chara = 0;
int menu_expr = 0;

char menu_chat_msg[128];

void menu_playerlist()
{

	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin("Server");
	if (!Core::Client.connected)
	{
		ImGui::BeginChild("Server Connect");
		ImGui::InputText("Server Addr", menu_connect_ip, 256);
		ImGui::InputInt("Server Port", &menu_connect_port);
		ImGui::InputText("User name", client_name, 32);
		if (ImGui::Button("Connect to server"))
		{
			Core::Client.SetName(client_name);
			Core::Client.Connect(menu_connect_ip, menu_connect_port);
		}
		ImGui::EndChild();
	}
	else
	{
		ImGui::InputInt("Character", &menu_chara);
		ImGui::InputInt("Expression", &menu_expr);
		if (ImGui::Button("Request character model"))
			Core::Client.RequestCharaChange(menu_chara, menu_expr);

		if (ImGui::Button("Disconnect"))
			Core::Client.Disconnect();

	}
	ImGui::End();

	std::stringstream st;
	ImGui::SetNextWindowPos(ImVec2(0, 400), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin("Players");

	for (int i = 0; i < dr_mp::MAX_PLAYERS; i++)
	{
		if (!Core::Client.Players[i].Active)
			continue;

		st.str(std::string());
		st  << Core::Client.Players[i].Name << "##" << i;
		std::string id_str = st.str();
		const char* id_str_str = id_str.c_str();

		if (ImGui::CollapsingHeader(id_str_str))
		{

			ImGui::Value("Map", Core::Client.Players[i].Map);


			ImGui::Value("Chr", Core::Client.Players[i].CharID);

			ImGui::Value("Chr_Exp", Core::Client.Players[i].ExpID);
		}
	}


	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(500, 0), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin("Chat");
	ImGui::BeginListBox("##Chat_History");
	for (int i = Core::Client.chat.Length() - 1; i >= 0; i--)
	{
		if (Core::Client.chat.ChatHistory[i].length() == 0)
			continue;
		auto msg = Core::Client.chat.ChatHistory[i].c_str();
		ImGui::Text("%s", msg);
	}

	ImGui::EndListBox();

	if (ImGui::Button("Send Message"))
	{
		Core::Client.SendChat(menu_chat_msg);
		menu_chat_msg[0] = '\0';
	}
	ImGui::SameLine();
	ImGui::InputText("##Message_INP", menu_chat_msg, 128);

	ImGui::End();

}

void Menu::Render()
{
	menu_playerlist();
}