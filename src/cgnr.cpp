#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <algorithm>
#include <cblas.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "utils/stb_image_write.h"
#include "utils/readData.cpp"

using namespace std;

struct imagem {
    int algoritmo;
    int tamanho;
    int numIteracoes;
    double tempo;
    std::string usuario;
    std::string dataInicio;
    std::string dataFim;
    std::string path;
};

double norm2(const vector<double>& v) {
    double sum = 0.0;
    for (double val : v) {
        sum += val * val;
    }
    return sqrt(sum);
}

void matVecMult(const vector<vector<double>>& mat, const vector<double>& vec, vector<double>& result) {
    int rows = mat.size();
    int cols = mat[0].size();
    result.assign(rows, 0.0);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result[i] += mat[i][j] * vec[j];
        }
    }
}

void transposeMatVecMult(const vector<vector<double>>& mat, const vector<double>& vec, vector<double>& result) {
    int rows = mat.size();
    int cols = mat[0].size();
    result.assign(cols, 0.0);
    for (int j = 0; j < cols; ++j) {
        for (int i = 0; i < rows; ++i) {
            result[j] += mat[i][j] * vec[i];
        }
    }
}

int cgnr(const vector<vector<double>>& H, const vector<double>& g, vector<double>& f) {
    int maxIterations = 20;
    double tolerance = 1e-4;
    int iterations = 0;
    cout << "Iniciando CGNR..." << endl;
    cout << "Tamanho do vetor g: " << g.size() << endl;

    auto start = chrono::high_resolution_clock::now();
    int m = H.size();
    int n = H[0].size();

    f.assign(n, 0.0);
    vector<double> r(m, 0.0), z(n, 0.0), p(n, 0.0), w(m, 0.0);
    vector<double> temp(m, 0.0);

    matVecMult(H, f, temp);
    for (int i = 0; i < m; ++i) {
        r[i] = g[i] - temp[i];
    }

    transposeMatVecMult(H, r, z);
    p = z;

    while (iterations < maxIterations) {
        matVecMult(H, p, w);
        double alpha = norm2(z) * norm2(z) / norm2(w) / norm2(w);

        for (int i = 0; i < n; ++i) {
            f[i] += alpha * p[i];
        }

        for (int i = 0; i < m; ++i) {
            r[i] -= alpha * w[i];
        }

        vector<double> z_next(n, 0.0);
        transposeMatVecMult(H, r, z_next);

        double Nr = norm2(r);
        if (iterations % 10 == 0) {
            cout << "Iteração " << iterations << ": norm2(r) = " << Nr << endl;
        }
        if (norm2(r) < tolerance) {
            break;
        }

        double beta = norm2(z_next) * norm2(z_next) / norm2(z) / norm2(z);
        for (int i = 0; i < n; ++i) {
            p[i] = z_next[i] + beta * p[i];
        }

        z = z_next;
        ++iterations;
    }

    return iterations;
}

// Função para salvar a imagem reconstruída
void saveImage(const vector<double>& f, const string& filename, imagem& img, int model = 1) {
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

    img.tamanho = size;
    std::cout << "A imagem foi salva como " << filename << std::endl;
}

// Run cgnr and save the reconstructed image
imagem execute_cgnr(const vector<vector<double>>& H, const vector<double>& g, int model = 1) {
    vector<double> f(H[0].size(), 0.0); // Inicializa a imagem reconstruída
    imagem img;
    img.algoritmo = model;
    img.tamanho = f.size();

    auto start = chrono::high_resolution_clock::now();

    // Obter a data e hora atual
    auto start_now = chrono::system_clock::now();
    time_t start_time = chrono::system_clock::to_time_t(start_now);
    tm start_local_tm = *localtime(&start_time);

    // Formatar a data e hora
    char start_buffer[80];
    strftime(start_buffer, sizeof(start_buffer), "%Y-%m-%d %H:%M:%S", &start_local_tm);
    img.dataInicio = string(start_buffer);

    int iteracoes = cgnr(H, g, f);
    img.numIteracoes = iteracoes;

    auto end = chrono::high_resolution_clock::now();
    auto end_now = chrono::system_clock::now();
    time_t end_time = chrono::system_clock::to_time_t(end_now);
    tm end_local_tm = *localtime(&end_time);

    // Formatar a data e hora
    char end_buffer[80];
    strftime(end_buffer, sizeof(end_buffer), "%Y-%m-%d %H:%M:%S", &end_local_tm);
    img.dataFim = string(end_buffer);

    chrono::duration<double> elapsed = end - start;
    cout << "Tempo de reconstrução: " << elapsed.count() << " segundos." << endl;
    img.tempo = elapsed.count();

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
    string filename = "./server_files/imgs/" + datetime + ".png";
    img.path = filename;

    // Salvar a imagem reconstruída
    saveImage(f, filename, img, model);

    return img;
}