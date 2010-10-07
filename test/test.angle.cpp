// -*- mode:c++; indent-tabs-mode:nil; -*-

#include "vincenty/vincenty.h"

#include <iostream>
#include <iomanip>

using namespace vincenty;

int
main(int, char**)
{
  vposition linkping( to_rad(58.415755),to_rad(15.625419));
  vposition uddevalla(to_rad(58.355630),to_rad(11.938019));
  vposition stockholm(to_rad(59.335991),to_rad(18.064270));
  vposition karlstad( to_rad(59.381901),to_rad(13.504128));

  std::cout.precision(4);
  std::cout.setf(std::ios::fixed,std::ios::floatfield);

  std::cout << format::dd << format::deg;
  std::cout << linkping  << std::endl;
  std::cout << uddevalla << std::endl;
  std::cout << stockholm << std::endl;
  std::cout << karlstad  << std::endl << std::endl;

  std::cout << "us" << inverse(uddevalla,stockholm) << "  70.08 255.34" << std::endl;
  std::cout << "uk" << inverse(uddevalla,karlstad)  << "  37.42 218.34" << std::endl;
  std::cout << "ul" << inverse(uddevalla,linkping)  << "  86.35 269.57" << std::endl << std::endl;

  std::cout << "ku" << inverse(karlstad,uddevalla)  << " 219.00  37.49" << std::endl;
  std::cout << "ks" << inverse(karlstad,stockholm)  << "  89.15 273.08" << std::endl;
  std::cout << "kl" << inverse(karlstad,linkping)   << " 130.41 312.17" << std::endl;

  std::cout << "lu" << inverse(linkping,uddevalla)  << " 269.69  86.42" << std::endl;
  std::cout << "su" << inverse(stockholm,uddevalla) << " 255.35  70.08" << std::endl;
  std::cout << "sl" << inverse(stockholm,linkping)  << " 235.06  52.89" << std::endl;

  return 0;
}
