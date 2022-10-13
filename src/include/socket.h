#ifndef TRACKER_SOCKET_H
#define TRACKER_SOCKET_H

#include <netdb.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unordered_map>
#include "logger.h"
#include "runtime_utils.h"
#include "web_socket.h"

#define PORT 5600

class MessageException : public exception {
    string _message;
public:
    explicit MessageException(string message) : _message(move(message)) {}
    const char *what() const noexcept override {
        return _message.c_str();
    }
};

struct Message {
    string method;
    string path;
    string protocol;
    map<string, string> headers;
};

class Socket {

private:
    unordered_map<int, int> handshakes;

    static const int WS = 1;
    static const int RW = 2;

    int master_socket_fd = -1, new_fd = -1;
//    int master_poll_fd;
    unsigned char input_buffer[4096];
    bool compress_array = false;
    volatile bool running = false;
    struct sockaddr_in addr;
    struct pollfd fds[65536];
    int nfds = 1, current_size = 0, i, j, rc, on = 1;

    function<void(int, const char *)> newConnectionCallback = nullptr;
    function<void(int, unsigned char *, ssize_t)> receiveCallback = nullptr;
    function<void(int)> disconnectCallback = nullptr;

    void initializeSocket();
    bool acceptConnection();
    bool readData(int fd);
    void loop();

    void handle_message(int fd, unsigned char *buffer, ssize_t size);
    Message parse_message(unsigned char *buffer, ssize_t size);
    map<string, string> extract_headers(unsigned char *buffer, ssize_t size);

public:
    char *getPeerName(int fd);
    void shutdown(int fd);
    void shutdown();
    void startup();

    //callback setters
    void onConnect(const function<void(int, const char *)> &handler);
    void onDisconnect(const function<void(int)> &handler);
    void onInput(const function<void(int, unsigned char *, ssize_t)> &handler);

    ssize_t sendMessage(int fd, unsigned char *buffer, int size);

};

#endif //TRACKER_SOCKET_H