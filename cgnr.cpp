#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <Eigen/Dense>

using namespace std;

// Function to read the data from the file
void readData(vector<vector<double>> &data, string filename) {
    ifstream file(filename);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            vector<double> row;
            size_t pos = 0;
            string token;
            while ((pos = line.find(',')) != string::npos) {
                token = line.substr(0, pos);
                row.push_back(stod(token));
                line.erase(0, pos + 1);
            }
            row.push_back(stod(line));
            data.push_back(row);
        }
        file.close();
    }
}


// Main
int main() {
    // Read the data from the file
    vector<vector<double>> H;
    readData(H, "Dados de testes para o modelo 1/H-1.csv");

    return 0;
}