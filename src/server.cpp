#include <iostream>
#include <cstring>
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h> 
#include <map>
#include <sstream>
#include <vector>
#include <fstream>
#include <string>

#define PORT 8080
#define BUFFER_SIZE 1024

/* Estrutura da imagem
- Identificação do usuário;
- Identificação do algoritmo utilizado
- Data e hora do início da reconstrução;
- Data e hora do término da reconstrução;
- Tamanho em pixels;
- O número de iterações executadas.
*/
struct imagem {
    std::string usuario;
    int algoritmo;
    std::string dataInicio;
    std::string dataFim;
    int tamanho;
    int numIteracoes;
};

/* Verifica recursos da maquina
- Consumo de memória;
- Ocupação de CPU;
*/
struct desempenho {
    double memoria;
    double cpu;
};

void getUsage() {
    // TODO
    
}

// Função para obter o uso de memória em KB
long getMemoryUsage() {
    std::ifstream file("/proc/self/status");
    std::string line;
    while (std::getline(file, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            std::istringstream iss(line);
            std::string key;
            long value;
            std::string unit;
            iss >> key >> value >> unit;
            return value; // Retorna o valor em KB
        }
    }
    return 0;
}

// Função para obter o uso de CPU em porcentagem
double getCpuUsage() {
    std::ifstream file("/proc/stat");
    std::string line;
    std::getline(file, line);
    std::istringstream iss(line);
    std::string cpu;
    long user, nice, system, idle;
    iss >> cpu >> user >> nice >> system >> idle;
    static long prevTotal = 0;
    static long prevIdle = 0;
    long total = user + nice + system + idle;
    long totalDiff = total - prevTotal;
    long idleDiff = idle - prevIdle;
    prevTotal = total;
    prevIdle = idle;
    return (totalDiff - idleDiff) * 100.0 / totalDiff;
}

/* Função para obter o relatório
Gerar um relatório com todas as imagens reconstruídas com as seguintes informações: 
imagem gerada, usuário, número de iterações e tempo de reconstrução; */
void getRelatorio(int new_socket) {
    std::string response = "RELATORIO";
    send(new_socket, response.c_str(), response.size(), 0);
    std::cout << "Response sent: " << response << std::endl;
}

/* Função para obter o desempenho
Gerar um relatório de desempenho do servidor, com as informações de 
consumo de memória e de ocupação de CPU num determinado intervalo de tempo; */
void getDesempenho(int new_socket) {
    long memoryUsage = getMemoryUsage();
    double cpuUsage = getCpuUsage();
    std::string response = "Memória usada: " + std::to_string(memoryUsage) + " KB\n";
    response += "Uso de CPU: " + std::to_string(cpuUsage) + " %\n";
    send(new_socket, response.c_str(), response.size(), 0);
    std::cout << "Response sent: " << response << std::endl;
}

void getStatus(int new_socket) {
    std::string response = "STATUS";
    send(new_socket, response.c_str(), response.size(), 0);
    std::cout << "Response sent: " << response << std::endl;
}

void getSinal(int new_socket) {
    std::string response = "OK";
    send(new_socket, response.c_str(), response.size(), 0);
    std::cout << "Response sent: " << response << std::endl;

    char buffer[BUFFER_SIZE] = {0};
    read(new_socket, buffer, BUFFER_SIZE);
    std::string header(buffer);
    std::cout << "Header received: " << header << std::endl;

    // Parse header
    std::istringstream headerStream(header);
    std::string token;
    std::getline(headerStream, token, ':'); // MSG
    std::getline(headerStream, token, ':'); // usuario
    std::string usuario = token;
    std::getline(headerStream, token, ':'); // modelo
    int modelo = std::stoi(token);
    std::getline(headerStream, token, ':'); // numPackets
    int numPackets = std::stoi(token);
    std::cout << "Usuario: " << usuario << " Modelo: " << modelo << " NumPackets: " << numPackets << std::endl;
    std::cin.ignore();

    std::vector<double> sinal;
    for (int i = 0; i < numPackets; ++i) {
        memset(buffer, 0, BUFFER_SIZE);
        read(new_socket, buffer, BUFFER_SIZE);
        std::istringstream packetStream(buffer);
        while (std::getline(packetStream, token, ',')) {
            sinal.push_back(std::stod(token));
        }
        std::cout << "Pacote " << i << " recebido\n";
        std::cout << "Sinal size: " << sinal.size() << std::endl;
        std::cout << std::endl << "Precione enter para continuar\n";
        std::cin.ignore();
    }

    read(new_socket, buffer, BUFFER_SIZE);
    if (std::string(buffer) != "END") {
        std::cerr << "Erro ao receber pacote END\n";
    }

    std::cout << "Sinal recebido de " << usuario << " com modelo " << modelo << std::endl;
    response = "OK";
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
        std::string sinal = signal.substr(4);
        response = "";
        if (sinal == "RELATORIO") {
            getRelatorio(new_socket);
        } else if (sinal == "DESEMPENHO") {
            getDesempenho(new_socket);
        } else if (sinal == "STATUS") {
            getStatus(new_socket);
        } else if (sinal == "SINAL") {
            getSinal(new_socket);
        } else if (sinal == "SAIR"){
            std::cout << "Cliente desconectado: " << new_socket << std::endl;
            response = "Desconectado";
            send(new_socket, response.c_str(), response.size(), 0);
            std::cout << "Response sent: " << response << std::endl;
            break;
        } else {
            response = "ERRO";
            std::cerr << "Sinal inválido:" << sinal << std::endl;
        }

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

    // limpa terminal
    std::cout << "\033[2J\033[1;1H";

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

