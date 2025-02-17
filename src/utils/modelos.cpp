#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

class ModelMatrix {
public:
    bool isLoaded = false;
    bool isLoading = false;
    bool isClearing = false;

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
        if (isLoaded) {
            std::cout << "[MATRIZ] Matriz já carregada... Modelo: " << this->filePath.substr(39) << std::endl;
            return;
        }
        while (isLoading || isClearing) {
            std::cout << "[MATRIZ] Aguarde a matriz ser carregada ou limpa... Modelo: " << this->filePath.substr(39) << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            if (isLoaded) {
                return;
            }
        }
        isLoading = true;
        std::cout << "[MATRIZ] Carregando matriz " << filePath.substr(39) << std::endl;
        loadMatrix(this->filePath);
        isLoaded = true;
        isLoading = false;
    }

    void clear() {
        if (isClearing) {
            std::cout << "[MATRIZ] Matriz já está sendo limpa..." << std::endl;
            return;
        } else if (!isLoaded) {
            std::cout << "[MATRIZ] Matriz não carregada..." << std::endl;
            return;
        } else if (isLoading) {
            std::cout << "[MATRIZ] Matriz está sendo carregada..." << std::endl;
            return;
        }
        if (process.size() > 0) {
            std::cout << "[MATRIZ] Matriz não pode ser limpa enquanto houver processos ativos..." << std::endl;
            return;
        }
        isClearing = true;
        std::cout << "[MATRIZ] Limpando " << filePath.substr(39) << std::endl;
        matrix.clear();
        isClearing = false;
        isLoaded = false;
    } 

private:
    std::vector<std::vector<double>> matrix;
    std::vector<int> process;
    std::string filePath;

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
        std::cout << "[MATRIZ] Matriz carregada com sucesso de " << filePath.substr(39) << std::endl;
    }
};
