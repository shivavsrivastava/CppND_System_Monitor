#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() { 
    float seconds = UpTime();
    float procTime = LinuxParser::ActiveJiffies(pid_)/sysconf(_SC_CLK_TCK);

    if(seconds > 0.0)
        cpuUtil_ = procTime/seconds;
    else 
        cpuUtil_ = 0.0;
    
    return cpuUtil_;
}

// Return the command that generated this process
string Process::Command() { return LinuxParser::Command(pid_); }

// Return this process's memory utilization
string Process::Ram() {
    ram_ =  stol(LinuxParser::Ram(pid_));
    return LinuxParser::Ram(pid_); }

// Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(pid_); }

// Return the age of this process (in seconds)
long int Process::UpTime() {
    return (LinuxParser::UpTime() - (LinuxParser::UpTime(pid_)/sysconf(_SC_CLK_TCK)));
}

// Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a) const {
    return ( (cpuUtil_*100) < (100*a.cpuUtil_));
}

// Constructor
Process::Process(int pid) : pid_(pid) {}