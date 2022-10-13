#include "service.h"

Service::Service() {
    _socket.onConnect([this](auto &&PH1, auto &&PH2) { onSocketConnect(PH1, PH2); });
    _socket.onDisconnect([this](auto PH1) { onSocketDisconnect(PH1); });
    _socket.onInput([this](auto &&PH1, auto &&PH2, auto &&PH3) { onSocketInput(PH1, PH2, PH3); });
}

void Service::shutdown() {
    _socket.shutdown();
}

void Service::accept() {
    LOG_INFO("Listen at: 5600");
    _socket.startup();
}

void Service::onSocketConnect(int fd, const char *ip) {
    LOG_INFO("New connection %s, fd: %d", ip, fd);
}

void Service::onSocketDisconnect(int fd) {
    LOG_DEBUG("onSocketDisconnect fd: %d", fd);
}

void Service::onSocketInput(int fd, unsigned char *buffer, ssize_t buffer_size) {
    LOG_DEBUG("onSocketInput fd: %d", fd);
    string msg((char *) buffer, buffer_size);
    Utils::GlobalQueue.dispatch([this, fd, msg] {
        LOG_DEBUG("data on input: %s", msg.data());
    });
}