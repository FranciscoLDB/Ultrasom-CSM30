#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>
#include <atomic>
#include <chrono>
#include <fstream>
#include <sstream>
#include "../cgnr.cpp"
#include "modelos.cpp"

class SignalProcess {
public:
    std::vector<double> signal;
    int model;
    std::string user;

    SignalProcess(const std::vector<double>& sig, int mdl, const std::string& usr)
        : signal(sig), model(mdl), user(usr) {}

};

class SignalProcessController {
    public:
        SignalProcessController() : stop(false), 
        ModelH1("/workspaces/Ultrasom-CSM30/data/model1/H-1.csv"), ModelH2("/workspaces/Ultrasom-CSM30/data/model2/H-2.csv") {
            monitorThread = std::thread(&SignalProcessController::monitorCPU, this);
        }

        ~SignalProcessController() {
            stop = true;
            cv.notify_all();
            if (monitorThread.joinable()) {
                monitorThread.join();
            }
        }

        void addProcess(const SignalProcess process) {
            std::lock_guard<std::mutex> lock(queueMutex);
            processQueue.push(process);
            cv.notify_one();
        }

    private:
        std::queue<SignalProcess> processQueue;
        std::mutex queueMutex;
        std::condition_variable cv;
        std::atomic<bool> stop;
        std::thread monitorThread;
        ModelMatrix ModelH1;
        ModelMatrix ModelH2;

        void monitorCPU() {
            while (!stop) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                double cpuUsage = getCPUUsage();
                if (cpuUsage < 75.0) {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    if (!processQueue.empty()) {
                        SignalProcess process = processQueue.front();
                        processQueue.pop();
                        lock.unlock();
                        std::thread(&SignalProcessController::executeProcess, this, process).detach();
                    } else {
                        cv.wait(lock);
                    }
                } else {
                    std::cout << "CPU usage high: " << cpuUsage << "%, waiting...\n";
                }
            }
        }

        double getCPUUsage() {
            std::ifstream file("/proc/stat");
            std::string line;
            std::getline(file, line);
            std::istringstream iss(line);
            std::string cpu;
            long user, nice, system, idle;
            iss >> cpu >> user >> nice >> system >> idle;
            static long prevTotal = 0;
            static long prevIdle = 0;
            long total = user + nice + system + idle;
            long totalDiff = total - prevTotal;
            long idleDiff = idle - prevIdle;
            prevTotal = total;
            prevIdle = idle;
            return (totalDiff - idleDiff) * 100.0 / totalDiff;
        }

        void executeProcess(SignalProcess process) {
            // Placeholder for actual signal processing
            std::cout << "Processing signal for user: " << process.user << " with model: " << process.model << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5)); // Simulate processing time
            imagem img;
            if (process.model == 1) {
                ModelH1.load();
                ModelH1.addProcess();
                img = execute_cgnr(ModelH1.getMatrix(), process.signal, process.model);
                ModelH1.removeProcess();
                ModelH1.clear();
            } else if (process.model == 2) {
                ModelH2.load();
                ModelH2.addProcess();
                img = execute_cgnr(ModelH2.getMatrix(), process.signal, process.model);
                ModelH2.removeProcess();
                ModelH2.clear();
            } else {
                std::cerr << "Erro: modelo inválido\n";
            }
            img.usuario = process.user;
            //std::cout << "Imagem reconstruída: " << img.path << std::endl;
            // std::cout << "==================================================================" << std::endl;
            // std::cout << "=| Usuario:    " << img.usuario << std::endl;
            // std::cout << "=| Img gerada: " << img.path << std::endl;
            // std::cout << "=| Algoritmo:  " << img.algoritmo << std::endl;
            // std::cout << "=| DataInicio: " << img.dataInicio << std::endl;
            // std::cout << "=| DataFim:    " << img.dataFim << std::endl;
            // std::cout << "=| Tamanho:    " << sqrt(img.tamanho) << "x" << sqrt(img.tamanho) << std::endl;
            // std::cout << "=| NumIteracoes: " << img.numIteracoes << std::endl;
            // std::cout << "===================================================================" << std::endl;

            saveToFile(img);
            std::cout << "Finished processing signal for user: " << process.user << std::endl;
        }

        void saveToFile(const imagem img) {
            std::ofstream file("./server_files/images.csv", std::ofstream::app);
            if (!file.is_open()) {
                std::cerr << "Erro ao criar o arquivo\n";
                return;
            }
            // Formato: usuario,algoritmo,dataInicio,dataFim,tamanho,numIteracoes,path,tempo
            file << img.usuario << "," << img.algoritmo << "," << img.dataInicio << "," << img.dataFim << ",";
            file << img.tamanho << "," << img.numIteracoes << "," << img.path << "," << img.tempo <<"\n";
            file.close();
        }
};