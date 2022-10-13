#ifndef TRACKER_SERVICE_H
#define TRACKER_SERVICE_H

#include <chrono>
#include <unordered_set>
#include "socket.h"
#include "storage.h"

using namespace std::chrono;

class Service {

private:
    Socket _socket;

    void onSocketConnect(int fd, const char *ip);
    void onSocketDisconnect(int fd);
    void onSocketInput(int fd, unsigned char *buffer, ssize_t buffer_size);

public:
    Service();

    void shutdown();
    void accept();
};


#endif //TRACKER_SERVICE_H
