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
    std::cout << "Response received:" << buffer << std::endl;
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
// sinal: vetor de n elementos double
// modelo: 1 ou 2
// se modelo 1, n = 60
// se modelo 2, n = 30
// usuario: definido aleatoriamente
struct Mensagem {
    std::vector<double> sinal;
    int modelo;
    std::string usuario;
} mensagem;

// Cria sinal de ultrassom com operações adicionais
void criaSinalUltrassom(std::vector<double>& sinal, int modelo) {
    int n1 = 50816, n2 = 27904;
    sinal.clear();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(1.2e-20, 7.5e+20);
    std::uniform_int_distribution<> opDis(0, 3); // Distribuição para escolher a operação
    std::uniform_real_distribution<> divDis(1.0, 10000.0); // Distribuição para o divisor

    auto applyOperation = [&](double num) {
        int op = opDis(gen);
        std::string str = "";
        double mantissa = 0.0;
        switch (op) {
            case 0:
                str = std::to_string(num);
                std::reverse(str.begin(), str.end());
                return std::stod(str);
            case 1:
                return -num;
            case 2:
                return num / divDis(gen);
            case 3:
                int exp;
                mantissa = std::frexp(num, &exp);
                return std::ldexp(mantissa, -exp);
            default:
                return num;
        }
    };

    if (modelo == 1) {
        for (int i = 0; i < n1; i++) {
            double num = dis(gen);
            sinal.push_back(applyOperation(num));
        }
    } else {
        for (int i = 0; i < n2; i++) {
            double num = dis(gen);
            sinal.push_back(applyOperation(num));
        }
    }
}

// Gera mensagem
// Preenche a estrutura mensagem com valores aleatórios
// modelo: se diferente de 0, usa o valor passado
// mensagem: mensagem a ser gerada
// signal: sinal a ser gerado
void geraMensagem(Mensagem& mensagem, const std::string& signal) {
    mensagem.modelo = mensagem.modelo != 0 ? mensagem.modelo : rand() % 2 + 1;
    criaSinalUltrassom(mensagem.sinal, mensagem.modelo);
    static const std::vector<std::string> usuarios = {
        "Alice", "Bob", "Charlie", "David", "Eve", "Frank", "Grace", "Heidi", "Ivan", "Judy"
    };
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, usuarios.size() - 1);
    std::uniform_int_distribution<> numDis(1000, 9999); // Distribuição para o número aleatório
    mensagem.usuario = usuarios[dis(gen)] + std::to_string(numDis(gen));
}

void enviaArquivo(int sock, const std::string& filePath, char* buffer) {
    std::ifstream file("client_files/" + filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Erro ao abrir o arquivo\n";
        std::cout << "Aperte enter para continuar!\n";
        std::cin.ignore();
        std::cin.ignore();
        return;
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string header = "FILE:" + filePath + ":" + std::to_string(fileSize);
    send(sock, header.c_str(), header.size(), 0);
    std::cout << "Enviando arquivo: " << header << std::endl;

    char fileBuffer[BUFFER_SIZE];
    while (file.read(fileBuffer, BUFFER_SIZE)) {
        send(sock, fileBuffer, BUFFER_SIZE, 0);
    }
    send(sock, fileBuffer, file.gcount(), 0);
    file.close();

    std::cout << "Arquivo enviado\n";

    memset(buffer, 0, BUFFER_SIZE);
    read(sock, buffer, BUFFER_SIZE);
    std::cout << "Response received:" << buffer << std::endl;
}

void salvaSinalEmCSV(const Mensagem& mensagem, const std::string& filePath) {
    std::ofstream file("client_files/" + filePath, std::ofstream::trunc);
    if (!file.is_open()) {
        std::cout << "Erro ao criar o arquivo\n";
        return;
    }

    for (const auto& valor : mensagem.sinal) {
        file << valor << "\n";
    }

    file.close();
}

void enviaMensagem(int sock, const Mensagem& mensagem, char* buffer) {
    std::string filePath = mensagem.usuario + ".csv";
    salvaSinalEmCSV(mensagem, filePath);
    enviaArquivo(sock, filePath, buffer);
    // Exclui o arquivo após enviar
    if (remove(("client_files/" + filePath).c_str()) != 0) {
        std::cout << "Erro ao excluir o arquivo\n";
    }
}

// g++ client.cpp -o client
int main() { 
    int sock = 0; 
    struct sockaddr_in serv_addr; 
    char buffer[BUFFER_SIZE] = {0}; 
    std::string signal = "SINAL, model, user";
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
                geraMensagem(mensagem, signal);
                std::string sinal = "MSG:SINAL";
                resposta = enviarSinal(sock, sinal, buffer);
                if (resposta == "OK") {
                    enviaMensagem(sock, mensagem, buffer);
                    mensagem.sinal.clear();
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
                break;
            }
            case 4: {
                std::string sinal = "MSG:DESEMPENHO";
                resposta = enviarSinal(sock, sinal, buffer);
                break;
            }
            case 5: {
                std::string sinal = "MSG:STATUS";
                resposta = enviarSinal(sock, sinal, buffer);
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