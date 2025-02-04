#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <unistd.h>
#include <cstring>
#include <map>

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

// Cria sinal de ultrassom
// sinal: vetor de n elementos double
// modelo: 1 ou 2
// se modelo 1, n = 50816, range de 0 a 793
// se modelo 2, n = 27904, range de 0 a 435
void criaSinalUltrassom(std::vector<double>& sinal, int modelo) {
    if (modelo == 1) {
        for (int i = 0; i < 50816; i++) {
            sinal.push_back(rand() % 794);
        }
    } else {
        for (int i = 0; i < 27904; i++) {
            sinal.push_back(rand() % 436);
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
    mensagem.usuario = "user" + std::to_string(rand() % 1000);
}

void enviaPacote(int sock, const std::string& pacote) {
    send(sock, pacote.c_str(), pacote.size(), 0);
}

void enviaMensagem(int sock, const Mensagem& mensagem, char* buffer) {
    size_t totalSize = mensagem.sinal.size();
    size_t numPackets = (totalSize * sizeof(double) + BUFFER_SIZE - 1) / BUFFER_SIZE;
    std::string header = "MSG:" + mensagem.usuario + ":" + std::to_string(mensagem.modelo) + ":" + std::to_string(numPackets);
    enviaPacote(sock, header);
    std::cout << "Enviando mensagem: " << header << std::endl;
    std::cout << "Enviando " << numPackets << " pacotes\n";

    std::cout << "Precione enter para começar a enviar os pacotes\n";
    std::cin.ignore();
    std::cin.ignore();

    for (size_t i = 0; i < numPackets; ++i) {
        size_t start = i * BUFFER_SIZE / sizeof(double);
        size_t end = std::min(start + BUFFER_SIZE / sizeof(double), totalSize);
        std::string pacote;

        // print a cada 20% do total concluido
        if (i % (numPackets / 5) == 0) {
            std::cout << "Enviando pacote " << i << " de " << numPackets << std::endl;
        }

        for (size_t j = start; j < end; ++j) {
            pacote += std::to_string(mensagem.sinal[j]) + ",";
        }
        pacote.pop_back(); // Remove a última vírgula
        enviaPacote(sock, pacote);
    }

    enviaPacote(sock, "END");
    std::cout << "Mensagem enviada\n";

    memset(buffer, 0, BUFFER_SIZE);
    read(sock, buffer, BUFFER_SIZE);
    std::cout << "Response received:" << buffer << std::endl;
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