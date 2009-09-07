#include "acceptor.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
using namespace std;

int
setnonblock(int fd) {
    int flags;

    flags = fcntl(fd, F_GETFL);
    if (flags < 0)
        return flags;
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0)
        return -1;

    return 0;
}

void Acceptor::setCallback(void(*cb)(int)) {
    m_cb = cb;
}

void Acceptor::newClient(ev::io& iow, int revents) {
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof (client_addr);
    client_fd = accept(iow.fd, (struct sockaddr *) & client_addr, &client_len);
    if (client_fd == -1) {
        return;
    }
    setnonblock(client_fd);
    m_cb(client_fd);
}

Acceptor::Acceptor(const char* ip, int port) {
    int fd;
    struct sockaddr_in listen_addr;
    int reuseaddr_on = 1;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on,
            sizeof (reuseaddr_on)) == -1)
        exit(1);
    memset(&listen_addr, 0, sizeof (listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = inet_addr((char *)ip);
    listen_addr.sin_port = htons(port);
    if (bind(fd, (struct sockaddr *) & listen_addr,
            sizeof (listen_addr)) < 0)
        exit(1);
    if (listen(fd, 5) < 0)
        exit(1);
    if (setnonblock(fd) < 0)
        exit(1);

    iow.set<Acceptor,&Acceptor::newClient>(this);
    iow.set(loop);
    iow.start(fd, ev::READ);
}

void Acceptor::operator ()(){
        loop.loop();
}
