#include "conn_sock.h"

#include <string>
#include <unistd.h>

Conn* Conn::GetConn(pid_t hostPid, Type type) {
    return new ConnSock(hostPid, type);
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
        /*
        timeval t;
        t.tv_sec = 5;
        t.tv_usec = 0;
        fd_set sockr, sockw;
        FD_ZERO(&sockr);
        FD_ZERO(&sockw);
        FD_SET(sockfd, &sockr);
        FD_SET(sockfd, &sockw);
        listen(sockfd, 1);
        int rc = select(sockfd+1, &sockr, &sockw, nullptr, &t);
        if (rc == -1)
            return false;
        rc = accept(sockfd, (struct sockaddr *)&addr, &addrLen);
        if (rc == -1)
            return false;*/
        listen(sockfd, 1);
        int rc = accept(sockfd, (struct sockaddr *)&addr, &addrLen);
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
