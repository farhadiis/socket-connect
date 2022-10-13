#include "utils.h"

namespace Utils {
    DispatchQueue GlobalQueue("GlobalQueue", 128);

    string generateRandChar(int length, bool digit) {
        const string CHARACTERS = digit ? "0123456789"
                                        : "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        std::random_device random_device;
        std::mt19937 generator(random_device());
        std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);
        std::string random_string;
        for (std::size_t i = 0; i < length; ++i) {
            random_string += CHARACTERS[distribution(generator)];
        }
        return random_string;
    }

    vector<string> split(const string &str, const string &del) {
        vector<string> tokens;
        size_t prev = 0, pos = 0;
        do {
            pos = str.find(del, prev);
            if (pos == string::npos) pos = str.length();
            string token = str.substr(prev, pos - prev);
            if (!token.empty()) tokens.push_back(token);
            prev = pos + del.length();
        } while (pos < str.length() && prev < str.length());
        return tokens;
    }

    string trim(const string &data) {
        auto lt = std::regex_replace(data, std::regex("^\\s+"), "");
        auto rt = std::regex_replace(lt, std::regex("\\s+$"), "");
        return rt;
    }

    const char *getenv(const char *key, const char *fallback) {
        const char *value = std::getenv(key);
        if (value) {
            return value;
        }
        return fallback;
    }
}