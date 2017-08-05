#include <iostream>
#include "system.hpp"

int main(int argc, char **argv) 
{
  SimSolve::System system(argv[1]);
  system.solve();
  return 0;
}
