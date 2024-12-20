#include "Room.h"
#include "ClientSession.h"

#include "Packet.h"
#include "ReqComPacket.h"
#include "ResComPacket.h"
#include "ReqRoundOverPacket.h"
#include "ResRoundOverPacket.h"
#include "ResStartPacket.h"
#include "ResEndPacket.h"

Room::Room()
{
    player1 = nullptr;
    player2 = nullptr;
    scoreP1 = 0;
    scoreP2 = 0;
    currentRound = 0;
    frameCount = 0;
    roundActive = false;
}

void Room::Init(ClientSession* p1, ClientSession* p2)
{
    player1 = p1;
    player2 = p2;
    player1->SetRoom(this);
    player1->SetRoomId(1);
    player2->SetRoom(this);
    player2->SetRoomId(2);

    p1Input.roomId = 1;
    p1Input.valid = false;

    p2Input.roomId = 2;
    p2Input.valid = false;
    scoreP1 = 0;
    scoreP2 = 0;
    currentRound = 0;
    frameCount = 0;
    roundActive = true;
    p1Input.valid = false;
    p2Input.valid = false;

    // 클라이언트에게 roomId를 알리는 패킷 전송
    ResStartPacket roomJoinP1((char)1);
    char buf1[1024] = { 0 };
    roomJoinP1.serialize(buf1);
    int size1 = roomJoinP1.getLength();
    SendPacket(player1->GetSock(), buf1, size1);

    ResStartPacket roomJoinP2((char)2);
    char buf2[1024] = { 0 };
    roomJoinP2.serialize(buf2);
    int size2 = roomJoinP2.getLength();
    SendPacket(player2->GetSock(), buf2, size2);
}

void Room::OnFrameUpdate()
{
    if (!roundActive) return;

    frameCount++;
    if (frameCount >= FRAMES_PER_SECOND * ROUND_DURATION_SEC) {
        // 라운드 타임아웃
        //EndRound(-1);
    }
}

void Room::OnReqComPacket(const ReqComPacket& pkt)
{
    std::lock_guard<std::mutex> lock(roomMutex);

    if (!player1 || !player2) return;

    // pNum 필드는 "1 or 2" (인게임 룸 ID)
    char pNum = pkt.getpNum();

    if (pNum == (char)player1->GetRoomId()) {
        p1Input.valid = true;
        p1Input.roomId = player1->GetRoomId();
        p1Input.move = pkt.getMove();
        p1Input.attack = pkt.getAttack();
        p1Input.sec = pkt.getSec();
        p1Input.frame = pkt.getFrame();
    }
    else if (pNum == (char)player2->GetRoomId()) {
        p2Input.valid = true;
        p2Input.roomId = player2->GetRoomId();
        p2Input.move = pkt.getMove();
        p2Input.attack = pkt.getAttack();
        p2Input.sec = pkt.getSec();
        p2Input.frame = pkt.getFrame();
    }

    /*if (!(p1Input.sec == p2Input.sec && p1Input.frame == p2Input.frame))
        //추가 프레임 차이 처리 로직
    */

    // 두 클라이언트 모두 입력 받았으면 ResComPacket 만들기
    if (p1Input.valid && p2Input.valid) {
        if ((p1Input.sec == p2Input.sec && p1Input.frame == p2Input.frame)) 
        {
            ResComPacket resPkt(
                (char)p1Input.roomId, p1Input.move, p1Input.attack,
                (char)p2Input.roomId, p2Input.move, p2Input.attack,
                p1Input.sec, p1Input.frame
            );

            char sendBuf[1024] = { 0 };
            resPkt.serialize(sendBuf);
            int totalSize = resPkt.getLength();

            SendPacket(player1->GetSock(), sendBuf, totalSize);
            SendPacket(player2->GetSock(), sendBuf, totalSize);

            p1Input.valid = false;
            p2Input.valid = false;
        }
    }
}

void Room::OnReqRoundOverPacket(const ReqRoundOverPacket& pkt)
{
    std::lock_guard<std::mutex> lock(roomMutex);
    if (!player1 || !player2) return;

    char pNum = pkt.getpNum();    // 1 또는 2
    RoundOver r = pkt.getROState(); // Win, Lose, Draw, 등

    // 플레이어 1,2에 따른 라운드 종료 상태 저장
    if (pNum == (char)player1->GetRoomId()) {
        p1RoundOver.valid = true;
        p1RoundOver.state = r;
        std::cout << "[Room] P1 round over state: " << (int)r << std::endl;
    }
    else if (pNum == (char)player2->GetRoomId()) {
        p2RoundOver.valid = true;
        p2RoundOver.state = r;
        std::cout << "[Room] P2 round over state: " << (int)r << std::endl;
    }

    // 양쪽 모두 라운드 보고가 끝났는지 확인
    if (p1RoundOver.valid && p2RoundOver.valid) {
        // 여기서 최종 판단
        if (p1RoundOver.state == RoundOver::Win && p2RoundOver.state == RoundOver::Lose) {
            // P1 승
            scoreP1++;
            EndRound(player1->GetGId());
        }
        else if (p1RoundOver.state == RoundOver::Lose && p2RoundOver.state == RoundOver::Win) {
            // P2 승
            scoreP2++;
            EndRound(player2->GetGId());
        }
        else if (p1RoundOver.state == RoundOver::Draw && p2RoundOver.state == RoundOver::Draw) {
            // 무승부 -> 양쪽 점수 +1
            scoreP1++;
            scoreP2++;
            std::cout << "[Room] Round Draw!" << std::endl;
            EndRound(-1);  // winnerId=-1로 표시
        }
        else {
            // 그 외 경우(Win vs Win, Lose vs Lose, Win vs Draw 등)는 로직상 에러 처리
            std::cout << "[Room] RoundOver mismatch or unknown combination!\n";
            // 에러 응답 후 룸 종료
            CloseRoom();
            return;
        }
    }
    // else 아직 반대편 클라이언트가 라운드 종료 패킷 안 보냄 -> 대기 상태
}

void Room::EndRound(int winnerId)
{
    roundActive = false;
    currentRound++;
    frameCount = 0;

    if (winnerId == -1) {
        char checkState = 1; // 1=정상 종료
        ResRoundOverPacket resPkt(
            (char)player1->GetRoomId(), (RoundOver::Draw),
            (char)player2->GetRoomId(), (RoundOver::Draw),
            checkState
        );

        char sendBuf[1024] = { 0 };
        resPkt.serialize(sendBuf);
        int totalSize = resPkt.getLength();

        if (player1->GetSock() != INVALID_SOCKET) {
            SendPacket(player1->GetSock(), sendBuf, totalSize);
        }
        if (player2->GetSock() != INVALID_SOCKET) {
            SendPacket(player2->GetSock(), sendBuf, totalSize);
        }

        // 다음 라운드를 위해 다시 valid 상태를 리셋
        p1RoundOver.valid = false;
        p1RoundOver.state = RoundOver::None;
        p2RoundOver.valid = false;
        p2RoundOver.state = RoundOver::None;
    }
    else {
        std::cout << "[Room] Round Winner: " << winnerId << "\n";
        // 라운드 종료 정보를 다시 두 클라이언트에게 보내기
        char checkState = 1; // 1=정상 종료
        ResRoundOverPacket resPkt(
            (char)player1->GetRoomId(), (p1RoundOver.state),
            (char)player2->GetRoomId(), (p2RoundOver.state),
            checkState
        );

        char sendBuf[1024] = { 0 };
        resPkt.serialize(sendBuf);
        int totalSize = resPkt.getLength();

        if (player1->GetSock() != INVALID_SOCKET) {
            SendPacket(player1->GetSock(), sendBuf, totalSize);
        }
        if (player2->GetSock() != INVALID_SOCKET) {
            SendPacket(player2->GetSock(), sendBuf, totalSize);
        }

        // 다음 라운드를 위해 다시 valid 상태를 리셋
        p1RoundOver.valid = false;
        p1RoundOver.state = RoundOver::None;
        p2RoundOver.valid = false;
        p2RoundOver.state = RoundOver::None;
    }

    if (scoreP1 >= ROUNDS_TO_WIN && scoreP2 >= ROUNDS_TO_WIN) 
    {
        // 매치 종료
        std::cout << "[Room] Match End. Draw. Score(" << scoreP1 << " - " << scoreP2 << ")\n";
        char checkState = 1; // 1=정상 종료
        ResEndPacket resPkt(checkState);

        char sendBuf[1024] = { 0 };
        resPkt.serialize(sendBuf);
        int totalSize = resPkt.getLength();

        if (player1->GetSock() != INVALID_SOCKET) {
            SendPacket(player1->GetSock(), sendBuf, totalSize);
        }
        if (player2->GetSock() != INVALID_SOCKET) {
            SendPacket(player2->GetSock(), sendBuf, totalSize);
        }
        std::cout << "[Room] Room Close\n";
        CloseRoom(); // 소켓 종료 등
    }
    else if (scoreP1 >= ROUNDS_TO_WIN || scoreP2 >= ROUNDS_TO_WIN) {
        // 매치 종료
        std::cout << "[Room] Match End. Score(" << scoreP1 << " - " << scoreP2 << ")\n";
        char checkState = 1; // 1=정상 종료
        ResEndPacket resPkt(checkState);

        char sendBuf[1024] = { 0 };
        resPkt.serialize(sendBuf);
        int totalSize = resPkt.getLength();

        if (player1->GetSock() != INVALID_SOCKET) {
            SendPacket(player1->GetSock(), sendBuf, totalSize);
        }
        if (player2->GetSock() != INVALID_SOCKET) {
            SendPacket(player2->GetSock(), sendBuf, totalSize);
        }
        std::cout << "[Room] Room Close\n";
        CloseRoom(); // 소켓 종료 등
    }
    else {
        // 다음 라운드로 진행
        roundActive = true;
    }
}

void Room::CloseRoom()
{
    if (player1) {
        closesocket(player1->GetSock());
        player1->Reset();
    }
    if (player2) {
        closesocket(player2->GetSock());
        player2->Reset();
    }
}

void Room::SendPacket(SOCKET sock, const char* data, int size)
{
    OverlappedEx* sendOver = new OverlappedEx;
    ZeroMemory(sendOver, sizeof(OverlappedEx));
    sendOver->isSend = true;
    memcpy(sendOver->buffer, data, size);
    sendOver->wsabuf.buf = sendOver->buffer;
    sendOver->wsabuf.len = size;

    DWORD sendBytes = 0;
    int result = WSASend(sock, &sendOver->wsabuf, 1, &sendBytes, 0,
        (LPWSAOVERLAPPED)sendOver, NULL);
    if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        std::cout << "[Room] WSASend failed with error: " << WSAGetLastError() << "\n";
        delete sendOver;
    }
}