#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"


// DONE: An example of how to read data from the filesystem
std::string LinuxParser::OperatingSystem() {
  std::string line;
  std::string key;
  std::string value;
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
std::string LinuxParser::Kernel() {
  std::string os, kernel, version;
  std::string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}


// BONUS: Update this to use std::filesystem
std::vector<int> LinuxParser::Pids() {
  std::vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      std::string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}


// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  std::string line;
  float mem_total, mem_free;

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream iss(line);
      std::string label;
      int value;

      if (iss >> label >> value) {
        if (label == "MemTotal:") {
          mem_total = value;
        } else if (label == "MemFree:") {
          mem_free = value;
          return float{mem_total - mem_free} / mem_total;  // Based on https://access.redhat.com/solutions/406773
        }
      }
    }
  }
  return mem_total - mem_free;
}


// Read and return the system uptime
long LinuxParser::UpTime() {
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  std::string line;

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream iss(line);
      float uptime_s, idle_time_s;

      if (iss >> uptime_s >> idle_time_s) {
        return static_cast<long>(uptime_s);
      } 
    }
  }
  return 0;
}


// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  long hertz = sysconf(_SC_CLK_TCK); 
  return LinuxParser::UpTime() * hertz;
}


std::ifstream GetProcFilestream(int pid, std::string filename) {
  std::ifstream filestream(
    LinuxParser::kProcDirectory
    + std::to_string(pid)
    + filename
  );
  return filestream;
}


std::vector<std::string> GetPidProcStatParts(int pid) {
  std::ifstream filestream = GetProcFilestream(pid, LinuxParser::kStatFilename);
  std::string line;

  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream iss(line);
    std::vector<std::string> parts;
    for (std::string part_str; iss >> part_str;)
      parts.push_back(part_str);
    
    return parts;
  }
  
  return {};
}


// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  std::vector<std::string> parts = GetPidProcStatParts(pid);
    
  if (parts.size() > 0) {
    // Based on https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
    long utime = std::stoi(parts[13]);
    long stime = std::stoi(parts[14]);
    long cutime = std::stoi(parts[15]);
    long cstime = std::stoi(parts[16]);
    return utime + stime + cutime + cstime;
  }

  return 0;
}


// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  std::vector<std::string> parts = CpuUtilization();
  long total = 0;
  for (std::string &part: parts)
    total += std::stoi(part);

  return total;
}


// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  // TODO: I don't think this is needed
  return 0;
}


// Read and return CPU utilization
std::vector<std::string> LinuxParser::CpuUtilization() {
  std::ifstream filestream(kProcDirectory + kStatFilename);
  std::string line;

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream iss(line);
      if (line.find("cpu ") == 0) {
        std::vector<std::string> parts;
        std::istringstream iss(line);
        for (std::string part_str; iss >> part_str; ) {
          if (part_str != "cpu")
            parts.push_back(part_str);
        }
        return parts;
      }
    }
  }
  return {};
}


// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  std::ifstream filestream(kProcDirectory + kStatFilename);
  std::string line;

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      if (line.find("processes") == 0) {
        std::istringstream iss(line);
        std::string label;
        int total_processes;
        if (iss >> label >> total_processes)
            return total_processes;
      } 
    }
  }
  return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  std::ifstream filestream(kProcDirectory + kStatFilename);
  std::string line;

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      if (line.find("procs_running") == 0) {
        std::istringstream iss(line);
        std::string label;
        int procs_running;
        if (iss >> label >> procs_running)
            return procs_running;
      } 
    }
  }
  return 0;
}

// Read and return the command associated with a process
std::string LinuxParser::Command(int pid) {
  std::ifstream filestream = GetProcFilestream(pid, LinuxParser::kCmdlineFilename);
  std::string line;

  if (filestream.is_open()) {
    if (std::getline(filestream, line))
      return line; 
  }

  return std::string();
}

// Read and return the memory used by a process
std::string LinuxParser::Ram(int pid) {
  std::ifstream filestream = GetProcFilestream(pid, LinuxParser::kStatusFilename);
  std::string line;

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      if (line.find("VmSize:") == 0) {
        std::istringstream iss(line);
        std::string vm_size, actual_value, kB;
        if (iss >> vm_size >> actual_value >> kB) {
            return actual_value;
        }
      }
    } 
  }
  return std::string(); 
}

// Read and return the user ID associated with a process
std::string LinuxParser::Uid(int pid) {
  std::ifstream filestream = GetProcFilestream(pid, kStatusFilename);
  std::string line;

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream iss(line);
      if (line.find("Uid:") == 0) {
        std::vector<std::string> parts;
        std::istringstream iss(line);
        for (std::string part_str; iss >> part_str; ) {
          if (part_str != "Uid:")
            parts.push_back(part_str);
        }
        return parts[0];
      }
    }
  }
  return std::string();
}


// Read and return the user associated with a process
std::string LinuxParser::User(int pid) {
  std::string uid = Uid(pid);

  std::ifstream filestream(LinuxParser::kPasswordPath);
  std::string line;
  std::vector<std::string> elements;

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::stringstream ss(line);
      std::string element;
      while (std::getline(ss, element, ':')) {
        elements.push_back(element);
      }
      if (elements[2] == uid) {
        return elements[0];
      }
    }
  }
  return std::string();
}


// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  std::vector<std::string> parts = GetPidProcStatParts(pid);

  long hertz = sysconf(_SC_CLK_TCK);

  // Based on: https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
  int starttime = std::stoi(parts[21]);
  return UpTime() - starttime / hertz;
}
