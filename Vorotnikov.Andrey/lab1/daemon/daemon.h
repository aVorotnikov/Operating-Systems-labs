/// @file
/// @brief Объявление класса демона
/// @author Воротников Андрей

#include <functional>
#include <chrono>
#include <filesystem>

/// @brief Класс, осуществляющий копирование файлов
class Daemon
{
public:
    /// @brief Получить ссылку на экземпляр синглтона
    /// @return Ссылка на экземпляр
    static Daemon& GetRef();

    /// @brief Сохранить конфигурационный файл
    /// @param[in] path Путь к конфигурационному файлу

    /// @brief Сохранить конфигурационный файл
    /// @param[in] path Путь к конфигурационному файлу
    /// @param[in] onWork Функция обратного вызова на работу сервиса
    /// @param[in] onReloadConfig Функция обратного вызова на перезагрузка файла конфигурации
    /// @param[in] onTerminate Функция обратного вызова на выключение сервиса
    /// @param[in] duration Периодичность срабатывания демона
    /// @return true если удалось установить параметры, иначе - false
    bool SetParams(
        const std::string& path,
        const std::function<void ()>& onWork,
        const std::function<bool ()>& onReloadConfig,
        const std::function<void ()>& onTerminate,
        const std::chrono::seconds& duration = std::chrono::seconds(defaultDurationInSeconds)
    );

private:
    /// @brief Экземпляр синглтона
    static Daemon instance;

    /// @brief Конструктор
    Daemon();

    /// @brief Периодичность срабатывания демона по умолчанию
    constexpr static int defaultDurationInSeconds = 60;

    /// @brief Информация о директориях копирования
    std::filesystem::path configPath_;

    std::function<void ()> onWork_;
    std::function<bool ()> onReloadConfig_;
    std::function<void ()> onTerminate_;
    std::chrono::seconds duration_;

    /// @brief Флаг исполнения
    bool needWork_;
};
