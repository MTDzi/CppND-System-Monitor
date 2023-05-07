#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>
#include <algorithm>  // std::find

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;
using std::set;


// Return the system's CPU
Processor& System::Cpu() { 
    return cpu_;
}

// Return a container composed of the system's processes
vector<Process>& System::Processes() {
    vector<int> current_pids = LinuxParser::Pids();
    for (int &pid : current_pids) {
        
    }

    // We need to look for two types of PIDs:
    // A. Those that are in the current_pids but not in processes_
    // B. Those that are in the processes_ but not in the current_pids
    //
    // Those found in A correspond to new processes that need to be added
    //  to the processes_
    // Those found in B correspond to processes that are no longer active
    // For both, we'll create set containers to easily perform std::set_difference

    // The set of current PIDs is easy
    set<int> current_pids_set = set(current_pids.begin(), current_pids.end());
    
    // The set of tracked PIDs is a bit more complicated
    set<int> tracked_pids_set;
    std::transform(
        processes_.begin(), processes_.end(),
        std::inserter(tracked_pids_set, tracked_pids_set.begin()),
        [](const Process &proc) { return proc.Pid(); }
    );

    // Now, we can easily get the PIDs of new processes
    set<int> new_pids;
    std::set_difference(
        current_pids_set.begin(), current_pids_set.end(),
        tracked_pids_set.begin(), tracked_pids_set.end(),
        std::inserter(new_pids, new_pids.begin())
    );
    // And the dead processes' PIDs 
    set<int> dead_pids;
    std::set_difference(
        tracked_pids_set.begin(), tracked_pids_set.end(),
        current_pids_set.begin(), current_pids_set.end(),
        std::inserter(dead_pids, dead_pids.begin())
    );

    for (int new_pid : new_pids) {
        string command = LinuxParser::Command(new_pid);
        string ram = LinuxParser::Ram(new_pid);
        string user = LinuxParser::User(new_pid);
        long up_time = LinuxParser::UpTime(new_pid);

        processes_.push_back(Process(new_pid, user, command));
    }
    
    return processes_;
}

// DONE: Return the system's kernel identifier (string)
std::string System::Kernel() {
    return LinuxParser::Kernel();
}

// DONE: Return the system's memory utilization
float System::MemoryUtilization() {
    return LinuxParser::MemoryUtilization();
}

// DONE: Return the operating system name
std::string System::OperatingSystem() {
    return LinuxParser::OperatingSystem();
}

// Return the number of processes actively running on the system
int System::RunningProcesses() {
    return LinuxParser::RunningProcesses();
}

// Return the total number of processes on the system
int System::TotalProcesses() {
    return LinuxParser::TotalProcesses();
}

// Return the number of seconds since the system started running
long int System::UpTime() {
    return LinuxParser::UpTime();
}
