#include <string>

#include "format.h"

using std::string;
using std::to_string;


// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
    string  hours = to_string(seconds/3600);
    string mins = to_string((seconds%3600)/60);
    string secs = to_string(seconds%60);
    // adding leading zero
    string hh = string(2 - hours.length(), '0') + hours;
    string mm = string(2 - mins.length(), '0') + mins;
    string ss = string(2 - secs.length(), '0') + secs;
    return (hh + ':' + mm + ":" + ss);
 }