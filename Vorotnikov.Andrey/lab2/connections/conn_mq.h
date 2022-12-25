#include "conn.h"

class ConnectionMq : public Connection
{
public:
    ConnectionMq(pid_t pid, Connection::Type type);
    ~ConnectionMq();
    bool Read(void* buf, const std::size_t count) override;
    bool Write(void* buf, const std::size_t count) override;
};
