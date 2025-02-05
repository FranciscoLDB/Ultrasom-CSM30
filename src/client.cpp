#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <unistd.h>
#include <cstring>
#include <map>
#include <fstream>
#include <random>
#include <functional>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <thread>
#include <sstream>

#define PORT 8080
#define BUFFER_SIZE 1024

// Conecta ao servidor
// Retorna true se a conexão foi bem sucedida
// Retorna false caso contrário
// sock: socket
// serv_addr: endereço do servidor
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

// Envia sinal ao servidor
// Retorna a resposta do servidor
// sock: socket
// signal: sinal a ser enviado
// buffer: buffer para armazenar a resposta
std::string enviarSinal(int sock, const std::string& signal, char* buffer) {
    send(sock, signal.c_str(), signal.size(), 0);
    std::cout << "Signal sent\n";
    memset(buffer, 0, BUFFER_SIZE); // limpa variavel buffer
    read(sock, buffer, BUFFER_SIZE);
    std::cout << "\nResponse received:" << buffer << std::endl;
    return std::string(buffer);
}

// Menu de opções
// 1. Enviar sinal
// 2. Escolher modelo
// 3. Relatório de imagens
// 4. Relatório de desempenho
// 5. Verifica status
// 6. Sair
int menu(int modelo) {
    // limpa terminal
    std::cout << "\033[2J\033[1;1H";
    std::cout << "==============MENU==============\n";
    std::cout << "=| 1. Enviar sinal            |=\n";
    std::cout << "=| 2. Escolher modelo [" << modelo << "]     |=\n"; 
    std::cout << "=| 3. Relatório de imagens    |=\n"; 
    std::cout << "=| 4. Relatório de desempenho |=\n"; 
    std::cout << "=| 5. Verifica status         |=\n"; 
    std::cout << "=| 6. Sair                    |=\n";
    std::cout << "================================\n";
    std::cout << "Escolha uma opção: ";
    int opcao;
    std::cin >> opcao;
    return opcao;
}

// Estrutura da mensagem
// modelo: 1 ou 2
// se modelo 1, n = 60
// se modelo 2, n = 30
// usuario: definido aleatoriamente
struct Mensagem {
    int modelo;
    std::string usuario;
} mensagem;

// Gera mensagem
// Preenche a estrutura mensagem com valores aleatórios
// modelo: se diferente de 0, usa o valor passado
// mensagem: mensagem a ser gerada
void geraMensagem(Mensagem& mensagem) {
    mensagem.modelo = mensagem.modelo != 0 ? mensagem.modelo : rand() % 2 + 1;
    static const std::vector<std::string> usuarios = {
        "Alice", "Bob", "Charlie", "David", "Eve", "Frank", "Grace", "Heidi", "Ivan", "Judy",
        "Kevin", "Laura", "Michael", "Nancy", "Oliver", "Pamela", "Quentin", "Rachel", "Steve", "Tina"
    };
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, usuarios.size() - 1);
    std::uniform_int_distribution<> numDis(1000, 9999); // Distribuição para o número aleatório
    mensagem.usuario = usuarios[dis(gen)] + std::to_string(numDis(gen));
}

double geraSinal(int modelo){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> disModel1(-0.000009, 0.000001);
    std::uniform_real_distribution<double> disModel2(-0.000009, 0.000001);

    if (modelo == 1) {
        return disModel1(gen);
    } else {
        return disModel2(gen);
    }
}

double geraSinalCsv(int modelo, int i, std::vector<double>& g) {
    if (g.size () == 0) {
        std::string filePath = (modelo == 1) ? "/workspaces/Ultrasom-CSM30/data/model1/G-2.csv" : "/workspaces/Ultrasom-CSM30/data/model2/g-30x30-2.csv";
        std::ifstream file(filePath);
        std::string line;
        if (file.is_open()) {
            while (std::getline(file, line)) {
                std::istringstream iss(line);
                double num;
                if (iss >> num) {
                    g.push_back(num);
                }
            }
        }
    }
    return g[i];
}

// Print barra de progresso
void printBarraProgresso(int progress) {
    int barWidth = 50;
    std::cout << "[";
    int pos = barWidth * progress / 100;
    for (int j = 0; j < barWidth; ++j) {
        if (j < pos) std::cout << "=";
        else if (j == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << progress << " %\r";
    std::cout.flush();
}

bool isValidNumber(const std::string& str) {
    std::istringstream iss(str);
    double d;
    return iss >> d && iss.eof();
}

// Enviar uma sequência de sinais em intervalos de tempo aleatórios
void enviaSinais(int sock, const Mensagem& mensagem, char* buffer) {
    int n1 = 50816, n2 = 27904, n = 0;
    n = mensagem.modelo == 1 ? n1 : n2;
    std::string header = "SINAL:" + std::to_string(mensagem.modelo) + ":" + mensagem.usuario + ":" + std::to_string(n);
    send(sock, header.c_str(), header.size(), 0);
    std::cout << "Enviando arquivo: " << header << std::endl;
    
    std::vector<double> g;
    g.clear();
    for (int i = 0; i < n; i++) {
        // double signal = geraSinal(mensagem.modelo);
        double signal = geraSinalCsv(mensagem.modelo, i, g);
        int interval = rand() % 3 + 0; // Intervalo aleatório entre 0ms e 100ms
        //std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        
        std::ostringstream oss;
        oss << signal;
        std::string sinal = oss.str();

        // verifica se sinal é um numero valido
        // deve suportar numeros negativos e decimais
        // deve suportar numeros com expoente ex: 8.90836e+17
        if (!isValidNumber(sinal)) {
            std::cerr << "Sinal inválido: " << sinal << std::endl;
            std::cout << "Aperte enter para continuar!\n";
            std::cin.ignore();
            send(sock, "ERRO", 4, 0);
            return;
        }
        // if (i >= n - 20){
        //     std::cout << "Sinal " << i << ": " << sinal.c_str() << std::endl;
        // }
        send(sock, sinal.c_str(), BUFFER_SIZE, 0);

        int progress = (i * 100 / n);
        if (i % (n / 50) == 0) {
            printBarraProgresso(progress);
        }
    }
    std::cout << "[==================================================] 100 %\n";
    std::cout << std::endl; // Nova linha após a barra de progresso
    std::cout << "Sinais enviados\n";

    send(sock, "END", BUFFER_SIZE, 0);
    std::cout << "Sinal de termino enviado\n";

    memset(buffer, 0, BUFFER_SIZE);
    read(sock, buffer, BUFFER_SIZE);
    std::cout << "Response received:" << buffer << std::endl;

    std::cout << "Aperte enter para continuar!\n";
    std::cin.ignore();
    std::cin.ignore();
}

// g++ client.cpp -o client
int main() { 
    int sock = 0; 
    struct sockaddr_in serv_addr; 
    char buffer[BUFFER_SIZE] = {0}; 
    
    if (!conectaServidor(sock, serv_addr)) {
        std::cout << "Erro ao conectar ao servidor\n";
        return 1;
    }
    std::cout << "Conectado ao servidor\n";

    int opcao;
    Mensagem mensagem;
    mensagem.modelo = 0;
    opcao = menu(mensagem.modelo);
    std::string resposta;
    while (opcao != 6) {
        switch (opcao) {
            case 1: {
                geraMensagem(mensagem);
                std::string sinal = "MSG:SINAL";
                resposta = enviarSinal(sock, sinal, buffer);
                if (resposta == "OK") {
                    enviaSinais(sock, mensagem, buffer);
                    mensagem.modelo = 0;
                    mensagem.usuario = "";
                } else {
                    std::cout << "Erro ao enviar sinal\n";
                }
                break;
            }
            case 2: {
                std::string modelo;
                std::cout << "Escolha o modelo (0 para aleatório): ";
                std::cin >> modelo;
                if (modelo != "1" && modelo != "2" && modelo != "0") {
                    std::cout << "Modelo inválido\n";
                    std::cout << "Aperte enter para continuar!\n";
                    std::cin.ignore();
                    break;
                }
                mensagem.modelo = std::stoi(modelo);
                break;
            }
            case 3: {
                std::string sinal = "MSG:RELATORIO";
                resposta = enviarSinal(sock, sinal, buffer);
                std::cout << "Pressione enter para continuar\n";
                std::cin.ignore();
                std::cin.ignore();
                break;
            }
            case 4: {
                std::string sinal = "MSG:DESEMPENHO";
                resposta = enviarSinal(sock, sinal, buffer);
                std::cout << "Pressione enter para continuar\n";
                std::cin.ignore();
                std::cin.ignore();
                break;
            }
            case 5: {
                std::string sinal = "MSG:STATUS";
                resposta = enviarSinal(sock, sinal, buffer);
                std::cout << "Pressione enter para continuar\n";
                std::cin.ignore();
                std::cin.ignore();
                break;
            }
            default:
                std::cout << "Opção inválida\n";
        }
        opcao = menu(mensagem.modelo);
    }
    
    enviarSinal(sock, "MSG:SAIR", buffer);
    close(sock);
    return 0;
}