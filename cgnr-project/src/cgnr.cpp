#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <algorithm>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "utils/stb_image_write.h"
#include "utils/readData.cpp"

using namespace std;

// Função para calcular a norma L2 de um vetor
double l2Norm(const vector<double>& vec) {
    double sum = 0.0;
    for (double val : vec) {
        sum += val * val;
    }
    return sqrt(sum);
}

// Função principal do algoritmo CGNR
void cgnr(const vector<vector<double>>& H, const vector<double>& g, vector<double>& f) {
    vector<double> r = g; // r_0 = g
    vector<double> z(H[0].size(), 0.0); // z_0 = H^T * r_0
    vector<double> p(H[0].size(), 0.0); // p_0 = z_0
    double epsilon = 1e-4; // Limite de erro
    int maxIterations = 1000; // Limite de iterações

    // Inicialização
    for (size_t i = 0; i < H[0].size(); ++i) {
        z[i] = 0; // z_0 = H^T * r_0
    }
    cout << "Inicialização concluída." << endl;

    // Iteração do algoritmo
    for (int i = 0; i < maxIterations; ++i) { // Limite de iterações
        vector<double> w(H.size(), 0.0); // w_i = H * p_i
        for (size_t j = 0; j < H.size(); ++j) {
            for (size_t k = 0; k < H[0].size(); ++k) {
                w[j] += H[j][k] * p[k];
            }
        }

        double numerator = l2Norm(z) * l2Norm(z);
        double denominator = l2Norm(w) * l2Norm(w);
        double alpha = (denominator != 0.0) ? numerator / denominator : 0.0; // alpha_i

        if (denominator == 0.0) {
            //cerr << "Erro: divisão por zero ao calcular alpha" << endl;
        }

        for (size_t j = 0; j < f.size(); ++j) {
            f[j] += alpha * p[j]; // f_{i+1} = f_i + alpha_i * p_i
        }

        for (size_t j = 0; j < r.size(); ++j) {
            if (!isnan(alpha) && !isnan(w[j])) {
                r[j] -= alpha * w[j]; // r_{i+1} = r_i - alpha_i * w_i
            } else {
                //cerr << "Erro: valor inválido detectado em alpha ou w[" << j << "]" << endl;
            }
        }

        double normR = l2Norm(r);
        if (i % 10 == 0){
            cout << "Iteração " << i << endl;
            cout << "Norma L2 de r: " << normR << endl;
        }

        // Verificar condição de parada
        if (normR < epsilon) {
            cout << "Convergência alcançada na iteração " << i << endl;
            break;
        }

        // Atualização de z e p
        for (size_t j = 0; j < z.size(); ++j) {
            z[j] = 0; // z_{i+1} = H^T * r_{i+1}
            for (size_t k = 0; k < H.size(); ++k) {
                z[j] += H[k][j] * r[k];
            }
        }

        // Cálculo de beta e atualização de p
        double beta = l2Norm(z) * l2Norm(z) / (l2Norm(z) * l2Norm(z)); // beta_i
        for (size_t j = 0; j < p.size(); ++j) {
            p[j] = z[j] + beta * p[j]; // p_{i+1} = z_{i+1} + beta_i * p_i
        }
    }
}

// Função para salvar a imagem reconstruída
void saveImage(const vector<double>& f, const string& filename, int model = 1) {
    int width = 60;
    if (model == 2){
        width = 30;
    }
    int size = width * width;

    // Verificar se o tamanho do vetor é compativel com a imagem
    if (f.size() != size) {
        std::cerr << "O vetor deve conter exatamente " << size <<  " elementos." << std::endl;
        std::cerr << "O arquivo contém " << f.size() << " elementos." << std::endl;
        return;
    }

    // Normalizar os dados para o intervalo [0, 255]
    double min_val = *std::min_element(f.begin(), f.end());
    double max_val = *std::max_element(f.begin(), f.end());
    std::vector<uint8_t> normalized_data(size);

    for (size_t i = 0; i < f.size(); ++i) {
        normalized_data[i] = static_cast<uint8_t>(255 * (f[i] - min_val) / (max_val - min_val));
    }

    // Rotacionar a imagem 90 graus a esquerda
    std::vector<uint8_t> rotated_data(size);
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < width; ++j) {
            rotated_data[j * width + i] = normalized_data[(width - i - 1) * width + j];
        }
    }

    // Inverter a imagem horizontalmente
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < width / 2; ++j) {
            std::swap(rotated_data[i * width + j], rotated_data[i * width + width - j - 1]);
        }
    }

    // Salvar a imagem usando stb_image_write
    if (stbi_write_png(filename.c_str(), width, width, 1, rotated_data.data(), width) == 0) {
        std::cerr << "Falha ao salvar a imagem." << std::endl;
        return;
    }
    std::cout << "A imagem foi salva como " << filename << std::endl;
}

// Run cgnr and save the reconstructed image
void execute_cgnr(const vector<vector<double>>& H, const vector<double>& g, int model = 1) {
    vector<double> f(H[0].size(), 0.0); // Inicializa a imagem reconstruída

    // Executar o algoritmo CGNR
    auto start = chrono::high_resolution_clock::now();
    cgnr(H, g, f);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "Tempo de reconstrução: " << elapsed.count() << " segundos." << endl;

    // Imprimir informações da reconstrução
    cout << "Tamanho da imagem reconstruída: " << f.size() << " pixels" << endl;

    // Salvar a imagem reconstruída
    // Obter a data e hora atual
    auto now = chrono::system_clock::now();
    time_t now_time = chrono::system_clock::to_time_t(now);
    tm local_tm = *localtime(&now_time);

    // Formatar a data e hora
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &local_tm);
    string datetime(buffer);

    // Nome do arquivo com data e hora
    string filename = "imgs/reconstructed_image_" + datetime + ".png";

    // Salvar a imagem reconstruída
    saveImage(f, filename, model);
}

// Função para aplicar o ganho de sinal a um vetor de sinal
void applyGain(vector<double>& signal) {
    int N = 64;
    int S = signal.size() / N;
    for(int c = 1; c <= N; ++c) {
        for (int l = 1; l <= S; ++l) {
            double gain = 100 + (1.0 / 20.0) * l * sqrt(l);
            signal[(c - 1) * S + l - 1] *= gain;
        }
    }
}

// Main
int main() {
    cout << "Iniciando algoritmo!"<< endl;
    int model = 1;
    string h_file = "../../data/model1/H-1.csv";
    string g1_file = "../../data/model1/G-1.csv";
    string g2_file = "../../data/model1/G-2.csv";

    if(model == 2){
        h_file = "../../data/model2/H-2.csv";
        g1_file = "../../data/model2/g-30x30-1.csv";
        g2_file = "../../data/model2/g-30x30-2.csv";
    }
    // Ler os dados do arquivo
    vector<vector<double>> H;
    readData(H, h_file);
    cout << "Matriz H: " << H.size() << " x " << H[0].size() << endl;

    vector<double> g1, g2;
    readData(g1, g1_file);
    cout << "Vetor g1: " << g1.size() << endl;
    readData(g2, g2_file);
    cout << "Vetor g2: " << g2.size() << endl;

    // Aplicar o ganho de sinal
    // applyGain(g1);
    // applyGain(g2);

    // Executar o algoritmo CGNR
    execute_cgnr(H, g1, model);
    execute_cgnr(H, g2, model);

    return 0;
}