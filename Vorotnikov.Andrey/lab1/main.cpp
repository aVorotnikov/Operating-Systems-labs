/// @file
/// @brief Главный файл программы
/// @author Воротников Андрей

#include "daemon/daemon.h"
#include "copier/copier.h"

int main(int argc, char* argv[])
{
    if (2 != argc)
        return EXIT_FAILURE;

    auto& daemon = Daemon::GetRef();
    Copier copier;
    daemon.SetParams(
        argv[1],
        std::bind(&Copier::Copy, &copier),
        [&copier](const std::string& path)
        {
            std::vector<Copier::CopyInfo> copyInfoList;
            if (!Copier::ReadConfig(path, copyInfoList))
                return;
            copier.UpdateCopyInfo(copyInfoList);
        },
        [](){}
    );
    daemon.Init();
    daemon.Run();

    return EXIT_SUCCESS;
}
