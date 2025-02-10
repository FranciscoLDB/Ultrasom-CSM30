#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <sys/resource.h>
#include <unistd.h>
#include <iomanip>
#include <sstream>

void logPerformance(const std::string& filename, int interval_seconds, int duration_seconds) {
    std::ofstream report(filename);
    if (!report.is_open()) {
        std::cerr << "Failed to open report file." << std::endl;
        return;
    }

    report << "Timestamp, CPU Usage (%), Memory Usage (KB)" << std::endl;

    auto start_time = std::chrono::steady_clock::now();
    auto end_time = start_time + std::chrono::seconds(duration_seconds);

    long prevIdleTime = 0, prevTotalTime = 0;

    std::cout << "Logging performance data to " << filename << " every " << interval_seconds << " seconds." << std::endl;
    while (std::chrono::steady_clock::now() < end_time) {
        // Get CPU usage
        std::ifstream file("/proc/stat");
        std::string line;
        std::getline(file, line);
        std::istringstream iss(line);

        std::string cpu;
        long user, nice, system, idle, iowait, irq, softirq, steal;
        iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

        long idleTime = idle + iowait;
        long totalTime = user + nice + system + idle + iowait + irq + softirq + steal;

        double percent = 0.0;
        if (prevTotalTime != 0) {
            long totalDiff = totalTime - prevTotalTime;
            long idleDiff = idleTime - prevIdleTime;
            percent = (totalDiff - idleDiff) * 100.0 / totalDiff;
        }

        prevIdleTime = idleTime;
        prevTotalTime = totalTime;

        // Get memory usage
        std::ifstream meminfo("/proc/meminfo");
        long totalMemory = 0, freeMemory = 0;
        while (std::getline(meminfo, line)) {
            if (line.find("MemTotal:") == 0) {
                std::istringstream iss(line);
                std::string key;
                iss >> key >> totalMemory;
            } else if (line.find("MemFree:") == 0) {
                std::istringstream iss(line);
                std::string key;
                iss >> key >> freeMemory;
            }
            if (totalMemory != 0 && freeMemory != 0) {
                break;
            }
        }
        long usedMemory = totalMemory - freeMemory;

        // Log the data
        auto timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        report << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S") << ", " << percent << ", " << usedMemory << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
    }

    report.close();
}
