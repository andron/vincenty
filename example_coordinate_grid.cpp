
#include "coordinate_grid.h"

#include <cstdlib>
#include <iostream>

double get_geodata(const double& lat, const double& lon) __attribute__((pure));

int
main(int, char** av)
{
  std::cout << "Running: " << av[0] << std::endl;

  vincenty::vposition sw(0.0,0.0); 
  vincenty::vposition ne = vincenty::direct(sw,vincenty::direction::northeast,1000);
  
  std::cout << sw << std::endl;
  std::cout << ne << std::endl;

  CoordinateGrid cg(sw,ne);
  cg.setVirtualGridSize(1000);
  cg.splitUntil(1e4);

  std::cout
      << std::endl << cg << std::endl << std::endl
      << cg(0,0) << std::endl
      << cg(0,999) << std::endl
      << cg(999,0) << std::endl
      << cg(999,999) << std::endl;
  
  return 0;
}

double
get_geodata(const double&, const double&)
{
  return drand48();
}
