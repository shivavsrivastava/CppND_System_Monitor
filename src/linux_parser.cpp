#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>
//#include <experimental/filesystem>

#include "linux_parser.h"

using std::stof;
using std::stoi;
using std::string;
using std::to_string;
using std::vector;
//namespace fs = std::experimental::filesystem;


// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);

  //My code - cannot be used in the current system
  // string pPath = kProcDirectory.c_str();
  // if(fs::exists(pPath) && fs::is_directory(pPath)) {
  //   // get all the digit directories 
  //   for(const auto& entry : fs::directory_iterator(pPath)) {
  //     string filename = entry.path().filename();
  //     std::cout << "Filename" << filename << std::endl;
  //     if(fs::is_directory(entry.status())) {
  //       if (std::all_of(filename.begin(), filename.end(), isdigit)) {
  //         int pid = std::stoi(filename);
  //         pids.push_back(pid);
  //       }
  //     }
  //   }
  // }

  
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  string line;
  string key;
  string value;
  float memTotal;
  float memFree;
  float memUsedPercent;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal") {
          memTotal = stof(value);
        }
        if (key == "MemFree") {
          memFree = stof(value);
        }
      }
    }
  }
  if(memTotal>0 && memFree>0) {
  	memUsedPercent = (memTotal - memFree)/memTotal;
  }
  return memUsedPercent; 
}

// Read and return the system uptime
long LinuxParser::UpTime() { 
  string value;
  string line;
  long uptime;
  float temp;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> value;
    temp = stof(value);
    uptime = abs(temp);
    //std::cout << "Capturing uptime float vlaue= " << temp << " absolute value= " << uptime << std::endl;
  }
  return uptime;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  long uptime = UpTime();
  return uptime;
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) { 
  string line;
  string val;
  int count=1;
  long utime, stime, cutime, cstime;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> val) { 
        if((count==14) && (std::all_of(val.begin(), val.end(), isdigit)))
          utime = stol(val);
        else
          utime = 0;
        if((count==15) && (std::all_of(val.begin(), val.end(), isdigit)))
          stime = stol(val);
        else
          stime = 0;  
        if((count==16) && (std::all_of(val.begin(), val.end(), isdigit)))
          cutime = stol(val);
        else
          cutime = 0;
        if((count==17) && (std::all_of(val.begin(), val.end(), isdigit)))
          cstime = stol(val);
        else
          cstime = 0;
        count += 1;
      }
    }
  }
  return (utime + stime + cutime + cstime);
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  vector<string> util_array = CpuUtilization();
  long active_jiffies = stol(util_array[CPUStates::kUser_]) + stol(util_array[CPUStates::kNice_]) 
                    + stol(util_array[CPUStates::kSystem_]) + stol(util_array[CPUStates::kIRQ_]) 
                    + stol(util_array[CPUStates::kSoftIRQ_]) + stol(util_array[CPUStates::kSteal_]);
  return active_jiffies;
              
 }

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> util_array = CpuUtilization();
  long idle_jiffies = stol(util_array[CPUStates::kIdle_]) + stol(util_array[CPUStates::kIOwait_]);
  return idle_jiffies;
}

// Read and return CPU utilization
/*
     user    nice   system  idle      iowait irq   softirq  steal  guest  guest_nice
cpu  74608   2520   24433   1117073   6176   4054  0        0      0      0
*/
vector<string> LinuxParser::CpuUtilization() { 

  vector<string> temp;
  string line;
  string tok;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    // get  the first line
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> tok; // ignore the first word "cpu"
    while(linestream) {
      linestream >> tok;
      temp.push_back(tok);
    }
  }

  return temp;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  string line;
  string key;
  string value;
  int totalP;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          if (std::all_of(value.begin(), value.end(), isdigit)) {
            totalP = stoi(value);
          }
          return totalP;
        }
      }
    }
  }
  return totalP;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  string line;
  string key;
  string value;
  int runP;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          if (std::all_of(value.begin(), value.end(), isdigit)) {
            runP = stoi(value);
          }
          return runP;
        }
      }
    }
  }
  return runP;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string line; 
  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if(filestream.is_open()) {
    std::getline(filestream, line);
    return line;
  }
  return line; 
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) { 
  string line;
  string key;
  string value;
  long memVal;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize:") {
          if (std::all_of(value.begin(), value.end(), isdigit)) {
            memVal = stol(value)/1024;
            return to_string(memVal);
          }
        }
      }
    }
  }
  return to_string(memVal);
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line;
  string key;
  string value;
  string uid;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid:") {
          uid = value;
          //std::cout << "User Uid " << uid << std::endl;
          return uid;
        }
      }
    }
  }
  return uid;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string line;
  string key;
  string v1, v2;
  string uid;
  string user;
  uid = Uid(pid);
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      //std::cout << line << std::endl;
      std::istringstream linestream(line);
      while (linestream >> key >> v1 >> v2) {
        if (uid == v2) {
          user = key;
          return user;
        }
      }
    }
  }
  return user;
}


// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line;
  string val;
  int count=1;
  long starttime;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> val) { 
        if(count==22) {
          starttime = stol(val);
          return starttime;
        }
        count += 1;
      }
    }
  }
  return starttime;
 }
