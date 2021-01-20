#include "Application.h"

#include <iostream>

int main(int argc, const char* argv[])
{
  Application app;
  if(! app.parse(argc, argv)) {
    std::cerr << app.errorMessage() << std::endl;
    return 1;
  }

  if(app.usage()) {
    std::cout << app.usageMessage() << std::endl;
    return 0;
  }

  return app.run();
}
