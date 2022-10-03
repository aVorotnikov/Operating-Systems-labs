/// @file
/// @brief Главный файл программы
/// @author Воротников Андрей

#include "copier/copier.h"

int main(void)
{
    auto& copier = Copier::GetRef();
    copier.UpdateCopyInfo({});
}
