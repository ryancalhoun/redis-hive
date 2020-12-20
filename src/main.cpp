#include "Application.h"

int main(int argc, const char* argv[])
{
  Application app;
  if(! app.parse(argc, argv)) {
    return 1;
  }
  return app.run();
}
