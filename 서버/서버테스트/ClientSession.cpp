#include "ClientSession.h"

ClientSession::ClientSession() : sock(INVALID_SOCKET), state(SessionState::FREE), room(nullptr), roomId(0), globalid(0)
{
    ZeroMemory(&clientAddr, sizeof(clientAddr));
    ZeroMemory(&overlapped, sizeof(overlapped));
    ZeroMemory(recvBuffer, MAX_BUFFER_SIZE);
}

void ClientSession::Reset()
{
    sock = INVALID_SOCKET;
    ZeroMemory(&clientAddr, sizeof(clientAddr));
    ZeroMemory(&overlapped, sizeof(overlapped));
    ZeroMemory(recvBuffer, MAX_BUFFER_SIZE);
    state = SessionState::FREE;
    room = nullptr;
    roomId = 0;
    globalid = 0;
}

SessionState ClientSession::GetSession()
{
    return state;
}

void ClientSession::SetSession(SessionState changestate)
{
    state = changestate;
}

int ClientSession::GetGId()
{
    return globalid;
}

void ClientSession::SetGId(int gId)
{
    globalid = gId;
}

Room* ClientSession::GetRoom()
{
    return room;
}

void ClientSession::SetRoom(Room* Room)
{
    room = Room;
}

SOCKET& ClientSession::GetSock()
{
    return sock;
}

void ClientSession::SetSock(SOCKET& Sock)
{
    sock = Sock;
}

void ClientSession::SetClientAddr(sockaddr_in& addr)
{
    clientAddr = addr;
}

int ClientSession::GetRoomId()
{
    return roomId;
}

void ClientSession::SetRoomId(int rid)
{
    roomId = rid;
}
