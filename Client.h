#pragma once

#include <WinSock2.h>
#include <string>

class Room;

class Client {
public:
	SOCKET sock;
	std::string name;
	Room* currentRoom = nullptr;

	Client(SOCKET s, const std::string& n) : sock(s), name(n) {}
	Client(SOCKET s) : sock(s), name("unnamed") {}  // �ӽ� �߰� ������

	SOCKET GetSocket() const { return sock; }

	void Send(const std::string& msg);

	void LeaveRoom();
};