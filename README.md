# Socket connect
An implementation of socket and websocket protocol for mobile and web in one purpose in C++.

## Deployment

To deploy this project clone it and run.
```bash
  $ git clone https://github.com/farhadiis/socket-connect
  $ cd socket-connect
  $ docker build -t socket-connect .
  $ docker run -p 5600:5600 socket-connect
```
## Usage
Now you can connect to this service through the socket in TCP. This connection also supports websocket frames and can be used in the web browsers.

```cgo
int main(int argc, const char *argv[]) {
    Socket socket;    
    socket.onConnect((int fd, const char *ip) { 
        LOG_INFO("onConnect %s, fd: %d", ip, fd); 
    });
    socket.onDisconnect((int fd) { 
        LOG_INFO("onSocketDisconnect fd: %d", fd);
    });
    socket.onSocketInput((int fd, unsigned char *buffer, ssize_t buffer_size) { 
        LOG_INFO("onSocketInput fd: %d", fd);
        string msg((char *) buffer, buffer_size);
        LOG_INFO("data on input: %s", msg.data());
    });
    
    LOG_INFO("Start listen at 5600");
    socket.startup();
    return 0;
}
```

## Develop
We're open for pull requests.
