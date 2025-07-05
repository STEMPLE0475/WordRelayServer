#include "Client.h"
#include "Room.h" // 여기서 include 해야 사용 가능. 헤더에서 하면 순환이 발생함.

void Client::Send(const std::string& msg) {
	send(sock, msg.c_str(), static_cast<int>(msg.size()), 0);
}

void Client::LeaveRoom() {
	if (currentRoom) {
		currentRoom->RemovePlayer(this);
		currentRoom = nullptr;
	}
}