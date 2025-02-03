#include <iostream>
#include <cstring>
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h> 

#define PORT 8080
#define BUFFER_SIZE 4096

void processSignal(const std::string& signal, std::string& response) {
    // mensagens que o cliente manda:
    // std::string sinal = "MSG:RELATORIO";
    // std::string sinal = "MSG:DESEMPENHO";
    // std::string sinal = "MSG:STATUS";
    // std::string sinal = "MSG:SINAL";
    // std::string sinal = "MSG:ERRO";
    // std::string sinal = "MSG:SAIR";

    // pega o sinal (depois de MSG:) e faz o que tem que fazer
    std::string sinal = signal.substr(4);
    response = "";
    if (sinal == "RELATORIO") {
        response = "RELATORIO";
    } else if (sinal == "DESEMPENHO") {
        response = "DESEMPENHO";
    } else if (sinal == "STATUS") {
        response = "STATUS";
    } else if (sinal == "SINAL") {
        response = "SINAL";
    } else if (sinal == "ERRO") {
        response = "ERRO";
    }

}

// Função para lidar com um cliente
// new_socket: socket do cliente
// Recebe um sinal do cliente, processa e envia uma resposta
void handleClient(int new_socket) {
    char buffer[BUFFER_SIZE] = {0};
    std::string response;

    while (true) {
        // Recebe dados do cliente
        int bytesRead = read(new_socket, buffer, BUFFER_SIZE);
        if (bytesRead < 0) {
            std::cerr << "Erro ao ler dados do cliente\n";
            break;
        }
        std::string signal(buffer, bytesRead);

        // Verifica se a mensagem é "SAIR"
        if (signal.substr(4) == "SAIR") {
            std::cout << "Client requested to close the connection\n";
            break;
        }

        processSignal(signal, response);

        // Envia resposta ao cliente
        send(new_socket, response.c_str(), response.size(), 0);
        std::cout << "Response sent: " << response << std::endl;

    }

    close(new_socket);
}

bool startServer(int& server_fd, struct sockaddr_in& address, int& opt) {
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return false;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        return false;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return false;
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        return false;
    }

    return true;
}

// g++ server.cpp -o server -pthread
int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if (!startServer(server_fd, address, opt)) {
        std::cout << "Erro ao iniciar o servidor\n";
        return 1;
    }
    std::cout << "Servidor iniciado\n";

    std::cout << "Server listening on port " << PORT << std::endl;
    while (true) {
        // Aceita conexões de clientes
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        std::cout << "New client connected: " << new_socket  << " | " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << std::endl;
        std::thread client_thread(handleClient, new_socket);
        client_thread.detach();
    }

    close(server_fd);
    return 0;
}

