#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h> 
#include <map>
#include "utils/desempenho.cpp"
#include "utils/control.cpp"

#define PORT 8080
#define BUFFER_SIZE 1024

// Função para obter o uso de memória em mb
// Retorna o uso de memória em mb
double getMemoryUsage() {
    std::ifstream file("/proc/self/status");
    std::string line;
    long totalMemory = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE) / 1024;
    while (std::getline(file, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            std::istringstream iss(line);
            std::string key;
            long value;
            std::string unit;
            iss >> key >> value >> unit;
            return (value / totalMemory) * 100.0; // Retorna o valor em %
        }
    }
    return 0;
}

/* Função para obter o relatório
Gerar um relatório com todas as imagens reconstruídas com as seguintes informações: 
imagem gerada, usuário, número de iterações e tempo de reconstrução; */
void getRelatorio(int new_socket) {
    send(new_socket, "OK", BUFFER_SIZE, 0);
    std::cout << "socket: " << new_socket << " | Enviando relatório...\n";

    std::ifstream file("server_files/images.csv");
    std::string line;
    std::string response = "";
    std::string aux = "";
    while (std::getline(file, line)) {
        response += line + "\n";
        if (response.size() + aux.size() >= BUFFER_SIZE) {
            send(new_socket, response.c_str(), BUFFER_SIZE, 0);
            response = response.substr(BUFFER_SIZE);
        }
    }
    if (!response.empty()) {
        send(new_socket, response.c_str(), BUFFER_SIZE, 0);
    }
    send(new_socket, "END", BUFFER_SIZE, 0);
    std::cout << "Socket: " << new_socket << " | Relatório envidado!\n";
}

/* Função para obter o desempenho
Gerar um relatório de desempenho do servidor, com as informações de 
consumo de memória e de ocupação de CPU num determinado intervalo de tempo; */
void getDesempenho(int new_socket) {
    send(new_socket, "OK", BUFFER_SIZE, 0);
    std::cout << "socket: " << new_socket << " | Enviando desempenho...\n";

    std::ifstream file("server_files/server_performance_report.csv");
    std::string line;
    std::string response = "";
    while (std::getline(file, line)) {
        response += line + "\n";
        if (response.size() >= BUFFER_SIZE) {
            send(new_socket, response.c_str(), BUFFER_SIZE, 0);
            response = response.substr(BUFFER_SIZE);
        }
    }
    if (!response.empty()) {
        send(new_socket, response.c_str(), BUFFER_SIZE, 0);
    }
    send(new_socket, "END", BUFFER_SIZE, 0);

    std::cout << "Socket: " << new_socket << " | Desempenho enviado!\n";
}

bool isValidNumber(const std::string& str) {
    std::istringstream iss(str);
    double d;
    return iss >> d && iss.eof();
}

extern SignalProcessController controller;
void getSinal(int new_socket, struct sockaddr_in& address) {
    std::string response = "OK";
    send(new_socket, response.c_str(), response.size(), 0);
    //std::cout << "Response sent: " << response << std::endl;
    std::vector<double> sinal;

    char buffer[BUFFER_SIZE] = {0};
    memset(buffer, 0, BUFFER_SIZE);
    read(new_socket, buffer, BUFFER_SIZE);
    int quant_seq = std::stoi(std::string(buffer).substr(6));
    //std::cout << "quant:" << quant_seq << std::endl;

    for (int j = 0; j < quant_seq; j++) {
        memset(buffer, 0, BUFFER_SIZE);
        read(new_socket, buffer, BUFFER_SIZE);
        std::string header(buffer);
        //std::cout << "Header: " << header << std::endl;
        // Parse header
        std::string token;
        std::istringstream iss(header);
        std::getline(iss, token, ':'); // SINAL
        std::getline(iss, token, ':'); // modelo
        int modelo = std::stoi(token);
        std::getline(iss, token, ':'); // usuario
        std::string filePath = token;
        std::getline(iss, token, ':'); // n
        int n = std::stoi(token);

        // vetor para armazenar o indice dos sinais que deram erro
        std::vector<int> erros;
        erros.clear();
        sinal.clear();
        std::cout << "Modelo: " << modelo << " | Usuario: " << filePath << " | n: " << n << std::endl;

        bool end = false;
        int msgCont = 0;
        while (true) {
            msgCont++;
            memset(buffer, 0, BUFFER_SIZE); // limpa variavel buffer
            read(new_socket, buffer, BUFFER_SIZE);
            if (std::string(buffer).find("END") != std::string::npos) {
                //std::cout << "===================Sinal de END recebido===================\n";
                break;
            }
            std::string str(buffer);
            std::istringstream iss(str);
            std::string token;
            while (std::getline(iss, token, ';')) {
                if (isValidNumber(token)) {
                    sinal.push_back(std::stod(token));
                } else {
                    std::cerr << "Erro: valor inválido recebido: " << token << std::endl;
                    erros.push_back(sinal.size());
                }
            }
        }

        //std::cout << "Socket: " << new_socket << " | IP: " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << " | 100% concluído" << std::endl;
        std::cout << "Sinal recebido com " << sinal.size() << " elementos | Erros: " << erros.size() << std::endl;

        controller.addProcess(SignalProcess(sinal, modelo, filePath));
    }

    memset(buffer, 0, BUFFER_SIZE);
    read(new_socket, buffer, BUFFER_SIZE);
    std::cout << "Response received:" << buffer << std::endl;
    if (std::string(buffer) != "FINISH") {
        std::cerr << "Erro ao receber sinal de FINISH!\n";
        send(new_socket, "ERRO", 4, 0);
        return;
    }    

    send(new_socket, "OK", 2, 0);
}

// Função para lidar com um cliente
// new_socket: socket do cliente
// Recebe um sinal do cliente, processa e envia uma resposta
void handleClient(int new_socket, struct sockaddr_in& address) {
    char buffer[BUFFER_SIZE] = {0};
    std::string response;
    while (true) {
        // Recebe dados do cliente
        memset(buffer, 0, BUFFER_SIZE); // limpa variavel buffer
        int bytesRead = read(new_socket, buffer, BUFFER_SIZE);
        if (bytesRead <= 0) {
            if (bytesRead == 0) {
                std::cout << "Cliente desconectado: " << new_socket << std::endl;
            } else {
                std::cerr << "Erro ao ler dados do cliente\n";
            }
            break;
        }

        std::string signal(buffer, bytesRead);
        if (signal.size() < 4) {
            std::cerr << "Erro: sinal recebido é muito curto\n";
            continue;
        }
        std::string codigo = signal.substr(4);
        response = "";
        if (codigo == "RELATORIO") {
            getRelatorio(new_socket);
        } else if (codigo == "DESEMPENHO") {
            getDesempenho(new_socket);
        } else if (codigo == "SINAL") {
            getSinal(new_socket, address);
        } else if (codigo == "SAIR"){
            std::cout << "Cliente desconectado: " << new_socket << std::endl;
            response = "Desconectado";
            send(new_socket, response.c_str(), response.size(), 0);
            std::cout << "Response sent: " << response << std::endl;
            break;
        } else {
            response = "ERRO";
            std::cerr << "Codigo inválido:" << codigo << std::endl;
            close(new_socket);
            return;
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

// g++ server.cpp -o server -pthread -lblas

SignalProcessController controller;
int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    std::cout << "\033[2J\033[1;1H"; // limpa terminal
    std::cout << "Iniciando servidor...\n";

    // Inicia a thread para monitorar o desempenho do servidor
    std::thread performance_thread(logPerformance, "server_files/server_performance_report.csv", 5, 2200);
    performance_thread.detach();

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
        std::thread client_thread(handleClient, new_socket, std::ref(address));
        client_thread.detach();
    }

    close(server_fd);
    return 0;
}

