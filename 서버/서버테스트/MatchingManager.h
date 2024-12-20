#pragma once
#include <WinSock2.h>
#include <Windows.h>
#include <mutex>
#include <queue>
#include <iostream>

class Room;
class ClientSession;
class MatchingManager {
private:
    std::mutex matchMutex;
    std::queue<ClientSession*> waitingQueue;
    int globalIdCounter = 1001;
public:
    Room* MatchOrWait(ClientSession* session);
    bool RemoveFromQueue(ClientSession* session);
};