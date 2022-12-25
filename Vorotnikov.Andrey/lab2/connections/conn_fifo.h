#include "conn.h"

class ConnectionFifo : public Connection
{
public:
    ConnectionFifo(std::size_t id, Connection::Type type);
    ~ConnectionFifo();
    bool Read(void* buf, const std::size_t count) override;
    bool Write(void* buf, const std::size_t count) override;
};
