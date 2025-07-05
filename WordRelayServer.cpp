#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <format>
#include <set>
#include <string>
#include <unordered_map>
#include <memory>
#include "Room.h"
#include "Client.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int SERVER_PORT = 12345;
const int MAX_CLIENTS = FD_SETSIZE;
constexpr int BUFFER_SIZE = 1024;

// 전역 상태 관리
vector<unique_ptr<Client>> clients;
unordered_map<string, unique_ptr<Room>> rooms; // 클라이언트가 방에 입장
//unordered_map<SOCKET, string> clientToRoom; // 클라이언트가 어느 방에 있는지 추적

int currentTurnIdx = 0; // 현재 턴 플레이어의 인덱스


set<string> usedWords;
string lastWord;
int room_count = 1;

void ConnectClient(SOCKET listenSock)
{
    sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);
    SOCKET clientSock = accept(listenSock, (sockaddr*)&clientAddr, &addrLen);

    if (clients.size() < MAX_CLIENTS) {
        auto newClient = make_unique<Client>(clientSock);
        clients.push_back(std::move(newClient));

        cout << "새 클라이언트 연결됨: " << clientSock << endl;
    }
    else {
        cout << " 최대 연결 수 초과\n";
        closesocket(clientSock);
    }
}
void DisconnectClient(Client* client, vector<unique_ptr<Client>>::iterator& it)

{
    cout << "클라이언트 연결 종료: " << client->GetSocket() << endl;
    closesocket(client->GetSocket());
    it = clients.erase(it);
}
void CreateRoom() {
    string roomName = to_string(room_count);
    rooms.emplace(roomName, make_unique<Room>(roomName));
    cout << room_count << "번 방이 생성되었습니다";
    ++room_count;
}

int main()
{
    // 1. Winsock 초기화
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 2. 서버 소켓 생성
    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(listenSock, SOMAXCONN);
    
    // 3. select용 fd_set 준비
    fd_set readfds;
    

    cout << "서버 시작. 포트: " << SERVER_PORT << endl;

    while (true)
    {
        FD_ZERO(&readfds);
        FD_SET(listenSock, &readfds);
        SOCKET maxSock = listenSock;

        // 클라이언트 소켓들을 fd_set에 추가
        for (const auto& client : clients) {
            SOCKET sock = client->GetSocket();
            FD_SET(sock, &readfds);
            if (sock > maxSock) maxSock = sock;
        }

        // 4. select 호출 (blocking)
        int result = select(0, &readfds, nullptr, nullptr, nullptr); // 윈도우에서는 select의 첫 인자가 의미 없음. 리눅스 호환을 고려

        if (result < 0) {
            cerr << "select() 오류\n";
            break;
        }

        // 5. 새로운 연결 수락
        if (FD_ISSET(listenSock, &readfds)) ConnectClient(listenSock);

        // 각 클라이언트 처리
        char buffer[BUFFER_SIZE] = {};

        for (auto it = clients.begin(); it != clients.end();) {
            Client* client = it->get();
            SOCKET sock = client->GetSocket();

            if (FD_ISSET(sock, &readfds)) {

                // 클라이언트 연결 확인
                int recvLen = recv(sock, buffer, BUFFER_SIZE - 1, 0);
                if (recvLen <= 0) {
                    DisconnectClient(client, it);
                    continue;
                }

                buffer[recvLen] = '\0';
                string word(buffer); // buffer의 첫 글자부터 널 문자가 나올 때까지 복사해서 string 객체 생성
                

                //명령어 처리
                if (word == "enter") { // 방 번호로 입력은 나중에
                    if (rooms.empty()) {
                        CreateRoom();
                    }
                    rooms["1"]->AddPlayer(client);
                }
                else if (word == "start") {
                    if (rooms.contains("1")) {
                        rooms["1"]->StartGame();
                    }
                }
            }
            ++it;
        }
    }

    // 7. 종료 처리
    for (const auto& client : clients) closesocket(client->GetSocket());
    closesocket(listenSock);
    WSACleanup();

    return 0;
}
