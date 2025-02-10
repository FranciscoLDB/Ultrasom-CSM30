#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <fstream>

// Estrutura para armazenar as informações da imagem
struct ImagemInfo {
    std::string usuario;
    std::string algoritmo;
    std::string dataInicio;
    std::string dataFim;
    std::string tamanho;
    std::string numIteracoes;
    std::string path;
    std::string tempo;
};

// Função para dividir uma string em substrings com base em um delimitador
std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Função para gerar o relatório
void geraRelatorio(const std::string& data) {
    std::cout << "Gerando relatório...\n";
    
    std::vector<ImagemInfo> imagens;
    std::istringstream dataStream(data);
    std::string line;

    // Ler cada linha da string de dados
    while (std::getline(dataStream, line)) {
        std::vector<std::string> tokens = split(line, ',');
        if (tokens.size() == 8) {
            ImagemInfo info;
            info.usuario = tokens[0];
            info.algoritmo = tokens[1];
            info.dataInicio = tokens[2];
            info.dataFim = tokens[3];
            info.tamanho = tokens[4];
            info.numIteracoes = tokens[5];
            info.path = tokens[6];
            info.tempo = tokens[7];
            imagens.push_back(info);
        }
    }

    // Gerar o relatório
    std::ofstream relatorio("relatorio_imagens.txt");
    if (!relatorio.is_open()) {
        std::cerr << "Erro ao criar o arquivo de relatório\n";
        return;
    }

    relatorio << "Relatório de Imagens Reconstruídas\n";
    relatorio << "===========================================================\n";
    imagens.erase(imagens.begin());
    std::cout << "Imagens reconstruídas: " << imagens.size() << std::endl;
    for (const auto& img : imagens) {
        relatorio << "Usuário: " << img.usuario << "\n";
        relatorio << "Algoritmo: " << img.algoritmo << "\n";
        relatorio << "Data Início: " << img.dataInicio << "\n";
        relatorio << "Data Fim: " << img.dataFim << "\n";
        relatorio << "Tamanho: " << img.tamanho << " pixels\n";
        relatorio << "Número de Iterações: " << img.numIteracoes << "\n";
        relatorio << "Caminho da Imagem: " << img.path << "\n";
        relatorio << "Tempo de Reconstrução: " << img.tempo << " segundos\n";
        relatorio << "-----------------------------------------------------------\n";
    }

    relatorio.close();
    std::cout << "Relatório gerado com sucesso: relatorio_imagens.txt\n";
}

void geraDesempenho(const std::string& data) {
    std::cout << "Gerando relatório de desempenho...\n";
    
    std::ofstream relatorio("relatorio_desempenho.txt");
    if (!relatorio.is_open()) {
        std::cerr << "Erro ao criar o arquivo de relatório de desempenho\n";
        return;
    }

    relatorio << "Relatório de Desempenho do Servidor\n";
    relatorio << "===========================================================\n";
    relatorio << data;

    relatorio.close();
    std::cout << "Relatório de desempenho gerado com sucesso: relatorio_desempenho.txt\n";
}
