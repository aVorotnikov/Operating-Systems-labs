/// @file
/// @brief Главный файл программы
/// @author Воротников Андрей

#include "copier/copier.h"

int main(void)
{
    std::vector<Copier::CopyInfo> copyInfoList;
    if (!Copier::ReadConfig("data/test.conf", copyInfoList))
    {
        return EXIT_FAILURE;
    }

    auto& copier = Copier::GetRef();
    copier.UpdateCopyInfo({});
    return EXIT_SUCCESS;
}
