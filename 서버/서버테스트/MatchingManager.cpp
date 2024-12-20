#include "MatchingManager.h"
#include "Room.h"
#include "ClientSession.h"

Room* MatchingManager::MatchOrWait(ClientSession* session)
{
    std::lock_guard<std::mutex> lock(matchMutex);
    // 세션에 글로벌 고유 ID 부여
    session->SetGId(globalIdCounter++);

    // 대기열에 누군가 없다면 대기시킴
    if (waitingQueue.empty()) {
        waitingQueue.push(session);
        session->SetSession(SessionState::MATCHING);
        std::cout << "[MatchingManager] Client " << session->GetGId()
            << " Add Queue\n";
        return nullptr;
    }
    else {
        // 이미 대기중인 플레이어가 있으면 매칭
        ClientSession* front = waitingQueue.front();
        waitingQueue.pop();

        Room* newRoom = new Room();
        newRoom->Init(front, session);

        front->SetSession(SessionState::IN_ROOM);
        session->SetSession(SessionState::IN_ROOM);

        std::cout << "[MatchingManager] Make new Matching Room: "
            << front->GetGId() << " vs " << session->GetGId() << "\n";
        return newRoom;
    }
}

bool MatchingManager::RemoveFromQueue(ClientSession* session)
{
    std::lock_guard<std::mutex> lock(matchMutex);

    // 임시 큐
    std::queue<ClientSession*> tempQueue;
    bool found = false;

    while (!waitingQueue.empty()) {
        ClientSession* front = waitingQueue.front();
        waitingQueue.pop();
        if (front == session) {
            found = true;
        }
        else {
            tempQueue.push(front);
        }
    }

    // tempQueue -> waitingQueue 복원
    waitingQueue = std::move(tempQueue);

    if (found) {
        std::cout << "[MatchingManager] Removed session ID="
            << session->GetGId() << " from waiting queue.\n";
    }
    return found;
}
