#include <iostream>
#include "system.hpp"

int main(int argc, char **argv) 
{
  SimSolve::System system(argv[1]);
  system.solve();
  std::cout << SimSolve::parameter_factory;
  return 0;
}
