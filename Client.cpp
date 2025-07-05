#include "Client.h"
#include "Room.h" // ���⼭ include �ؾ� ��� ����. ������� �ϸ� ��ȯ�� �߻���.

void Client::Send(const std::string& msg) {
	send(sock, msg.c_str(), static_cast<int>(msg.size()), 0);
}

void Client::LeaveRoom() {
	if (currentRoom) {
		currentRoom->RemovePlayer(this);
		currentRoom = nullptr;
	}
}