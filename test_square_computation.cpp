// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2; -*-

#include "vincenty.h"

#include <cstdlib>
#include <unistd.h>
#include <sys/time.h>

#include <sstream>
#include <iomanip>

#include <gtest/gtest.h>

using namespace vincenty;

namespace Test {

typedef std::vector<vposition> vposvec;
typedef std::vector<vdirection> vdirvec;

/**
 * Testing class for testing "square" computations. I.e. how does one compute
 * a "perfect" realworld square which have equal real world distances.
 */
class SquareTest : public testing::Test
{
 protected:
  const vposition p_center;

  SquareTest()
      : p_center(to_rad(58.41000),to_rad(15.66667))
  {
  }

  virtual ~SquareTest() 
  {
    // Nothing to remove.
  }
};

TEST_F(SquareTest, SquareCorners)
{
  const double distance = 500000.0;
  const double sqrt2    = sqrt(2.0);

  std::cout << format::dd;
  std::cout.precision(10);

  // Compute corners ...
  vposition p_nw = direct(p_center,vdirection(direction::nw,distance*sqrt2));
  vposition p_ne = direct(p_center,vdirection(direction::ne,distance*sqrt2));
  vposition p_sw = direct(p_center,vdirection(direction::sw,distance*sqrt2));
  vposition p_se = direct(p_center,vdirection(direction::se,distance*sqrt2));

  // Compute mid points from the corners ...
  vposition p_e = p_se ^ p_ne;
  vposition p_n = p_ne ^ p_nw;
  vposition p_s = p_sw ^ p_se;
  vposition p_w = p_sw ^ p_nw;

  // Now compute the mid points from the center ...
  vposition p_e2 = direct(p_center,vdirection(direction::e,distance));
  vposition p_n2 = direct(p_center,vdirection(direction::n,distance));
  vposition p_s2 = direct(p_center,vdirection(direction::s,distance));
  vposition p_w2 = direct(p_center,vdirection(direction::w,distance));
  
  // Display differences between the two methods.
  std::cout << get_distance(p_e,p_e2) << std::endl;
  std::cout << get_distance(p_n,p_n2) << std::endl;
  std::cout << get_distance(p_s,p_s2) << std::endl;
  std::cout << get_distance(p_w,p_w2) << std::endl;

  std::cout << get_distance(p_e,p_center) << std::endl;
  std::cout << get_distance(p_n,p_center) << std::endl;
  std::cout << get_distance(p_s,p_center) << std::endl;
  std::cout << get_distance(p_w,p_center) << std::endl;

  std::cout << get_distance(p_e2,p_center) << std::endl;
  std::cout << get_distance(p_n2,p_center) << std::endl;
  std::cout << get_distance(p_s2,p_center) << std::endl;
  std::cout << get_distance(p_w2,p_center) << std::endl;
}

} // namespace end


int
main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
