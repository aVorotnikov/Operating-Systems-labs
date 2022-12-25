#include "../conn.h"

Connection::Connection(pid_t pid, Type type) : hostPid_(pid), type_(type)
{
}
