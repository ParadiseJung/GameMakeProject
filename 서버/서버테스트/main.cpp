#include "Server.h"

// ----------------------------------
// 메인 함수
// ----------------------------------
int main() {
    GameServer server;
    if (!server.Initialize(SERVER_PORT, MAX_WORKER_THREADS)) {
        std::cout << "Server initialization failed!\n";
        return -1;
    }

    std::cout << "[Main] Server Start\n";
    server.Run();

    std::cout << "Press Enter to stop server...\n";
    std::cin.get();

    server.Stop();
    return 0;
}