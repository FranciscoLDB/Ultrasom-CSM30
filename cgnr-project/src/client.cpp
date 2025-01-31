#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <unistd.h>
#include <cstring>

#define PORT 8080

bool conectaServidor(int& sock, struct sockaddr_in& serv_addr) {
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "\nErro ao criar o socket\n";
        return false;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converte endereço IP para binário
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cout << "\nInvalid address/ Address not supported \n";
        return false;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        return false;
    }
    return true;
}

void enviarSinal(int sock, const std::string& signal) {
    send(sock, signal.c_str(), signal.size(), 0);
    std::cout << "Signal sent\n";
}

int main() { 
    int sock = 0; 
    struct sockaddr_in serv_addr; 
    char buffer[1024] = {0}; 
    std::string signal = "SINAL, model, user";
    if (!conectaServidor(sock, serv_addr)) {
        std::cout << "Erro ao conectar ao servidor\n";
        return 1;
    }
    std::cout << "Conectado ao servidor\n";

    // Envia dados ao servidor
    send(sock, signal.c_str(), signal.size(), 0);
    std::cout << "Signal sent\n";

    // servidor
    read(sock, buffer, 1024);
    std::cout << "Response from server: " << buffer << std::endl;

    close(sock);
    return 0;
}