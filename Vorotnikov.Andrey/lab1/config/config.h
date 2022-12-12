/// @file
/// @brief Работа с конфигурацией
/// @author Воротников Андрей

#include "../copier/copier.h"

/// @brief Пространство имён конфигурации
namespace config
{

/// @brief Прочитать конфигурационный файл
/// @param[in] path Путь к конфигурационному файлу
/// @param[out] copyInfoList Список структур об информации для копирования файлов
/// @param[out] duration Периодичность в секундах
/// @return true если прочитать удалось, false - иначе
bool Read(const std::string& path, std::vector<Copier::CopyInfo>& copyInfoList, unsigned& duration);

} // namespace config
