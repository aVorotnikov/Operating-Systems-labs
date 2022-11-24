#include "daemon.h"

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    printf("Incorrect usage. Please provide only 1 argument with configuration file\n");
    return EXIT_FAILURE;
  }

  Daemon::GetInstance().Init(argv[1]);
  Daemon::GetInstance().Run();

  return EXIT_SUCCESS;
}
