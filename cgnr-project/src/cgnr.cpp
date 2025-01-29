#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
// #include <opencv2/opencv.hpp>
#include "utils/readData.cpp"

using namespace std;
//using namespace cv;

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
    vector<double> z(H[0].size(), 0.0); // z_0 = 0
    vector<double> p(H[0].size(), 0.0); // p_0 = 0
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
        cout << "Iteração " << i << " concluída." << endl;

        double numerator = l2Norm(z) * l2Norm(z);
        double denominator = l2Norm(w) * l2Norm(w);
        double alpha = (denominator != 0.0) ? numerator / denominator : 0.0; // alpha_i

        if (denominator == 0.0) {
            cerr << "Erro: divisão por zero ao calcular alpha" << endl;
        }

        for (size_t j = 0; j < f.size(); ++j) {
            f[j] += alpha * p[j]; // f_{i+1} = f_i + alpha_i * p_i
        }

        for (size_t j = 0; j < r.size(); ++j) {
            if (!isnan(alpha) && !isnan(w[j])) {
                r[j] -= alpha * w[j]; // r_{i+1} = r_i - alpha_i * w_i
            } else {
                cerr << "Erro: valor inválido detectado em alpha ou w[" << j << "]" << endl;
            }
        }

        // Verificar condição de parada
        cout << "Norma L2 de r: " << l2Norm(r) << endl;
        if (l2Norm(r) < epsilon) {
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
// void saveImage(const vector<double>& f, int width, int height, const string& filename) {
//     Mat image(height, width, CV_64F);
//     for (int i = 0; i < height; ++i) {
//         for (int j = 0; j < width; ++j) {
//             image.at<double>(i, j) = f[i * width + j];
//         }
//     }
//     normalize(image, image, 0, 255, NORM_MINMAX);
//     image.convertTo(image, CV_8U);
//     imwrite(filename, image);
// }

// Main
int main() {
    // Ler os dados do arquivo
    vector<vector<double>> H;
    readData(H, "../../data/H-1.csv");
    cout << "Matriz H: " << H.size() << " x " << H[0].size() << endl;

    vector<double> g1, g2;
    readData(g1, "../../data/G-1.csv");
    cout << "Vetor g1: " << g1.size() << endl;
    readData(g2, "../../data/G-2.csv");
    cout << "Vetor g2: " << g2.size() << endl;

    vector<double> f(H[0].size(), 0.0); // Inicializa a imagem reconstruída

    // Executar o algoritmo CGNR
    auto start = chrono::high_resolution_clock::now();
    cgnr(H, g1, f);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    cout << "Tempo de reconstrução: " << elapsed.count() << " segundos." << endl;

    // Imprimir informações da reconstrução
    cout << "Tamanho da imagem reconstruída: " << f.size() << " pixels" << endl;

    // Salvar a imagem reconstruída
    //saveImage(f, 60, 60, "reconstructed_image.png");

    return 0;
}