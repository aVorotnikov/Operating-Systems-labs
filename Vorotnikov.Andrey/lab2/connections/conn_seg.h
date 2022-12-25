#include "conn.h"

class ConnectionSeg : public Connection
{
public:
    ConnectionSeg(std::size_t id, Connection::Type type);
    ~ConnectionSeg();
    bool Read(void* buf, const std::size_t count) override;
    bool Write(void* buf, const std::size_t count) override;
};
