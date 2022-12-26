#include <sys/syslog.h>

#include "host.h"

int main(int argc, char *argv[]) {
    openlog("Chat log", LOG_NDELAY | LOG_PID, LOG_USER);

    try {
        Host::getInstance().run();
    } catch (std::exception &e) {
        syslog(LOG_ERR, "ERROR: %s. Close chat...", e.what());
    }

    closelog();
    return 0;
}