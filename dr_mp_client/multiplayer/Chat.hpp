#include <list>
#include <string>
#include <algorithm>
#include <sstream>

const int MAX_CHAT_HISTORY = 20;
class Chat
{
public:

	std::string ChatHistory[MAX_CHAT_HISTORY] ;

	int Length() { return MAX_CHAT_HISTORY; }

	Chat()
	{
		for (int i = 0; i < MAX_CHAT_HISTORY; i++)
			ChatHistory[i] = std::string();
	}

	
	void PushMessage(const char* message)
	{
		shift();
		ChatHistory[0].assign(message);
	}

	void PushMessage(std::string str)
	{
		shift();
		ChatHistory[0].assign(str);
	}
private:
	void shift()
	{
		for (int i = MAX_CHAT_HISTORY - 1; i > 0; --i)
			ChatHistory[i] = ChatHistory[i - 1];
	}
};