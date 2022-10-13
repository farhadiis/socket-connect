#ifndef TRACKER_UTILS_H
#define TRACKER_UTILS_H

#include "dispatch_queue.h"
#include <string>
#include <regex>
#include <random>

using namespace std;

namespace Utils {
    extern DispatchQueue GlobalQueue;

    string generateRandChar(int length, bool digit);
    vector<string> split(const string &str, const string &del);
    string trim(const string& data);
    const char *getenv(const char *key, const char *fallback);
}

#endif //TRACKER_UTILS_H
