#include <string>
#include <sstream>

#include "format.h"

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
    std::ostringstream oss;
    long hours = seconds / (60 * 60);
    if (hours < 10) {
        oss << "0";
    }
    oss << hours << ":";

    long minutes = (seconds - hours * 60 * 60) / 60;
    if (minutes < 10) {
        oss << "0";
    }
    oss << minutes << ":";

    seconds = seconds % 60;
    if (seconds < 10) {
        oss << "0";
    }
    oss << seconds;
     
    return oss.str();
}