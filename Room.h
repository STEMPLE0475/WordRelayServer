#pragma once

#include <WinSock2.h>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <iostream>
#include "Client.h"

class Client; // 전방 선언. 이 클래스가 어딘가에 정의되어 있다고 알려줌. 참조 타입으로 사용 가능

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
		SendTo(client->sock, "게임 방 [" + room_name + "]에 입장했습니다.");
	}

	void RemovePlayer(Client* client) {
		players.erase(std::remove(players.begin(), players.end(), client), players.end());
		Broadcast("플레이어 퇴장됨");

		if (players.empty()) {
			std::cout << "방 [" << room_name << "] 비었음" << std::endl;
		}

		if (isGameStarted && currentTurnIdx >= players.size()) {
			currentTurnIdx = 0;
		}

	}
	void StartGame() {
		if (players.size() < 2) {
			Broadcast("게임 시작을 위해 최소 2명이 필요합니다.");
			return;
		}

		isGameStarted = true;
		currentTurnIdx = 0;
		usedWords.clear();
		lastWord.clear();
		Broadcast("게임이 시작되었습니다!");
		PromptNextPlayer();
	}

	void HandleTurn(Client* player, const std::string& word) {
		if (!isGameStarted) {
			SendTo(player->sock, "아직 게임이 시작되지 않았습니다.");
			return;
		}

		if (players[currentTurnIdx] != player) {
			SendTo(player->sock, "당신의 차례가 아닙니다.");
			return;
		}

		if (usedWords.count(word)) {
			SendTo(player->sock, "이미 사용된 단어입니다.");
			return;
		}

		if (!lastWord.empty() && lastWord.back() != word.front()) {
			SendTo(player->sock, "끝말이 맞지 않습니다. 마지막 단어: " + lastWord);
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
		SendTo(next->sock, "당신의 차례입니다.");
	}
	
};