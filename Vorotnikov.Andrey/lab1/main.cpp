/// @file
/// @brief Главный файл программы
/// @author Воротников Андрей

#include "daemon/daemon.h"
#include "config/config.h"

#include <syslog.h>
#include <iostream>

int main(int argc, char* argv[])
{
    if (2 != argc)
    {
        std::cout << "Incorrect number of arguments. Need config file argument." << std::endl;
        return EXIT_FAILURE;
    }

    auto& daemon = Daemon::GetRef();
    Copier copier;
    daemon.SetParams(
        argv[1],
        std::bind(&Copier::Copy, &copier),
        [&copier, &daemon](const std::string& path)
        {
            std::vector<Copier::CopyInfo> copyInfoList;
            unsigned duration;
            if (!config::Read(path, copyInfoList, duration))
            {
                syslog(LOG_ERR, "Failed to read '%s' config file", path.c_str());
                return;
            }
            if (!copier.UpdateCopyInfo(copyInfoList))
                syslog(LOG_ERR, "Failed to update info from '%s' config file", path.c_str());
            else
                syslog(LOG_INFO, "Config loaded");
            daemon.UpdateDuration(std::chrono::seconds(duration));
        },
        [](){}
    );
    daemon.Init();
    daemon.Run();

    return EXIT_SUCCESS;
}
