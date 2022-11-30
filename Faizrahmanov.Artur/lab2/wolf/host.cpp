#include "wolf.h"
#include "gui/gui.h"

#include <iostream>
#include <sys/syslog.h>

int main()
{
    if (Wolf::getInstance().init(Gui::getMessageFromHost))
    {
        Gui::getInstance().init(Wolf::isRunning, Wolf::stopRunning, Wolf::getNewWolfMessage);
        std::thread wolfThread(&Wolf::run, &Wolf::getInstance());
        Gui::getInstance().run();
        wolfThread.join();
    }
    else
    {
        std::cout << "ERROR: can't initialize wolf" << std::endl;
        syslog(LOG_ERR, "ERROR: failed to initialize host");
    }

    syslog(LOG_INFO, "INFO: game over");

    return 0;
}