#include "logger.h"

void Logger::logInfo(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int nSize = vscprintf(fmt, args);
    char vars[nSize];
    vsprintf(vars, fmt, args);
    va_end(args);
    char *t = timestamp();

    std::cout << t << " [info] " << vars << std::endl;
}

void Logger::logWarn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int nSize = vscprintf(fmt, args);
    char vars[nSize];
    vsprintf(vars, fmt, args);
    va_end(args);
    char *t = timestamp();

    std::cout << KYEL << t << " [warn] " << vars << KNRM << std::endl;
}

void Logger::logError(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int nSize = vscprintf(fmt, args);
    char vars[nSize];
    vsprintf(vars, fmt, args);
    va_end(args);
    char *t = timestamp();

    std::cout << KRED << t << " [error] " << vars << KNRM << std::endl;
}

void Logger::logDebug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int nSize = vscprintf(fmt, args);
    char vars[nSize];
    vsprintf(vars, fmt, args);
    va_end(args);
    char *t = timestamp();

    std::cout << KBLU << t << " [debug] " << vars << KNRM << std::endl;
}

char *Logger::timestamp() {
    static char buffer[32];
    auto t = std::time(nullptr);
    auto tm = std::localtime(&t);
    strftime(buffer, 32, "%Y-%m-%dT%H:%M:%S.%Z", tm);
    return buffer;
}

int Logger::vscprintf(const char *fmt, va_list args) {
    int ret;
    va_list arg_copy;
    va_copy(arg_copy, args);
    ret = vsnprintf(nullptr, 0, fmt, arg_copy);
    va_end(arg_copy);
    return ret;
}