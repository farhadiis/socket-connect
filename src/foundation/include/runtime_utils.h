#ifndef TRACKER_RUNTIME_UTILS_H
#define TRACKER_RUNTIME_UTILS_H

#include <execinfo.h>
#include <unistd.h>

class RuntimeUtils {
public:
    static void printStackTrace() {
        const int MAX_CALL_STACK = 100;
        void * callstack[MAX_CALL_STACK];
        int frames;

        // get void*'s for all entries on the stack...
        frames = backtrace(callstack, MAX_CALL_STACK);

        // print out all the frames to stderr...
        backtrace_symbols_fd(callstack, frames, STDERR_FILENO);
    }
};


#endif //TRACKER_RUNTIME_UTILS_H
