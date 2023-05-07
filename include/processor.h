#ifndef PROCESSOR_H
#define PROCESSOR_H

/*
Based on https://stackoverflow.com/a/23376195/783874:
         user    nice   system  idle      iowait irq   softirq  steal  guest  guest_nice
    cpu  74608   2520   24433   1117073   6176   4054  0        0      0      0  

so hence the following constants
*/
enum class CpuUtilizationAttributes : int {
    USER = 0,
    NICE = 1,
    SYSTEM = 2,
    IDLE = 3,
    IOWAIT = 4,
    IRQ = 5,
    SOFTIRQ = 6,
    STEAL = 7,
    GUEST = 8,
    GUEST_NICE = 9
};

class Processor {
 public:
  float Utilization();  // TODO: See src/processor.cpp

  // TODO: Declare any necessary private members
 private:
};

#endif