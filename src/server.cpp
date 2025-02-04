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
#include "utils/readData.cpp"
#include "cgnr.cpp"

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

// Função para obter o uso de memória em mb
// Retorna o uso de memória em mb
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
            return value/1000; // Retorna o valor em mb
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
    std::string response = "Memória usada: " + std::to_string(memoryUsage) + " KB / " + " 8 GB\n";
    response += "Uso de CPU: " + std::to_string(cpuUsage) + " %\n";
    send(new_socket, response.c_str(), response.size(), 0);
    std::cout << "Response sent: " << response << std::endl;
}

void getStatus(int new_socket) {
    std::string response = "STATUS";
    send(new_socket, response.c_str(), response.size(), 0);
    std::cout << "Response sent: " << response << std::endl;
}

void excluiArquivo(const std::string& filePath) {
    if (remove(("server_files/" + filePath).c_str()) != 0) {
        std::cout << "Erro ao excluir o arquivo\n";
    }
}

bool isValidNumber(const std::string& str) {
    std::istringstream iss(str);
    double d;
    return iss >> d && iss.eof();
}

void salvarSinal(const std::vector<double>& sinal, const std::string& filePath) {
    std::ofstream file("server_files/" + filePath, std::ofstream::trunc);
    if (!file.is_open()) {
        std::cerr << "Erro ao criar o arquivo\n";
        return;
    }

    for (const auto& valor : sinal) {
        file << valor << "\n";
    }
    file.close();
    std::cout << "Arquivo salvo\n";
}

void getSinal(int new_socket, std::vector<double>& sinal, struct sockaddr_in& address) {
    std::string response = "OK";
    send(new_socket, response.c_str(), response.size(), 0);
    std::cout << "Response sent: " << response << std::endl;

    char buffer[BUFFER_SIZE] = {0};
    read(new_socket, buffer, BUFFER_SIZE);
    std::string header(buffer);

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
    for (int i = 0; i < n; i++) {
        double num;
        do {
            memset(buffer, 0, BUFFER_SIZE); // limpa variavel buffer
            read(new_socket, buffer, BUFFER_SIZE);
        } while (std::string(buffer).empty());
        std::string str(buffer);
        if (isValidNumber(str)) {
            num = std::stod(str);
            sinal.push_back(num);
        } else {
            //std::cerr << "Erro: valor inválido recebido: " << str << " | i: " << i << std::endl;
            erros.push_back(i);
            //sinal.push_back(0.0);
        }

        // Barra de progresso
        int progress = (i * 100 / n);
        if (i % (n / 100) == 0) {
            if (progress % 25 == 0) {
                std::cout << "Socket: " << new_socket << " | IP: " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << " | " << progress << "% concluído" << std::endl;
            }
        }
    }
    std::cout << "Socket: " << new_socket << " | IP: " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << " | 100% concluído" << std::endl;
    std::cout << "Sinal recebido com " << sinal.size() << " elementos | Erros: " << erros.size() << std::endl;

    read(new_socket, buffer, BUFFER_SIZE);
    //std::cout << "Response received:" << buffer << std::endl;
    if (std::string(buffer) != "END") {
        std::cerr << "Erro ao receber sinal de termino!\n";
        send(new_socket, "ERRO", 4, 0);
        return;
    }
    send(new_socket, "OK", 2, 0);

    //salvarSinal(sinal, filePath + ".csv");
}

// Função para lidar com um cliente
// new_socket: socket do cliente
// Recebe um sinal do cliente, processa e envia uma resposta
void handleClient(int new_socket, struct sockaddr_in& address) {
    char buffer[BUFFER_SIZE] = {0};
    std::string response;
    std::vector<double> sinal_vet;
    while (true) {
        // Recebe dados do cliente
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
        } else if (codigo == "STATUS") {
            getStatus(new_socket);
        } else if (codigo == "SINAL") {
            getSinal(new_socket, sinal_vet, address);
        } else if (codigo == "SAIR"){
            std::cout << "Cliente desconectado: " << new_socket << std::endl;
            response = "Desconectado";
            send(new_socket, response.c_str(), response.size(), 0);
            std::cout << "Response sent: " << response << std::endl;
            break;
        } else {
            response = "ERRO";
            std::cerr << "Codigo inválido:" << codigo << std::endl;
        }
    }

    close(new_socket);
    while (true) {
        // Recebe dados do cliente
        int bytesRead = read(new_socket, buffer, BUFFER_SIZE);
        if (bytesRead < 0) {
            std::cerr << "Erro ao ler dados do cliente\n";
            break;
        }

        std::string signal(buffer, bytesRead);
        std::string codigo = signal.substr(4);
        response = "";
        if (codigo == "RELATORIO") {
            getRelatorio(new_socket);
        } else if (codigo == "DESEMPENHO") {
            getDesempenho(new_socket);
        } else if (codigo == "STATUS") {
            getStatus(new_socket);
        } else if (codigo == "SINAL") {
            getSinal(new_socket, sinal_vet, address);
        } else if (codigo == "SAIR"){
            std::cout << "Cliente desconectado: " << new_socket << std::endl;
            response = "Desconectado";
            send(new_socket, response.c_str(), response.size(), 0);
            std::cout << "Response sent: " << response << std::endl;
            break;
        } else {
            response = "ERRO";
            std::cerr << "Codigo inválido:" << codigo << std::endl;
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
    std::vector<std::vector<double>> H1;
    std::vector<std::vector<double>> H2;

    std::cout << "\033[2J\033[1;1H"; // limpa terminal

    std::cout << "Iniciando servidor...\n";
    readData(H1, "../data/model1/H-1.csv");
    std::cout << "Matriz H1: " << H1.size() << " x " << H1[0].size() << std::endl;
    readData(H2, "../data/model2/H-2.csv");
    std::cout << "Matriz H2: " << H2.size() << " x " << H2[0].size() << std::endl;

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

