#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <dirent.h>
#include <cstring>
#include <vector>
#include <algorithm>

// Function to display system information
void DisplaySystemInfo() {
    struct sysinfo sysInfo;
    if (sysinfo(&sysInfo) != 0) {
        std::cerr << "Error: Unable to retrieve system information." << std::endl;
        return;
    }

    std::cout << "=== System Information ===" << std::endl;
    std::cout << "Total RAM: " << sysInfo.totalram / (1024 * 1024) << " MB" << std::endl;
    std::cout << "Free RAM: " << sysInfo.freeram / (1024 * 1024) << " MB" << std::endl;
    std::cout << "Number of CPUs: " << get_nprocs() << std::endl;
    std::cout << "=========================" << std::endl << std::endl;
}

// Function to display CPU usage
void DisplayCpuUsage() {
    std::ifstream cpuStat("/proc/stat");
    std::string line;
    if (cpuStat.is_open()) {
        std::getline(cpuStat, line);
        std::istringstream iss(line);
        std::string cpuLabel;
        long user, nice, system, idle;
        iss >> cpuLabel >> user >> nice >> system >> idle;

        long total = user + nice + system + idle;
        long idleTotal = idle;
        double cpuUsage = 100.0 - ((idleTotal * 100.0) / total);

        std::cout << "=== CPU Usage ===" << std::endl;
        std::cout << "CPU Usage: " << std::fixed << std::setprecision(2) << cpuUsage << "%" << std::endl;
        std::cout << "=================" << std::endl << std::endl;
    } else {
        std::cerr << "Error: Unable to read CPU stats." << std::endl;
    }
}

// Function to display detailed information about a specific process
void DisplayProcessDetails(int pid) {
    std::cout << "=== Process Details (PID: " << pid << ") ===" << std::endl;

    // Read process status
    std::ifstream statusFile("/proc/" + std::to_string(pid) + "/status");
    if (statusFile.is_open()) {
        std::string line;
        while (std::getline(statusFile, line)) {
            if (line.find("Name:") == 0 || line.find("VmRSS:") == 0 || line.find("State:") == 0) {
                std::cout << line << std::endl;
            }
        }
    } else {
        std::cerr << "Error: Unable to read process status." << std::endl;
    }

    // Read process CPU usage
    std::ifstream statFile("/proc/" + std::to_string(pid) + "/stat");
    if (statFile.is_open()) {
        std::string line;
        std::getline(statFile, line);
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        if (tokens.size() > 21) {
            long utime = std::stol(tokens[13]);
            long stime = std::stol(tokens[14]);
            long starttime = std::stol(tokens[21]);
            long uptime = sysconf(_SC_CLK_TCK);
            double total_time = utime + stime;
            double seconds = uptime - (starttime / sysconf(_SC_CLK_TCK));
            double cpu_usage = 100.0 * ((total_time / sysconf(_SC_CLK_TCK)) / seconds);

            std::cout << "CPU Usage: " << std::fixed << std::setprecision(2) << cpu_usage << "%" << std::endl;
        }
    } else {
        std::cerr << "Error: Unable to read process stat." << std::endl;
    }

    std::cout << "=========================" << std::endl << std::endl;
}

// Function to display running processes
void DisplayRunningProcesses() {
    DIR* procDir = opendir("/proc");
    if (!procDir) {
        std::cerr << "Error: Unable to open /proc directory." << std::endl;
        return;
    }

    std::cout << "=== Running Processes ===" << std::endl;
    std::cout << std::left << std::setw(10) << "PID" << std::setw(30) << "Process Name" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    struct dirent* dirEntry;
    while ((dirEntry = readdir(procDir)) != nullptr) {
        if (dirEntry->d_type == DT_DIR) {
            std::string pidStr = dirEntry->d_name;
            if (pidStr.find_first_not_of("0123456789") == std::string::npos) {
                std::ifstream cmdlineFile("/proc/" + pidStr + "/comm");
                std::string processName;
                if (cmdlineFile.is_open()) {
                    std::getline(cmdlineFile, processName);
                    std::cout << std::left << std::setw(10) << pidStr << std::setw(30) << processName << std::endl;
                }
            }
        }
    }

    std::cout << "=========================" << std::endl << std::endl;
    closedir(procDir);
}

// Function to display advanced network information
void DisplayNetworkInfo() {
    std::cout << "=== Network Information ===" << std::endl;

    // Display active connections using `ss`
    std::cout << "Active Connections:" << std::endl;
    FILE* ssOutput = popen("ss -tunap", "r");
    if (ssOutput) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), ssOutput) != nullptr) {
            std::cout << buffer;
        }
        pclose(ssOutput);
    } else {
        std::cerr << "Error: Unable to retrieve active connections." << std::endl;
    }

    // Display firewall rules using `iptables`
    std::cout << "Firewall Rules:" << std::endl;
    FILE* iptablesOutput = popen("iptables -L -v -n", "r");
    if (iptablesOutput) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), iptablesOutput) != nullptr) {
            std::cout << buffer;
        }
        pclose(iptablesOutput);
    } else {
        std::cerr << "Error: Unable to retrieve firewall rules." << std::endl;
    }

    std::cout << "==========================" << std::endl << std::endl;
}

// Function to display hardware information
void DisplayHardwareInfo() {
    std::cout << "=== Hardware Information ===" << std::endl;

    // Display GPU information using `lspci`
    std::cout << "GPU Information:" << std::endl;
    FILE* gpuInfo = popen("lspci | grep -i vga", "r");
    if (gpuInfo) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), gpuInfo) != nullptr) {
            std::cout << buffer;
        }
        pclose(gpuInfo);
    } else {
        std::cerr << "Error: Unable to retrieve GPU information." << std::endl;
    }

    // Display disk information using `lsblk`
    std::cout << "Disk Information:" << std::endl;
    FILE* diskInfo = popen("lsblk", "r");
    if (diskInfo) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), diskInfo) != nullptr) {
            std::cout << buffer;
        }
        pclose(diskInfo);
    } else {
        std::cerr << "Error: Unable to retrieve disk information." << std::endl;
    }

    // Display CPU temperature
    std::cout << "CPU Temperature:" << std::endl;
    FILE* cpuTemp = popen("sensors | grep 'Core'", "r");
    if (cpuTemp) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), cpuTemp) != nullptr) {
            std::cout << buffer;
        }
        pclose(cpuTemp);
    } else {
        std::cerr << "Error: Unable to retrieve CPU temperature." << std::endl;
    }

    std::cout << "==========================" << std::endl << std::endl;
}

int main() {
    std::cout << "=== System Information Tool ===" << std::endl << std::endl;

    // Display system information
    DisplaySystemInfo();

    // Display CPU usage
    DisplayCpuUsage();

    // Display running processes
    DisplayRunningProcesses();

    // Display details of a specific process (e.g., PID 1)
    DisplayProcessDetails(1);

    // Display advanced network information
    DisplayNetworkInfo();

    // Display hardware information
    DisplayHardwareInfo();

    return 0;
}