/// @file
/// @brief Определение класса копирователя
/// @author Воротников Андрей

#include "copier.h"

extern Copier Copier::instance;

void Copier::UpdateCopyInfo(const std::vector<CopyInfo>& copyInfoList)
{
}

Copier& Copier::GetRef()
{
    return instance;
}
