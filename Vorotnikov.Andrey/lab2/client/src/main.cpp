#include "../lib/client.h"

#include <sys/syslog.h>
#include <string>

namespace
{

bool ParseArgs(int argc, char* argv[], pid_t& hostPid)
{
    if (2 != argc)
    {
        syslog(LOG_ERR, "Client need 1 argument - host pid");
        return false;
    }
    try
    {
        hostPid = std::atoi(argv[1]);
    }
    catch (...)
    {
        syslog(LOG_ERR, "Failed to convert string to pid");
        return false;
    }
    return true;
}

}

int main(int argc, char* argv[])
{
    openlog("lab2_client", LOG_PID | LOG_NDELAY | LOG_PERROR, LOG_USER);

    pid_t hostPid;
    if (!ParseArgs(argc, argv, hostPid))
    {
        syslog(LOG_ERR, "Failed to parse command line arguments");
        closelog();
        return false;
    }

    auto client = Client::GetRef();
    try
    {
        client.SetConnection(Connection::GetConnection(hostPid, Connection::Type::Client));
    }
    catch (std::exception& e)
    {
        syslog(LOG_ERR, "Failed to connect with host: %s", e.what());
        closelog();
        return false;
    }

    return 0;
}
