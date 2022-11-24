#include "daemon/daemon.h"

#include <iostream>

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Wrong number of arguments. Provide path to config file as single argument.\n";
    return EXIT_FAILURE;
  }

  auto& daemon = Daemon::getInstance();
  daemon.Init(argv[1]);
  daemon.Run();
  return EXIT_SUCCESS;
}