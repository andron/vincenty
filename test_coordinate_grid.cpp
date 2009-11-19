// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2; -*-

#include "coordinate_grid.h"

#include <cstdlib>
#include <unistd.h>
#include <sys/time.h>

#include <gtest/gtest.h>

using namespace vincenty;
using namespace coordinate;

namespace Test {

/**
 * Testing class for basic tests, non-verifiable tests, i.e. tests which only
 * tests "expected" behaviour and sane values like non-negative distances and
 * that jumping back and forth between positions results in the same distance
 * and that bearings are reversed etc.
 */
class CoordinateGridBasicTest : public testing::Test
{
 protected:
  const vposition la00lo00;
  const vposition la10lo10;
  const vposition nullposition;
  const vposition nullposition_init;
  const vposition northpole;
  const vposition southpole;
  const vposition sw;
  const vposition ne;
  vposition center;

  CoordinateGridBasicTest()
      : la00lo00(0,0),
        la10lo10(to_rad(10),to_rad(10)),
        nullposition(),
        nullposition_init(0.0,0.0),
        northpole(to_rad( 90.0),0.0),
        southpole(to_rad(-90.0),0.0),
        sw(to_rad(55),to_rad(16)),
        ne(to_rad(59.5),to_rad(16.5)),
        center(0.0,0.0)
  {
    vdirection d = inverse(sw,ne);
    center = direct(sw,d.bearing1,d.distance/2.0);
  }

  virtual ~CoordinateGridBasicTest() 
  {
    // Nothing to remove.
  }
};


TEST_F(CoordinateGridBasicTest, TwoPosInitializationCenterOk) {
  CoordinateGrid cg(sw,ne);
  EXPECT_EQ(cg.getCenter(),center) << "Center position is off";
}

TEST_F(CoordinateGridBasicTest, TwoPosInitializationCornersOk) {
  CoordinateGrid cg(sw,ne);
  EXPECT_EQ(sw,cg.getSW()) << "South west position is off";
  EXPECT_EQ(ne,cg.getNE()) << "North east position is off";
}

TEST_F(CoordinateGridBasicTest, FourPosInitializationCornersOk) {

  vposition sw(-1.00,-1.00);
  vposition nw( 1.00,-1.00);
  vposition ne( 1.00, 1.00);
  vposition se(-1.00, 1.01);

  std::cout << sw << std::endl;
  std::cout << nw << std::endl;
  std::cout << ne << std::endl;
  std::cout << se << std::endl;

  CoordinateGrid cg(sw,nw,ne,se);

  std::cout << cg << std::endl;
  
}

}
