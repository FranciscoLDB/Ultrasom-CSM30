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
#include "utils/relatorio.cpp"

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
    //std::cout << "\nResponse received:" << buffer << std::endl;
    return std::string(buffer);
}

// Menu de opções
// 1. Enviar sinal
// 2. Escolher modelo
// 3. Relatório de imagens
// 4. Relatório de desempenho
// 5. Verifica status
// 6. Sair
int menu() {
    // limpa terminal
    std::cout << "\033[2J\033[1;1H";
    std::cout << "==============MENU==============\n";
    std::cout << "=| 1. Enviar sinal            |=\n";
    std::cout << "=| 2. Relatório de imagens    |=\n"; 
    std::cout << "=| 3. Relatório de desempenho |=\n";
    std::cout << "=| 4. Sair                    |=\n";
    std::cout << "================================\n";
    std::cout << "Escolha uma opção: ";
    int opcao;
    std::cin >> opcao;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore();
        opcao = 0;
    }    
    return opcao;
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

void enviaSequencia(int sock, char* buffer) {
    std::srand(std::time(nullptr)); // Seta uma seed aleatória
    int modelo = rand() % 2 + 1;
    static const std::vector<std::string> usuarios = {
        "Alice", "Bob", "Charlie", "David", "Eve", "Frank", "Grace", "Heidi", "Ivan", "Judy",
        "Kevin", "Laura", "Michael", "Nancy", "Oliver", "Pamela", "Quentin", "Rachel", "Steve", "Tina"
    };
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, usuarios.size() - 1);
    std::uniform_int_distribution<> numDis(1000, 9999); // Distribuição para o número aleatório
    std::string usuario = usuarios[dis(gen)] + std::to_string(numDis(gen));

    std::vector<std::string> paths1 = {
        "/workspaces/Ultrasom-CSM30/data/model1/G-1.csv",
        "/workspaces/Ultrasom-CSM30/data/model1/G-2.csv",
    };
    std::vector<std::string> paths2 = {
        "/workspaces/Ultrasom-CSM30/data/model2/g-30x30-1.csv",
        "/workspaces/Ultrasom-CSM30/data/model2/g-30x30-2.csv",
    };
    int randomIndex = rand() % 2;
    std::string filePath = modelo == 1 ? paths1[randomIndex] : paths2[randomIndex];

    int n1 = 50816, n2 = 27904, n = 0;
    n = modelo == 1 ? n1 : n2;
    std::string header = "SINAL:" + std::to_string(modelo) + ":" + usuario + ":" + std::to_string(n);
    send(sock, header.c_str(), BUFFER_SIZE, 0);
    std::cout << "Enviando arquivo: " << header << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::vector<double> g;
    g.clear();
    if (g.size () == 0) {    
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
    
    std::string msg = "";
    std::string aux = "";
    std::ostringstream oss;
    int msgCont = 0;
    for (int i = 0; i < n; i++) {
        // Envia o maximo de sinais por vez
        //std::cout << "Enviando sinal " << i + 1 << " de " << n << std::endl;
        //std::cout << "Sinal: " << g[i] << std::endl;
        oss.str(""); // Limpa o conteúdo do ostringstream
        oss << g[i];
        //std::cout << "sinal string: " << oss.str() + ";" << std::endl;
        aux = oss.str() + ";";
        if (msg.size() + aux.size() >= BUFFER_SIZE) {
            //std::cout << "Enviando: " << msg << std::endl;
            send(sock, msg.c_str(), BUFFER_SIZE, 0);
            msgCont++;
            msg = "";

            // if (msgCont == 10){
            //     break;
            // }
        }
        msg += aux;

        int progress = (i * 100 / n);
        if (i % (n / 50) == 0) {
            printBarraProgresso(progress);
        }
    }
    if (!msg.empty()) {
        send(sock, msg.c_str(), BUFFER_SIZE, 0);
        std::cout << "last msg: " << msg << std::endl;
    }
    std::cout << "[==================================================] 100 %\n";
    std::cout << std::endl; // Nova linha após a barra de progresso
    std::cout << "Sinais enviados\n";

    send(sock, "END", BUFFER_SIZE, 0);
    std::cout << "Sinal de END enviado\n";
}

// Enviar uma sequência de sinais em intervalos de tempo aleatórios
void enviaSinais(int sock, char* buffer, int quant_seq = 1) {
    std::cout << "Enviando " << quant_seq << " sequências de sinais\n";
    std::string sinal = "START:"+std::to_string(quant_seq);
    send(sock, sinal.c_str(), BUFFER_SIZE, 0);

    for (int i = 0; i < quant_seq; i++) {
        std::cout << "Enviando sequência " << i + 1 << " de " << quant_seq << std::endl;
        enviaSequencia(sock, buffer);
        // add um intervalo aleatorio entre 1 e 5 segundos
        int interval = rand() % 5 + 1; // Intervalo aleatório entre 1s e 5s  
        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }
    send(sock, "FINISH", BUFFER_SIZE, 0);
    std::cout << "Sinal de FINISH enviado\n";
    memset(buffer, 0, BUFFER_SIZE);
    read(sock, buffer, BUFFER_SIZE);
    std::cout << "Response received:" << buffer << std::endl;

    std::cout << "Aperte enter para continuar!\n";
    std::cin.ignore();
    std::cin.ignore();
}



// g++ client.cpp -o client
int main() { 
    std::cout << "\033[2J\033[1;1H";
    
    int sock = 0; 
    struct sockaddr_in serv_addr; 
    char buffer[BUFFER_SIZE] = {0}; 
    
    if (!conectaServidor(sock, serv_addr)) {
        std::cout << "Erro ao conectar ao servidor\n";
        return 1;
    }
    std::cout << "Conectado ao servidor\n";

    int opcao;
    opcao = menu();
    std::string resposta;
    while (opcao != 4) {
        switch (opcao) {
            case 1: {
                std::cin.ignore();
                int quant_seq;
                std::cout << "Quantas sequências deseja enviar? ";
                std::cin >> quant_seq;
                std::string sinal = "MSG:SINAL";
                resposta = enviarSinal(sock, sinal, buffer);
                if (resposta == "OK") {
                    enviaSinais(sock, buffer, quant_seq);
                } else {
                    std::cout << "Erro ao enviar sinal\n";
                }
                break;
            }
            case 2: {
                std::string sinal = "MSG:RELATORIO";
                resposta = enviarSinal(sock, sinal, buffer);
                if (resposta != "ERRO") {
                    geraRelatorio(resposta);
                } else {
                    std::cout << "Erro ao obter relatório\n";
                }
                std::cout << "Pressione enter para continuar\n";
                std::cin.ignore();
                std::cin.ignore();
                break;
            }
            case 3: {
                std::string sinal = "MSG:DESEMPENHO";
                resposta = enviarSinal(sock, sinal, buffer);
                if (resposta != "ERRO") {
                    std::cout << "Copiando relatório de desempenho\n";
                    std::ofstream desempenhoFile("desempenho.txt");
                    if (desempenhoFile.is_open()) {
                        while (true) {
                            std::cout << "Recebendo dados...\n";
                            memset(buffer, 0, BUFFER_SIZE);
                            read(sock, buffer, BUFFER_SIZE);
                            //std::cout << buffer;
                            if (std::string(buffer).find("END") != std::string::npos) {
                                break;
                            }                            
                            desempenhoFile << buffer;
                        }
                        desempenhoFile.close();
                    } else {
                        std::cout << "Erro ao abrir o arquivo de desempenho\n";
                    }
                    //geraDesempenho(resposta);
                } else {
                    std::cout << "Erro ao obter relatório de desempenho\n";
                }
                std::cout << "Pressione enter para continuar\n";
                std::cin.ignore();
                std::cin.ignore();
                break;
            }
            default:
                std::cout << "Opção inválida\n";
        }
        opcao = menu();
    }
    
    enviarSinal(sock, "MSG:SAIR", buffer);
    close(sock);
    return 0;
}