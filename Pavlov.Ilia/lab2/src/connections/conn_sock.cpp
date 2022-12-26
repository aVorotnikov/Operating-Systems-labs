#include "conn_sock.h"

#include <string>
#include <unistd.h>

std::unique_ptr<Conn> Conn::GetConn(pid_t hostPid, Type type) {
    return std::make_unique<ConnSock>(hostPid, type);
}

ConnSock::ConnSock(pid_t hostPid, Type type) : hostPid(hostPid), type(type) {
    addr.sun_family = AF_UNIX;
    std::string sock_path = ("/tmp/socket" + std::to_string(hostPid)).c_str();
    strcpy(addr.sun_path, sock_path.c_str());
    addrLen = sizeof(addr);

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        throw ("Could not create socket");
    }
    if (type == Type::HOST) {
        int rc = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
        if (rc == -1){
            throw ("Could not bind socket");
        }
    }
}

bool ConnSock::Open() {
    if (type == Type::HOST) {
        listen(sockfd, 1);
        int rc = accept(sockfd, (struct sockaddr *)&addr, &addrLen);
        if (rc == -1) {
            return false;
        }
        sockfd = rc;
    }
    else
        if (connect(sockfd, (struct sockaddr *)&addr, addrLen) == -1)
            return false;

    return true;
}

bool ConnSock::Read(void *buf, size_t count) {
    return recv(sockfd, buf, count, 0) >= 0;
}

bool ConnSock::Write(void *buf, size_t count) {
    return send(sockfd, buf, count, 0) >= 0;
}

void ConnSock::Close() {
    close(sockfd);
}
