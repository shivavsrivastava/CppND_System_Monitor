#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include "processor.h"
#include "linux_parser.h"
using std::stol;
using std::string;
using std::vector;



// Return the aggregate CPU utilization
/*
     user    nice   system  idle      iowait irq   softirq  steal  guest  guest_nice
cpu  74608   2520   24433   1117073   6176   4054  0        0      0      0
*/
float Processor::Utilization() { 
    vector<string> util_array;

    // Get the first batch of CPU utilization
    util_array = LinuxParser::CpuUtilization();
    float cpu_util1 = 0.0;
    float cpu_idle1 = 0.0;
    
     cpu_util1 = stol(util_array[LinuxParser::CPUStates::kUser_]) + stol(util_array[LinuxParser::CPUStates::kNice_]) 
                    + stol(util_array[LinuxParser::CPUStates::kSystem_]) + stol(util_array[LinuxParser::CPUStates::kIRQ_]) 
                    + stol(util_array[LinuxParser::CPUStates::kSoftIRQ_]) + stol(util_array[LinuxParser::CPUStates::kSteal_]);
     cpu_idle1 = stol(util_array[LinuxParser::CPUStates::kIdle_]) + stol(util_array[LinuxParser::CPUStates::kIOwait_]);

    // Now invoke sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));//sleep for 1000 m/s

    // Get the second batch of CPU utilization
    util_array = LinuxParser::CpuUtilization();
    float cpu_util2 = 0.0;
    float cpu_idle2 = 0.0;

     cpu_util2 = stol(util_array[LinuxParser::CPUStates::kUser_]) + stol(util_array[LinuxParser::CPUStates::kNice_]) 
                    + stol(util_array[LinuxParser::CPUStates::kSystem_]) + stol(util_array[LinuxParser::CPUStates::kIRQ_]) 
                    + stol(util_array[LinuxParser::CPUStates::kSoftIRQ_]) + stol(util_array[LinuxParser::CPUStates::kSteal_]);
     cpu_idle2 = stol(util_array[LinuxParser::CPUStates::kIdle_]) + stol(util_array[LinuxParser::CPUStates::kIOwait_]);

    float totalDiff = cpu_util2 + cpu_idle2 - cpu_idle1 - cpu_util1;
    float cpuUtilD  = cpu_util2 - cpu_util1;


    return cpuUtilD/totalDiff;
 }