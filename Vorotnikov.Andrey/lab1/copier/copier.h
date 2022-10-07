/// @file
/// @brief Объявление класса копирователя
/// @author Воротников Андрей

#include <string>
#include <map>
#include <vector>
#include <set>
#include <mutex>

/// @brief Класс, осуществляющий копирование файлов
class Copier
{
public:
    /// @brief Получить ссылку на экземпляр синглтона
    /// @return Ссылка на экземпляр
    static Copier& GetRef();

    /// @brief Структура конфигурации
    struct CopyInfo
    {
        std::string src;        ///< Директория, откуда копируются файлы
        std::string dst;        ///< Директория, в которую будут копироваться файлы
        std::string extension;  ///< Расширение, которое будет копироваться в поддиректорию subfolder
        std::string subfolder;  ///< Имя поддиректории директории dst, в которую будут копироваться файлы с расширением extension
    };

    /// @brief Прочитать конфигурационный файл
    /// @param[in] path Путь к конфигурационному файлу
    /// @param[out] copyInfoList Список структур об информации для копирования файлов
    /// @return true если прочитать удалось, false - иначе
    static bool ReadConfig(const std::string& path, std::vector<CopyInfo>& copyInfoList);

    /// @brief Конструктор
    /// @param[in] copyInfoList Список структур об информации для копирования файлов
    /// @return true если удалось обновить данные о копировании, false - иначе
    bool UpdateCopyInfo(const std::vector<CopyInfo>& copyInfoList);

    /// @brief Осуществить копирование файлов
    /// @return true если удалось осуществить операцию копирования, false - иначе
    bool Copy();

private:
    /// @brief Экземпляр синглтона
    static Copier instance;

    /// @brief Конструктор
    Copier() = default;

    /// @brief Название поддиректории для файлов, расширения которых не указаны для копирования
    constexpr static char othersSubdirectory[] = "OTHERS";

    /// @brief Структура для сохранения пар расширение-поддиректория
    using ExtSubfolders = std::map<std::string, std::set<std::string>>;
    /// @brief Структура хранения информации о директории для копирования файлов
    using DstDirectory = std::map<std::string, ExtSubfolders>;

    /// @brief Информация о директориях копирования
    std::map<std::string, DstDirectory> dstDirectories_;

    /// @brief Мьютекс для информации о директориях копирования
    std::mutex mtx_;
};
