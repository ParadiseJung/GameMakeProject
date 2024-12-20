#include "Server.h"
#include "ClientSession.h"
#include "Room.h"

#include "Packet.h"
#include "ReqComPacket.h"
#include "ResComPacket.h"
#include "ReqRoundOverPacket.h"
#include "ResRoundOverPacket.h"
#include "ResStartPacket.h"
#include "ResEndPacket.h"
#include "ReqQuitPacket.h"


// Packet 헤더로부터 해당 Packet 객체를 생성
std::unique_ptr<Packet> CreatePacketByHeader(PacketHeader header)
{
    switch (header) {
    case PacketHeader::RES_START:  // 추가
        return std::make_unique<ResStartPacket>('0');
    case PacketHeader::REQ_COM:
        return std::make_unique<ReqComPacket>('0', '0', '0', '0', '0');
        // 임시 인자로 생성하되, deserialize()에서 실제 값이 덮어씌워짐
    case PacketHeader::RES_COM:
        return std::make_unique<ResComPacket>('0', '0', '0', '0', '0', '0', '0', '0');
    case PacketHeader::REQ_RO:
        return std::make_unique<ReqRoundOverPacket>('0', RoundOver::None);
    case PacketHeader::RES_RO:
        return std::make_unique<ResRoundOverPacket>('0', RoundOver::None, '0', RoundOver::None, '0');
    case PacketHeader::RES_END:
        return std::make_unique<ResEndPacket>('0');
    case PacketHeader::REQ_QUIT: // 추가
        return std::make_unique<ReqQuitPacket>();
    default:
        // 필요 시 다른 케이스 혹은 에러 처리
        return nullptr;
    }
}

GameServer::GameServer() : listenSocket(INVALID_SOCKET), iocpHandle(nullptr), serverRunning(false), sessionCounter(0)
{
}

GameServer::~GameServer()
{
	Stop();
}

bool GameServer::Initialize(int port, int WorkerThreadCount)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cout << "WSAStartup failed!\n";
        return false;
    }

    listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (listenSocket == INVALID_SOCKET) {
        std::cout << "WSASocket failed, err=" << WSAGetLastError() << "\n";
        return false;
    }
    if (!BindAndListen(port)) {
        return false;
    }

    iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (iocpHandle == NULL) {
        std::cout << "CreateIoCompletionPort failed!\n";
        return false;
    }

    workerThreadCount = WorkerThreadCount;
    return true;
}

void GameServer::Run()
{
    serverRunning = true;

    // 워커 스레드들
    for (int i = 0; i < workerThreadCount; ++i) {
        workerThreads.emplace_back(&GameServer::WorkerThreadLoop, this);
    }

    // Accept 스레드
    acceptThread = std::thread(&GameServer::AcceptThreadLoop, this);

    // 메인 루프(룸 프레임 업데이트)
    MainLoop();
}

void GameServer::Stop()
{
    if (!serverRunning) return;
    serverRunning = false;

    // 리슨 소켓 닫기
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }

    // 스레드 종료 대기
    if (acceptThread.joinable()) {
        acceptThread.join();
    }
    for (auto& t : workerThreads) {
        if (t.joinable()) t.join();
    }

    // IOCP 핸들 닫기
    if (iocpHandle) {
        CloseHandle(iocpHandle);
        iocpHandle = nullptr;
    }

    // 세션 정리
    {
        std::lock_guard<std::mutex> lock(sessionsMutex);
        for (auto& kv : sessions) {
            ClientSession* s = kv.second;
            if (s) {
                closesocket(s->GetSock());
                s->Reset();
                delete s;
            }
        }
        sessions.clear();
    }

    // 룸 정리
    {
        std::lock_guard<std::mutex> lock(roomsMutex);
        for (auto room : rooms) {
            delete room;
        }
        rooms.clear();
    }

    WSACleanup();
    std::cout << "[GameServer] Server Close\n";
}

void GameServer::MainLoop()
{
    using namespace std::chrono;
    auto prevTime = steady_clock::now();

    while (serverRunning) {
        auto curTime = steady_clock::now();
        auto elapsed = duration_cast<milliseconds>(curTime - prevTime).count();
        if (elapsed >= 16) { // 대략 60fps
            UpdateRooms();
            prevTime = curTime;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void GameServer::UpdateRooms()
{
    std::lock_guard<std::mutex> lock(roomsMutex);
    for (auto& room : rooms) {
        room->OnFrameUpdate();
    }
}

bool GameServer::BindAndListen(int port)
{
    SOCKADDR_IN serverAddr;
    ZeroMemory(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "bind error, err=" << WSAGetLastError() << "\n";
        return false;
    }
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "listen error, err=" << WSAGetLastError() << "\n";
        return false;
    }
    std::cout << "[GameServer] Listening on port " << port << "...\n";
    return true;
}

void GameServer::AcceptThreadLoop()
{
    SOCKADDR_IN clientAddr;
    int addrLen = sizeof(clientAddr);

    while (serverRunning) {
        SOCKET clientSock = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
        if (!serverRunning) break;
        if (clientSock == INVALID_SOCKET) {
            std::cout << "accept failed, err=" << WSAGetLastError() << "\n";
            continue;
        }

        u_long mode = 1;
        ioctlsocket(clientSock, FIONBIO, &mode);

        ClientSession* newSession = new ClientSession();
        newSession->SetSock(clientSock);
        newSession->SetClientAddr(clientAddr);
        newSession->SetSession(SessionState::CONNECTED);

        {
            std::lock_guard<std::mutex> lock(sessionsMutex);
            sessions[sessionCounter++] = newSession;
        }

        CreateIoCompletionPort((HANDLE)clientSock, iocpHandle, (ULONG_PTR)newSession, 0);

        OverlappedEx* recvOver = new OverlappedEx;
        ZeroMemory(recvOver, sizeof(OverlappedEx));
        recvOver->isSend = false;
        recvOver->wsabuf.buf = recvOver->buffer;
        recvOver->wsabuf.len = MAX_BUFFER_SIZE;

        DWORD flags = 0;
        WSARecv(newSession->GetSock(), &recvOver->wsabuf, 1, NULL, &flags,
            (LPWSAOVERLAPPED)recvOver, NULL);

        std::cout << "[AcceptThread] Client Accept, ";

        // 매칭 시도
        Room* joinedRoom = matchingManager.MatchOrWait(newSession);
        if (joinedRoom) {
            std::lock_guard<std::mutex> lock(roomsMutex);
            rooms.push_back(joinedRoom);
            std::cout << "[AcceptThread] ID=" << newSession->GetGId() << " Matching Complete\n";
        }
    }
}

void GameServer::WorkerThreadLoop()
{
    DWORD bytesTransferred = 0;
    ULONG_PTR completionKey = 0;
    OverlappedEx* pOverlappedEx = nullptr;

    while (serverRunning) {
        BOOL ret = GetQueuedCompletionStatus(
            iocpHandle,
            &bytesTransferred,
            &completionKey,
            (LPOVERLAPPED*)&pOverlappedEx,
            INFINITE
        );

        if (!serverRunning) break;

        if (!ret) {
            if (pOverlappedEx != nullptr) {
                ClientSession* session = reinterpret_cast<ClientSession*>(completionKey);
                if (session) {
                    closesocket(session->GetSock());
                    session->Reset();
                }
            }
            continue;
        }

        if (bytesTransferred == 0) {
            ClientSession* session = reinterpret_cast<ClientSession*>(completionKey);
            if (session) {
                std::cout << "[WorkerThread] Client Disconnected, ID=" << session->GetGId() << "\n";
                closesocket(session->GetSock());
                session->Reset();
            }
            continue;
        }

        ClientSession* session = reinterpret_cast<ClientSession*>(completionKey);
        if (!session) {
            if (pOverlappedEx) delete pOverlappedEx;
            continue;
        }

        if (!pOverlappedEx->isSend) {
            // ---------------------------
            // **패킷 처리**: pOverlappedEx->buffer 에 bytesTransferred 만큼의 데이터
            // ---------------------------
            HandleIncomingPacket(session, pOverlappedEx->buffer, bytesTransferred);

            // 다음 수신 대기
            ZeroMemory(pOverlappedEx->buffer, MAX_BUFFER_SIZE);
            pOverlappedEx->wsabuf.buf = pOverlappedEx->buffer;
            pOverlappedEx->wsabuf.len = MAX_BUFFER_SIZE;
            DWORD flags = 0;
            WSARecv(session->GetSock(), &pOverlappedEx->wsabuf, 1, NULL, &flags,
                (LPWSAOVERLAPPED)pOverlappedEx, NULL);
        }
        else {
            // 송신 완료 -> OverlappedEx 해제
            delete pOverlappedEx;
        }
    }
}

void GameServer::HandleIncomingPacket(ClientSession* session, char* buffer, int bytesTransferred)
{
    // 최소한의 유효성 검사: length, delimiter
    if (bytesTransferred < sizeof(short) + sizeof(PacketHeader) + sizeof(short)) {
        std::cout << "[Server] Wrong Packet(Too short Length)\n";
        closesocket(session->GetSock());
        session->Reset();
        return;
    }

    // 첫 2바이트: length, 다음 1바이트: header(실제는 char이지만 enum class라 sizeof=1), 마지막 2바이트 delimiter

    short length;
    std::memcpy(&length, buffer, sizeof(length));
    if (length != bytesTransferred) {
        std::cout << "[Server] Packet Length Incomplete. length=" << length << ", actual=" << bytesTransferred << "\n";
        closesocket(session->GetSock());
        session->Reset();
        return;
    }

    PacketHeader header;
    std::memcpy(&header, buffer + sizeof(length), sizeof(header));

    // 해당 header에 맞는 Packet 객체를 생성
    std::unique_ptr<Packet> pkt = CreatePacketByHeader(header);
    if (!pkt) {
        std::cout << "[Server] Unknown Packet Header\n";
        closesocket(session->GetSock());
        session->Reset();
        return;
    }

    // deserialize
    pkt->deserialize(buffer);

    // 패킷 헤더에 따라 처리 분기
    switch (header) {
    case PacketHeader::REQ_COM: {
        if (session->GetRoom() == nullptr) {
            std::cout << "[Server] Session has no Room\n";
            return; // 매칭 전 패킷이면 무시 or 에러 처리
        }
        ReqComPacket* req = dynamic_cast<ReqComPacket*>(pkt.get());
        if (req) {
            session->GetRoom()->OnReqComPacket(*req);
        }
    } break;
    case PacketHeader::REQ_RO: {
        if (session->GetRoom() == nullptr) {
            std::cout << "[Server] Session has no Room\n";
            return; // 매칭 전 패킷이면 무시 or 에러 처리
        }
        ReqRoundOverPacket* req = dynamic_cast<ReqRoundOverPacket*>(pkt.get());
        if (req) {
            session->GetRoom()->OnReqRoundOverPacket(*req);
        }
    } break;
    case PacketHeader::REQ_QUIT: {
        ReqQuitPacket* req = dynamic_cast<ReqQuitPacket*>(pkt.get());
        if (req) {
            std::cout << "[Server] Received ReqQuitPacket from ID="
                << session->GetGId() << "\n";

            // 1) MatchingManager 큐에서 제거
            bool wasInQueue = matchingManager.RemoveFromQueue(session);
            if (wasInQueue) {
                std::cout << "[Server] Session ID=" << session->GetGId()
                    << " removed from waitingQueue.\n";
            }

            // 2) 세션 목록에서 제거
            {
                std::lock_guard<std::mutex> lock(sessionsMutex);
                sessions.erase(session->GetGId());
            }

            std::cout << "[Server] Client ID=" << session->GetGId()
                << " fully disconnected.\n";

            // 3) 소켓 닫기, 세션 Reset
            closesocket(session->GetSock());
            session->Reset();

            // 세션 메모리 해제
            delete session;
        }
    }
    default:
        // 필요 시 다른 헤더도 처리
        break;
    }
}