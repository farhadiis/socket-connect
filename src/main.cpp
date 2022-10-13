#include "runtime_utils.h"
#include "logger.h"
#include "service.h"

using namespace std;

Service *service = nullptr;

void handleUserInterrupt(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nSIGINT trapped ..." << std::endl;
        if (service) {
            service->shutdown();
            delete service;
        }
    } else if (signal == SIGTSTP) {
        std::cout << "\nCannot execute Ctrl+Z" << std::endl;
    }
}

int main(int argc, const char *argv[]) {
    signal(SIGINT, handleUserInterrupt);
    signal(SIGTSTP, handleUserInterrupt);
    try {
        service = new Service();
        LOG_INFO("Start app...");
        service->accept();
    } catch (exception &e) {
        LOG_ERROR(e.what());
    } catch (...) {
        RuntimeUtils::printStackTrace();
    }
    return EXIT_SUCCESS;
}
