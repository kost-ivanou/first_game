#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <WS2tcpip.h>
#include <map>

#pragma comment(lib, "ws2_32.lib")
std::map<std::string, SOCKET> IdSockMap;
std::string generateGameId() {
    srand(time(0));
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string id(8, 0);
    for (int i = 0; i < 8; ++i) {
        id[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return id;
}
void playHangmanGame(SOCKET clientSocket1, SOCKET clientSocket2) {
    std::string msg = "your opponent is here";
    int bytesSent = send(clientSocket1, msg.c_str(), static_cast<int>(msg.length()), 0);
    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket1);
        closesocket(clientSocket2);
        return;
    }
    char buffer[1024];
    //word from 1st client
    int bytesReceived = recv(clientSocket1, buffer, sizeof(buffer), 0);
    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket1);
        closesocket(clientSocket2);
        return;
    }
    buffer[bytesReceived] = '\0';
    std::string word = buffer;
    // Отправляем слово второму клиенту
    int bytesSent1 = send(clientSocket2, word.c_str(), static_cast<int>(word.length()), 0);
    if (bytesSent1 == SOCKET_ERROR) {
        std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket1);
        closesocket(clientSocket2);
        return;
    }

    // Получаем ответ от второго клиента
    
    char buf[1024];
    int bytesReceived1 = recv(clientSocket2, buf, sizeof(buf), 0);
    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket1);
        closesocket(clientSocket2);
        return;
    }
    else if (bytesReceived1 == 0) {
        std::cout << "Client disconnected." << std::endl;
        closesocket(clientSocket1);
        closesocket(clientSocket2);
        return;
    }
    else {
        buf[bytesReceived] = '\0';
        std::string result = buffer;

        if (result == word) {
            std::cout << "Client guessed the word correctly!" << std::endl;
        }
        else {
            std::cout << "Client failed to guess the word." << std::endl;
        }
    }

    // Закрытие соединения с клиентами
    closesocket(clientSocket1);
    closesocket(clientSocket2);
}
void handleClient(SOCKET clientSocket) {
    std::string gameId = generateGameId();
    IdSockMap.insert(std::make_pair(gameId, clientSocket));
    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        return;
    }
    else if (bytesReceived == 0) {
        std::cout << "Client disconnected." << std::endl;
        closesocket(clientSocket);
        return;
    }
    else {
        buffer[bytesReceived] = '\0';
        std::string result = buffer;
        if (result == "1") {
            int bytesSent = send(clientSocket, gameId.c_str(), static_cast<int>(gameId.length()), 0);
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
                closesocket(clientSocket);
                return;
            }
        }
        else {
            bool flag = false;
            for (const auto& pair : IdSockMap) {
                if (result == pair.first) {
                    playHangmanGame(pair.second, clientSocket);
                    flag = true;
                }
            }
            if (!flag) {
                std::string error = "incorrect id!";
                int bytesSent5 = send(clientSocket, error.c_str(), static_cast<int>(error.length()), 0);
                if (bytesSent5 == SOCKET_ERROR) {
                    std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
                    closesocket(clientSocket);
                    return;
                }
            }
        }
        
    }

    // Закрытие соединения с клиентом
}

int main() {
    // Инициализация Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Создание сокета
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Связывание сокета с адресом
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);
    if (bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Прослушивание входящих соединений
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening on port 8080..." << std::endl;

    std::vector<std::thread> threads;

    // Ожидание входящих соединений и обработка запросов в отдельных потоках
    while (true) {
        SOCKET clientSocket;
        sockaddr_in clientAddress;
        int clientAddressLength = sizeof(clientAddress);
        clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &clientAddressLength);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "accept failed with error: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "Client connected." << std::endl;

        // Запуск обработки запроса клиента в отдельном потоке
        threads.emplace_back(std::thread(handleClient, clientSocket));
    }

    // Закрытие всех потоков
    for (auto& thread : threads) {
        thread.join();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}