#pragma once
#include <WinSock2.h>
#include <Windows.h>

#define MAX_BUFFER_SIZE 1024

// 게임 룰 상수
#define MAX_ROUNDS 5    // 최대 5판
#define ROUNDS_TO_WIN 3    // 3선승
#define FRAMES_PER_SECOND 60  // 초당 60프레임
#define ROUND_DURATION_SEC 60   // 라운드 시간 60초

struct OverlappedEx {
    OVERLAPPED overlapped;
    WSABUF wsabuf;
    char buffer[MAX_BUFFER_SIZE];
    bool isSend;
};

enum class SessionState {
    FREE,
    CONNECTED,
    MATCHING,
    IN_ROOM,
};

class Room;
class ClientSession {
private:
    SOCKET sock;
    sockaddr_in clientAddr;
    SessionState state;
    OVERLAPPED overlapped;
    char recvBuffer[MAX_BUFFER_SIZE];
    int globalid;  // 매칭 매니저에서 부여하는 "고유 ID" (예: 1000,1001,...)
    int roomId;    // Room 내부에서 1 or 2 (인게임 슬롯 번호)
    Room* room;
public:
    ClientSession();
    ~ClientSession() {}

    void Reset();
    SessionState GetSession();
    void SetSession(SessionState chagestate);
    int GetGId();
    void SetGId(int gid);
    Room* GetRoom();
    void SetRoom(Room* room);
    SOCKET& GetSock();
    void SetSock(SOCKET& sock);
    void SetClientAddr(sockaddr_in& addr);
    int GetRoomId();
    void SetRoomId(int rid);
};