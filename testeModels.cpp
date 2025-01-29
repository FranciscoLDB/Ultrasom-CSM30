#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <Eigen/Dense>
\
using namespace std;
using namespace Eigen;

double calculate_reduction_factor(const MatrixXd& H) {
    MatrixXd H_transpose_H = H.transpose() * H;
    return H_transpose_H.norm();
}

double calculate_regularization_coefficient(const MatrixXd& H, const VectorXd& g) {
    VectorXd H_transpose_g = H.transpose() * g;
    return H_transpose_g.cwiseAbs().maxCoeff() * 0.10;
}

double calculate_error(const VectorXd& r_i, const VectorXd& r_i_plus_1) {
    return r_i_plus_1.norm() - r_i.norm();
}

VectorXd cgnr(const MatrixXd& H, const VectorXd& g, double tol = 1e-10, int max_iter = 1000) {
    MatrixXd Ht = H.transpose();
    MatrixXd HtH = Ht * H;
    VectorXd Htg = Ht * g;
    VectorXd x = VectorXd::Zero(H.cols());
    VectorXd r = Htg - HtH * x;
    VectorXd p = r;
    double rsold = r.dot(r);

    double c = calculate_reduction_factor(H);
    cout << "Reduction factor (c): " << c << endl;

    double lambda_value = calculate_regularization_coefficient(H, g);
    cout << "Regularization coefficient (lambda): " << lambda_value << endl;

    for (int i = 0; i < max_iter; ++i) {
        VectorXd Hp = HtH * p;
        double alpha = rsold / p.dot(Hp);
        x += alpha * p;
        VectorXd r_new = r - alpha * Hp;

        double epsilon = calculate_error(r, r_new);
        cout << "Iteration " << i << ", Error (epsilon): " << epsilon << endl;

        r = r_new;
        double rsnew = r.dot(r);
        if (sqrt(rsnew) < tol) {
            break;
        }
        p = r + (rsnew / rsold) * p;
        rsold = rsnew;
    }

    return x;
}

MatrixXd load_data(const string& filepath, char delimiter = ',') {
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "Erro ao carregar dados de " << filepath << endl;
        return MatrixXd();
    }

    vector<vector<double>> data;
    string line;
    while (getline(file, line)) {
        vector<double> row;
        size_t start = 0, end = 0;
        while ((end = line.find(delimiter, start)) != string::npos) {
            row.push_back(stod(line.substr(start, end - start)));
            start = end + 1;
        }
        row.push_back(stod(line.substr(start)));
        data.push_back(row);
    }

    MatrixXd matrix(data.size(), data[0].size());
    for (size_t i = 0; i < data.size(); ++i) {
        for (size_t j = 0; j < data[i].size(); ++j) {
            matrix(i, j) = data[i][j];
        }
    }

    cout << "Loaded data from " << filepath << endl;
    return matrix;
}

void model1() {
    try {
        cout << "Loading H" << endl;
        MatrixXd H = load_data("Dados de testes para o modelo 1/H-1.csv");
        cout << "Loaded H" << endl;
        cout << "Loading g" << endl;
        MatrixXd g_matrix = load_data("Dados de testes para o modelo 1/G-1.csv");
        cout << "Loaded g" << endl;

        if (H.size() == 0 || g_matrix.size() == 0) {
            return;
        }

        VectorXd g = g_matrix.col(0);

        cout << "Iniciando CGNR..." << endl;
        VectorXd reconstructed_image = cgnr(H, g);
        cout << "CGNR concluído." << endl;

        int side_length = 60;
        if (side_length * side_length != reconstructed_image.size()) {
            cerr << "Erro: O tamanho da imagem reconstruída não é um quadrado perfeito" << endl;
            return;
        }

        MatrixXd f = Map<MatrixXd>(reconstructed_image.data(), side_length, side_length);
        // Salvar a imagem reconstruída (implementar a função de salvar imagem conforme necessário)
    } catch (const exception& e) {
        cerr << "Erro ao executar o modelo 1: " << e.what() << endl;
    }
}

int main() {
    model1();
    return 0;
}
