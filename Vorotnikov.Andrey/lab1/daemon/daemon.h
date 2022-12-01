/// @file
/// @brief Объявление класса демона
/// @author Воротников Андрей

#include <functional>
#include <chrono>
#include <filesystem>
#include <condition_variable>

/// @brief Класс, осуществляющий копирование файлов
class Daemon
{
public:
    /// @brief friend-функция обработки сигналов
    /// @param[in] sig Тип сигнала
    friend void SignalHandler(const int sig);

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
        const std::function<bool ()>& onWork,
        const std::function<void (const std::string&)>& onReloadConfig,
        const std::function<void ()>& onTerminate,
        const std::chrono::seconds& duration = std::chrono::seconds(defaultDurationInSeconds)
    );

    /// @brief Обновить периодичность
    /// @param duration Периодичность
    void UpdateDuration(const std::chrono::seconds& duration = std::chrono::seconds(defaultDurationInSeconds));

    /// @brief Инициализировать демона
    /// @return true если удалось инициализировать демон, иначе - false
    void Init();

    /// @brief Начать работу демона
    /// @return true если удалось запустить демон, иначе - false
    bool Run();

    /// @brief Периодичность срабатывания демона по умолчанию
    constexpr static int defaultDurationInSeconds = 60;

private:
    /// @brief Экземпляр синглтона
    static Daemon instance;

    /// @brief Конструктор
    Daemon();

    /// @brief Путь к PID-файлу
    constexpr static char pidFilePath[] = "/var/run/copying_daemon.pid";

    /// @brief Информация о директориях копирования
    std::filesystem::path configPath_;

    /// @brief Функция обратного вызова на работу сервиса
    std::function<bool ()> onWork_;
    /// @brief Функция обратного вызова на перезагрузка файла конфигурации
    std::function<void (const std::string&)> onReloadConfig_;
    /// @brief Функция обратного вызова на выключение сервиса
    std::function<void ()> onTerminate_;
    /// @brief Периодичность срабатывания демона
    std::chrono::seconds duration_;

    /// @brief Флаг было ли обноевление параметров
    bool updated_;
    /// @brief Флаг исполнения
    bool needWork_;
    /// @brief Условная переменная исполнения
    std::condition_variable work_;
    /// @brief Мьютекс
    std::mutex workMtx_;
};
