#include "conn.h"

class ConnectionMq : public Connection
{
public:
    ConnectionMq(std::size_t id, Connection::Type type);
    ~ConnectionMq();
    bool Read(void* buf, const std::size_t count) override;
    bool Write(void* buf, const std::size_t count) override;
};
