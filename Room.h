#pragma once

#include <WinSock2.h>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <iostream>
#include "Client.h"

class Client; // ���� ����. �� Ŭ������ ��򰡿� ���ǵǾ� �ִٰ� �˷���. ���� Ÿ������ ��� ����

class Room {
public:
	std::string room_name;
	std::vector<Client*> players;
	std::set<std::string> usedWords;
	std::string lastWord;
	int currentTurnIdx = 0;
	bool isGameStarted = false;

	Room(const std::string& name) : room_name(name) {}

	void Broadcast(const std::string& msg) {
		for (auto player : players)
		{
			if (player) {
				player->Send(msg);
			}
		}
	}
	void SendTo(SOCKET sock, const std::string& msg) {
		for (auto player : players) {
			if (player->sock == sock) {
				player->Send(msg);
				return;
			}
		}
	}
	void AddPlayer(Client* client) {
		players.push_back(client);
		client->currentRoom = this;
		SendTo(client->sock, "���� �� [" + room_name + "]�� �����߽��ϴ�.");
	}

	void RemovePlayer(Client* client) {
		players.erase(std::remove(players.begin(), players.end(), client), players.end());
		Broadcast("�÷��̾� �����");

		if (players.empty()) {
			std::cout << "�� [" << room_name << "] �����" << std::endl;
		}

		if (isGameStarted && currentTurnIdx >= players.size()) {
			currentTurnIdx = 0;
		}

	}
	void StartGame() {
		if (players.size() < 2) {
			Broadcast("���� ������ ���� �ּ� 2���� �ʿ��մϴ�.");
			return;
		}

		isGameStarted = true;
		currentTurnIdx = 0;
		usedWords.clear();
		lastWord.clear();
		Broadcast("������ ���۵Ǿ����ϴ�!");
		PromptNextPlayer();
	}

	void HandleTurn(Client* player, const std::string& word) {
		if (!isGameStarted) {
			SendTo(player->sock, "���� ������ ���۵��� �ʾҽ��ϴ�.");
			return;
		}

		if (players[currentTurnIdx] != player) {
			SendTo(player->sock, "����� ���ʰ� �ƴմϴ�.");
			return;
		}

		if (usedWords.count(word)) {
			SendTo(player->sock, "�̹� ���� �ܾ��Դϴ�.");
			return;
		}

		if (!lastWord.empty() && lastWord.back() != word.front()) {
			SendTo(player->sock, "������ ���� �ʽ��ϴ�. ������ �ܾ�: " + lastWord);
			return;
		}

		usedWords.insert(word);
		lastWord = word;
		Broadcast("[" + std::to_string(player->sock) + "] : " + word);

		currentTurnIdx = (currentTurnIdx + 1) % players.size();
		PromptNextPlayer();
	}


	void PromptNextPlayer()
	{
		Client* next = players[currentTurnIdx];
		SendTo(next->sock, "����� �����Դϴ�.");
	}
	
};