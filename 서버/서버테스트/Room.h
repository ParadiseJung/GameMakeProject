#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include <mutex>

// 라운드 결과를 저장하기 위한 임시 구조
struct PlayerInputData {
    bool valid;
    int  roomId;
    char move;
    char attack;
    char sec;
    char frame;
};

enum class RoundOver : char;
struct RoundOverData {
    bool valid;
    RoundOver state;
};

class ClientSession;
class ReqComPacket;
class ReqRoundOverPacket;
class Room {
private:
    ClientSession* player1;
    ClientSession* player2;

    int scoreP1;
    int scoreP2;
    int currentRound;
    bool roundActive;
    int frameCount;

    // 임시로 두 플레이어의 입력을 저장해둘 수 있는 공간
    PlayerInputData p1Input;
    PlayerInputData p2Input;

    RoundOverData p1RoundOver;
    RoundOverData p2RoundOver;

    std::mutex roomMutex;

public:
    Room();
    ~Room() {}

    void Init(ClientSession* p1, ClientSession* p2);
    // 매 프레임마다 호출
    void OnFrameUpdate();
    // 클라이언트에서 ReqComPacket 수신 시 저장
    void OnReqComPacket(const ReqComPacket& pkt);
    // 라운드 끝났다고 클라이언트가 보냈을 때
    void OnReqRoundOverPacket(const ReqRoundOverPacket& pkt);

private:
    void EndRound(int winnerId);
    void CloseRoom();
    // 패킷 송신 헬퍼
    void SendPacket(SOCKET sock, const char* data, int size);
};