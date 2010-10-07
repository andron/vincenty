
#include "coordinate_grid.h"

#include <cstdlib>
#include <iostream>
#include <string>

double get_geodata(const double& lat, const double& lon) __attribute__((pure));

int
main(int, char** av)
{
  std::cout << "Running: " << av[0] << std::endl;

  std::cout.precision(3);

  vincenty::vposition a(0.0,0.0); 
  vincenty::vposition b(0.0,M_PI);

  vincenty::vdirection d = vincenty::inverse(a,b);

  std::cout << vincenty::format::deg << d << std::endl;

  CoordinateGrid cg(a,d.distance,2e7);
  cg.splitUntil(5e4);

  std::string s;
  std::cin >> s;

  return 0;
}

double
get_geodata(const double&, const double&)
{
  return drand48();
}
