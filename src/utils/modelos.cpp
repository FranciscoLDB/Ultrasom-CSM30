#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

class ModelMatrix {
public:
    ModelMatrix(const std::string& filePath) {
        this->filePath = filePath;
        process.clear();
    }

    const std::vector<std::vector<double>>& getMatrix() {
        return matrix;
    }

    void addProcess() {
        process.push_back(1);
    }

    void removeProcess() {
        process.pop_back();
    }

    void load() {
        if (isLoaded || isLoading) {
            std::cout << "Matriz já carregada ou carregando..." << std::endl;
            return;
        }
        isLoading = true;
        std::cout << "Carregando matriz de " << filePath << std::endl;
        loadMatrix(this->filePath);
        isLoaded = true;
        isLoading = false;
    }

    void clear() {
        if (isClearing) {
            std::cout << "Matriz já está sendo limpa..." << std::endl;
            return;
        } else if (!isLoaded) {
            std::cout << "Matriz não carregada..." << std::endl;
            return;
        } else if (isLoading) {
            std::cout << "Matriz está sendo carregada..." << std::endl;
            return;
        }
        if (process.size() > 0) {
            std::cout << "Matriz não pode ser limpa enquanto houver processos ativos..." << std::endl;
            return;
        }
        isClearing = true;
        std::cout << "Limpando matriz de " << filePath << std::endl;
        matrix.clear();
        isLoaded = false;
    } 

private:
    std::vector<std::vector<double>> matrix;
    std::vector<int> process;
    std::string filePath;
    bool isLoaded = false;
    bool isLoading = false;
    bool isClearing = false;

    void loadMatrix(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filePath);
        }

        std::string line;
        while (std::getline(file, line)) {
            std::vector<double> row;
            std::stringstream ss(line);
            double value;
            while (ss >> value) {
                row.push_back(value);
                if (ss.peek() == ',') {
                    ss.ignore();
                }
            }
            matrix.push_back(row);
        }
        file.close();
        std::cout << "Matriz carregada com sucesso de " << filePath << std::endl;
    }
};
