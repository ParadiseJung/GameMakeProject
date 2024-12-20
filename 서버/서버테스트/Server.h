#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <Windows.h>
#include <MSWSock.h>
#include <process.h>
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <queue>
#include <chrono>
#include <memory>
#include <cstring>

#include "MatchingManager.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

// 서버 구성 상수
#define SERVER_PORT 9001
#define MAX_WORKER_THREADS 4

class Room;
class ClientSession;
// ----------------------------------------------------------------------------
// GameServer 클래스
// ----------------------------------------------------------------------------
class GameServer {

private:
    SOCKET listenSocket;
    HANDLE iocpHandle;
    std::atomic<bool> serverRunning;
    int workerThreadCount;

    std::thread acceptThread;
    std::vector<std::thread> workerThreads;

    std::mutex sessionsMutex;
    std::unordered_map<int, ClientSession*> sessions;
    std::atomic<int> sessionCounter;

    MatchingManager matchingManager;

    std::mutex roomsMutex;
    std::vector<Room*> rooms;

public:
    GameServer();
    ~GameServer();

    bool Initialize(int port = SERVER_PORT, int workerThreadCount = MAX_WORKER_THREADS);
    void Run();
    void Stop();

private:
    void MainLoop();
    void UpdateRooms();
    bool BindAndListen(int port);
    void AcceptThreadLoop();
    // Worker 스레드: 패킷 수신 처리
    void WorkerThreadLoop();
    // 패킷 파싱 및 처리
    void HandleIncomingPacket(ClientSession* session, char* buffer, int bytesTransferred);
};
