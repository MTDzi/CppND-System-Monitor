#include <sstream>
#include <string>
#include <vector>

#include "processor.h"

#include "linux_parser.h"

// Return the aggregate CPU utilization
float Processor::Utilization() {
    std::vector<std::string> parts = LinuxParser::CpuUtilization();
    std::vector<int> parts_int;
    for (std::string& part_str : parts)
        parts_int.push_back(std::stoi(part_str));
    
    /*
    Based on https://stackoverflow.com/a/23376195/783874:
        CPU_Percentage = (Total - Idle) / Total
    where:
        Total = Idle + NonIdle
        Idle = idle + iowait
        NonIdle = user + nice + system + irq + softirq + steal
    */
    int Idle = (
        parts_int[static_cast<int>(CpuUtilizationAttributes::IDLE)]
        + parts_int[static_cast<int>(CpuUtilizationAttributes::IOWAIT)]
    );
    int Total = (
        parts_int[static_cast<int>(CpuUtilizationAttributes::IDLE)]
        + parts_int[static_cast<int>(CpuUtilizationAttributes::USER)]
        + parts_int[static_cast<int>(CpuUtilizationAttributes::NICE)]
        + parts_int[static_cast<int>(CpuUtilizationAttributes::SYSTEM)]
        + parts_int[static_cast<int>(CpuUtilizationAttributes::IRQ)]
        + parts_int[static_cast<int>(CpuUtilizationAttributes::SOFTIRQ)]
        + parts_int[static_cast<int>(CpuUtilizationAttributes::STEAL)]
    );

    return static_cast<float>(Total - Idle) / Total;
}