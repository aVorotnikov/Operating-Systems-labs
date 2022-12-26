#include "../lib/host.h"

#include <sys/syslog.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    openlog("lab2_host", LOG_PID | LOG_NDELAY | LOG_PERROR, LOG_USER);

    auto& host = Host::GetRef();
    try
    {
        host.SetConnection(Connection::GetConnection(getpid(), Connection::Type::Host));
    }
    catch (std::exception& e)
    {
        syslog(LOG_ERR, "Failed to create connection: %s", e.what());
        closelog();
        return EXIT_FAILURE;
    }

    bool result = host.Run();
    closelog();
    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
