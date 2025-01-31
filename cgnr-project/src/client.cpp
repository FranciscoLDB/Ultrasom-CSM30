#include <iostream>
#include <vector>
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

void enviarSinal(int sock, const std::string& signal, char* buffer) {
    send(sock, signal.c_str(), signal.size(), 0);
    std::cout << "Signal sent\n";
    read(sock, buffer, 1024);
}

int menu() {
    std::cout << "1. Enviar sinal\n";
    std::cout << "2. Escolher modelo\n"; 
    std::cout << "3. Sair\n";
    std::cout << "Escolha uma opção: ";
    int opcao;
    std::cin >> opcao;
    return opcao;
}

// Estrutura da mensagem
// sinal: vetor de n elementos double
// modelo: 1 ou 2
// se modelo 1, n = 60
// se modelo 2, n = 30
// usuario: definido aleatoriamente
struct Mensagem {
    std::vector<double> sinal;
    int modelo;
    std::string usuario;
};

// g++ client.cpp -o client
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
    int opcao;
    opcao = menu();
    while (opcao != 3) {
        if (opcao == 1) {
            std::cout << "Digite o sinal: ";
            std::cin >> signal;
            enviarSinal(sock, signal, buffer);
        } else if (opcao == 2) {
            std::cout << "Escolha o modelo: ";
            std::cin >> signal;
            enviarSinal(sock, signal, buffer);
        }
        opcao = menu();
    }

    enviarSinal(sock, signal, buffer);

    close(sock);
    return 0;
}