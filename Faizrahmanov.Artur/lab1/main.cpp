#include <iostream>
#include <fstream>
#include "FileMoverDaemon.h"

// Пользователь задаёт конфигурационный файл, состоящий из произвольного числа строк вида: folder1 folder2 ext.
// Копировать из folder1 в папку folder2 все файлы с расширением “ext”, предварительно очищая содержимое папки folder2

int main()
{
    FileMoverDaemon::getInstance()->initialize("./config.txt");
    FileMoverDaemon::getInstance()->run();

    // Config *instance = Config::getInstance();
    // instance->setConfigPath("./config.txt");
    // if (instance->readConfig())
    // {
    //     do
    //     {
    //         std::cout << instance->getFileExt() << std::endl;
    //     } while (instance->next());
    // }
    // std::cout << instance->getSleepDuration() << std::endl;

    return 0;
}