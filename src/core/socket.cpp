#include "socket.h"

void Socket::initializeSocket() {

    /*************************************************************/
    /* Create an AF_INET stream socket to receive incoming       */
    /* connections on                                            */
    /*************************************************************/
    master_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (master_socket_fd < 0) {
        LOG_ERROR("socket() failed");
        shutdown();
        exit(EXIT_FAILURE);
    }

    /*************************************************************/
    /* Allow socket descriptor to be reusable                    */
    /*************************************************************/
    rc = setsockopt(master_socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
    if (rc < 0) {
        LOG_ERROR("setsockopt() failed");
        shutdown();
        exit(EXIT_FAILURE);
    }

    /*************************************************************/
    /* Set socket to be nonblocking. All of the sockets for      */
    /* the incoming connections will also be nonblocking since   */
    /* they will inherit that state from the listening socket.   */
    /*************************************************************/
    rc = ioctl(master_socket_fd, FIONBIO, (char *)&on);
    if (rc < 0) {
        LOG_ERROR("ioctl() failed");
        shutdown();
        exit(EXIT_FAILURE);
    }

    /*************************************************************/
    /* Bind the socket                                           */
    /*************************************************************/
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htons(INADDR_ANY);
    addr.sin_port = htons(PORT);
    rc = bind(master_socket_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (rc < 0) {
        LOG_ERROR("bind() failed");
        shutdown();
        exit(EXIT_FAILURE);
    }

    /*************************************************************/
    /* Set the listen back log                                   */
    /*************************************************************/
    rc = listen(master_socket_fd, SOMAXCONN);
    if (rc < 0) {
        LOG_ERROR("listen() failed");
        shutdown();
        exit(EXIT_FAILURE);
    }

    /*************************************************************/
    /* Initialize the pollfd structure                           */
    /*************************************************************/
    memset(fds, 0 , sizeof(fds));

    /*************************************************************/
    /* Set up the initial listening socket                        */
    /*************************************************************/
    fds[0].fd = master_socket_fd;
    fds[0].events = POLLIN;
}

void Socket::shutdown(int fd) {
    ::shutdown(fd, SHUT_RD);
}

void Socket::shutdown() {
    running = false;
    LOG_DEBUG("Destroying socket...");
    for (i = 0; i < nfds; i++) {
        if (fds[i].fd >= 0)
            close(fds[i].fd);
    }
    close(master_socket_fd);
}

void Socket::loop() {
    while (running) {
        rc = poll(fds, nfds, -1);
        if (rc <= 0) {
            LOG_ERROR("poll() failed");
            shutdown();
            exit(EXIT_FAILURE);
        }

        /***********************************************************/
        /* One or more descriptors are readable.  Need to          */
        /* determine which ones they are.                          */
        /***********************************************************/
        current_size = nfds;
        for (i = 0; i < current_size; i++) {
            /*********************************************************/
            /* Loop through to find the descriptors that returned    */
            /* POLLIN and determine whether it's the listening       */
            /* or the active connection.                             */
            /*********************************************************/
            if (fds[i].revents == 0)
                continue;

            /*********************************************************/
            /* If revents is POLLIN, otherwise log.                  */
            /*********************************************************/
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == master_socket_fd) {
                    while (acceptConnection()) {}
                } else {
                    while (readData(fds[i].fd)) {}
                }
            }
        }
        if (compress_array) {
            compress_array = false;
            for (i = 0; i < nfds; i++) {
                if (fds[i].fd == -1) {
                    for(j = i; j < nfds; j++) {
                        fds[j].fd = fds[j+1].fd;
                    }
                    i--;
                    nfds--;
                }
            }
        }
    }
}

bool Socket::acceptConnection() {

    /*****************************************************/
    /* Accept each incoming connection. If               */
    /* accept fails with EWOULDBLOCK, then we            */
    /* have accepted all of them. Any other              */
    /* failure on accept will cause us to end the        */
    /* server.                                           */
    /*****************************************************/
    struct sockaddr_in client_addr{};
    socklen_t addrLen = sizeof(client_addr);
    new_fd = accept(master_socket_fd, (struct sockaddr *) &client_addr, &addrLen);
    if (new_fd < 0) {
        if (errno != EWOULDBLOCK) {
            LOG_ERROR("accept() failed");
        }
        return false;
    }

    rc = ioctl(new_fd, FIONBIO, (char *)&on);
    if (rc < 0) {
        LOG_ERROR("ioctl() failed");
        shutdown();
        exit(EXIT_FAILURE);
    }

    /*****************************************************/
    /* Add the new incoming connection to the            */
    /* pollfd structure                                  */
    /*****************************************************/
    fds[nfds].fd = new_fd;
    fds[nfds].events = POLLIN;
    nfds++;

    /*****************************************************/
    /* Loop back up and accept another incoming          */
    /* connection                                        */
    /*****************************************************/

    if (newConnectionCallback) {
        char *client = inet_ntoa(client_addr.sin_addr);
        newConnectionCallback(new_fd, client);
    }

    return true;
}

bool Socket::readData(int fd) {

    /*****************************************************/
    /* Receive data on this connection until the         */
    /* recv fails with EWOULDBLOCK. If any other         */
    /* failure occurs, we will close the                 */
    /* connection.                                       */
    /*****************************************************/
    ssize_t len = recv(fds[i].fd, input_buffer, sizeof(input_buffer), 0);
    if (len < 0) {
        if (errno != EWOULDBLOCK) {
            LOG_ERROR("recv() failed");
            handshakes.erase(fd);
            if (disconnectCallback) {
                disconnectCallback(fd);
            }
            close(fds[i].fd);
            fds[i].fd = -1;
            bzero(&input_buffer, sizeof(input_buffer));
            compress_array = true;
        }
        return false;
    }

    /*****************************************************/
    /* Check to see if the connection has been           */
    /* closed by the client                              */
    /*****************************************************/
    if (len == 0) {
        LOG_DEBUG("close %d", fd);
        handshakes.erase(fd);
        if (disconnectCallback) {
            disconnectCallback(fd);
        }
        close(fds[i].fd);
        fds[i].fd = -1;
        bzero(&input_buffer, sizeof(input_buffer));
        compress_array = true;
        return false;
    }

    handle_message(fd, input_buffer, len);
    //memset(&input_buffer, 0, buffer_size); //zero buffer //bzero
    bzero(&input_buffer, sizeof(input_buffer));
    return true;
}

void Socket::startup() {
    initializeSocket();
    running = true;
    loop();
}

void Socket::onInput(const function<void(int, unsigned char *, ssize_t)> &handler) {
    receiveCallback = handler;
}

void Socket::onConnect(const function<void(int, const char *)> &handler) {
    newConnectionCallback = handler;
}

void Socket::onDisconnect(const function<void(int)> &handler) {
    disconnectCallback = handler;
}

ssize_t Socket::sendMessage(int fd, unsigned char *buffer, int size) {
    int hs = handshakes[fd];
    if (hs == WS) {
        unsigned char wsBuffer[size + 64];
        int wsBufferLength = ws_make_frame(BINARY_FRAME, buffer, size, wsBuffer, size + 64);
        int write_bytes = write(fd, wsBuffer, wsBufferLength);
        bzero(&wsBuffer, size + 64);
        return write_bytes;
    } else if (hs == RW) {
        return write(fd, buffer, size);
    } else {
        LOG_WARN("fd %d no handshake.", fd);
        return 0;
    }
}

char *Socket::getPeerName(int fd) {
    char *addr = nullptr;
    struct sockaddr_in client_addr{};
    socklen_t addrLen = sizeof(client_addr);
    int ret = getpeername(fd, (struct sockaddr *) &client_addr, &addrLen);
    if (ret < 0) {
        LOG_ERROR("getpeername() failed.");
    } else {
        addr = inet_ntoa(client_addr.sin_addr);
    }
    return addr;
}

void Socket::handle_message(int fd, unsigned char *buffer, ssize_t read_size) {
    try {
        if (handshakes.find(fd) == handshakes.end()) {
            LOG_DEBUG("handshake %d...", fd);
            auto data = parse_message(buffer, read_size);
            if (data.protocol.rfind("HTTP", 0) == 0) {
                string upgrade = data.headers["Upgrade"];
                if (upgrade == "websocket") {
                    auto sec_ws_key = data.headers.at("Sec-WebSocket-Key");
                    auto accept_key = ws_calc_accept_key(sec_ws_key);
                    auto response = ws_response_handshake(accept_key);
                    handshakes[fd] = WS;
                    send(fd, response.data(), response.size(), 0);
                    return;
                } else {
                    string reply = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/plain; charset=utf-8\r\n"
                                   "Content-Length: 21\r\n\r\nService is running...";
                    send(fd, reply.c_str(), reply.size(), 0);
                    return;
                }
            } else {
                handshakes[fd] = RW;
            }
        }

        int hs = handshakes[fd];
        if (hs == WS) {
            int WS_BUFFER_SIZE = 4096;
            unsigned char wsBuffer[WS_BUFFER_SIZE];
            int wsBufferLength;
            WebSocketFrameType type = ws_receive_frame(buffer, static_cast<int>(read_size), wsBuffer, WS_BUFFER_SIZE, &wsBufferLength);
            if (type == BINARY_FRAME) {
                if (strncmp((char *) wsBuffer, "ping", wsBufferLength) == 0) {
                    unsigned char pong[] = "pong";
                    unsigned char pongBuffer[128];
                    int pongBufferLength = ws_make_frame(TEXT_FRAME, pong, 4, pongBuffer, 128);
                    send(fd, pongBuffer, pongBufferLength, 0);
                    bzero(pong, 4);
                    bzero(pongBuffer, 128);
                } else {
                    if (receiveCallback) {
                        receiveCallback(fd, wsBuffer, wsBufferLength);
                    }
                }
            } else if (type == TEXT_FRAME) {
                if (strncmp((char *) wsBuffer, "ping", wsBufferLength) == 0) {
                    unsigned char pong[] = "pong";
                    unsigned char pongBuffer[128];
                    int pongBufferLength = ws_make_frame(TEXT_FRAME, pong, 4, pongBuffer, 128);
                    send(fd, pongBuffer, pongBufferLength, 0);
                    bzero(pong, 4);
                    bzero(pongBuffer, 128);
                } else {
                    LOG_ERROR("text frame not supported except ping.");
                    shutdown(fd);
                }
            } else if (type == CLOSING_FRAME) {
                shutdown(fd);
            } else {
                LOG_ERROR("socket type: %X", type);
                shutdown(fd);
            }

            bzero(wsBuffer, WS_BUFFER_SIZE);
        } else if (hs == RW) {
            if (strncmp((char *) buffer, "ping", read_size) == 0) {
                send(fd, "pong", 4, 0);
            } else {
                if (receiveCallback) {
                    receiveCallback(fd, buffer, read_size);
                }
            }
        }

    } catch (exception &e) {
        LOG_ERROR(e.what());
    } catch (...) {
        RuntimeUtils::printStackTrace();
    }
}

Message Socket::parse_message(unsigned char *buffer, ssize_t size) {
    try {
        std::string message((char *) buffer, size);
        std::istringstream ss(message);
        std::string line;
        std::getline(ss, line);
        line.pop_back();
        vector<string> tokens = Utils::split(line, " ");
        if (tokens.size() != 3) {
            throw MessageException("invalid token size.");
        }
        auto method = tokens[0];
        auto path = tokens[1];
        auto protocol = tokens[2];
        auto headers = extract_headers(buffer, size);
        if (method.empty() || path.empty() || protocol.empty() || headers.empty()) {
            throw MessageException("invalid message.");
        }
        return Message{method, path, protocol, headers};
    } catch (exception &e) {
        map<string, string> headers = {{"Upgrade", "rw"}};
        return Message{"", "", "", headers};
    }
}

map<string, string> Socket::extract_headers(unsigned char *buffer, ssize_t size) {
    std::string message((char *) buffer, size);
    std::istringstream ss(message);
    std::string line;
    map<string, string> map;
    while (std::getline(ss, line)) {
        std::istringstream is_line(line);
        std::string key;
        if (std::getline(is_line, key, ':')) {
            std::string value;
            if (std::getline(is_line, value) && !value.empty()) {
                value.pop_back();
                value = Utils::trim(value);
                map[key] = value;
            }
        }
    }
    return map;
}
