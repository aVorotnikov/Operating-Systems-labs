#include <iostream>
#include "daemon.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "Wrong arguments count!";
        return EXIT_FAILURE;
    }

    Daemon::GetInstance().Init(argv[1]);
    Daemon::GetInstance().Run();

    return EXIT_SUCCESS;
}