#include <iostream>
#include <WinSock2.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)
int main() {
    // Инициализация Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Создание сокета
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Подключение к серверу
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("26.105.187.156");
    serverAddress.sin_port = htons(8080);
    if (connect(clientSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "connect failed with error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    // Получение выбора от пользователя
    std::string choice;
    std::cout << "Select an option:\n1. Create game\n2. Join a game\nEnter your choice (1 or 2): ";
    std::cin >> choice;

    char buffer2[1024];
    char buffer3[1024];
    if (choice == "1") {
        int bytesSent = send(clientSocket, choice.c_str(), static_cast<int>(choice.length()), 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
        int bytesReceived = recv(clientSocket, buffer2, sizeof(buffer2), 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            return 1;
        }
        buffer2[bytesReceived] = '\0';
        std::string gameId = buffer2;
        std::cout << "Created a new game with ID: " << gameId << std::endl;
        bool start = false;
        int bytesReceived3 = recv(clientSocket, buffer3, sizeof(buffer3), 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            return 1;
        }
        buffer3[bytesReceived3] = '\0';
        std::string msg = buffer3;
        std::cout << msg << std::endl;
        if (msg == "your opponent is here") {
            start = true;
        }
        if (start) {
            std::string word_guess;
            std::cin >> word_guess;
            int bytesSent2 = send(clientSocket, word_guess.c_str(), static_cast<int>(word_guess.length()), 0);
            if (bytesSent2 == SOCKET_ERROR) {
                std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
                closesocket(clientSocket);
                WSACleanup();
                return 1;
            }
        }


    }
    else if (choice == "2") {
        // Ввод ID игры и отправка на сервер
        std::string gameId;
        std::cout << "Enter the game ID: ";
        std::cin >> gameId;
        int bytesSent = send(clientSocket, gameId.c_str(), static_cast<int>(gameId.length()), 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
        std::cout << "connected " << gameId << std::endl;
        char buffer[1024];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
        else if (bytesReceived == 0) {
            std::cout << "Server disconnected." << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return 0;
        }
        else {
            buffer[bytesReceived] = '\0';
            std::string word = buffer;
            // Логика игры "Виселица"
            std::string wordGuess(word.length(), '_');
            std::cout << "Guess the word: " << wordGuess << std::endl;

            char letterGuess;
            int incorrectGuesses = 0;
            while (wordGuess != word) {
                std::cout << "Enter a letter: ";
                std::cin >> letterGuess;

                bool found = false;
                for (size_t i = 0; i < word.length(); i++) {
                    if (word[i] == letterGuess) {
                        wordGuess[i] = letterGuess;
                        found = true;
                    }
                }

                if (!found) {
                    std::cout << "Letter is not in the word." << std::endl;
                    incorrectGuesses++;
                }

                std::cout << "Guess the word: " << wordGuess << std::endl;

                if (incorrectGuesses >= 6) {
                    std::cout << "You lose. The word was: " << word << std::endl;
                    break;
                }
            }

            if (wordGuess == word) {
                std::cout << "Congratulations! You guessed the word: " << wordGuess << std::endl;

                // Отправка результата серверу
                int bytesSent = send(clientSocket, wordGuess.c_str(), static_cast<int>(wordGuess.length()), 0);
                if (bytesSent == SOCKET_ERROR) {
                    std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
                    closesocket(clientSocket);
                    WSACleanup();
                    return 1;
                }
            }
        }
    }
    else {
        std::cerr << "Invalid choice." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    

    // Закрытие сокета и очистка Winsock
    closesocket(clientSocket);
    WSACleanup();
    system("pause");
    return 0;
   
}