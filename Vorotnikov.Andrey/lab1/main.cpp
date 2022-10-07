/// @file
/// @brief Главный файл программы
/// @author Воротников Андрей

#include "copier/copier.h"

int main(int argc, char* argv[])
{
    if (2 != argc)
        return EXIT_FAILURE;

    std::vector<Copier::CopyInfo> copyInfoList;
    if (!Copier::ReadConfig(argv[1], copyInfoList))
        return EXIT_FAILURE;

    auto copier = Copier();
    if (!copier.UpdateCopyInfo(copyInfoList))
        return EXIT_FAILURE;

    copier.Copy();

    return EXIT_SUCCESS;
}
