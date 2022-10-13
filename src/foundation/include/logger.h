#ifndef TRACKER_LOGGER_H
#define TRACKER_LOGGER_H

#include <iostream>
#include <fstream>
#include <mutex>
#include <cstring>
#include <cstdarg>
#include <cerrno>
#include "utils.h"
#include "manifest.h"

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

class Logger {
private:
    static char *timestamp();
    static int vscprintf(const char *fmt, va_list args);

public:
    static void logInfo(const char *fmt, ...);
    static void logWarn(const char *fmt, ...);
    static void logError(const char *fmt, ...);
    static void logDebug(const char *fmt, ...);
};

#define _FILE strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__

#define LOG_INFO(format, args...)   Logger::logInfo(format, ## args)
#define LOG_WARN(format, args...)   Logger::logWarn(format, ## args)
#define LOG_ERROR(format, args...)  Logger::logError(format, ## args)
#ifdef SERVER_DEBUG
#define LOG_DEBUG(format, args...)  Logger::logDebug(format, ## args)
#else
#define LOG_DEBUG(format, args...)
#endif

#endif //TRACKER_LOGGER_H
