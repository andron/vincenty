// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2; -*-

#include "coordinate_grid.h"

#include <cstdlib>
#include <unistd.h>
#include <sys/time.h>

#include <gtest/gtest.h>

using namespace vincenty;

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
  const vposition sweden_sw;
  const vposition sweden_ne;
  vposition sweden_center;

  CoordinateGridBasicTest()
      : la00lo00(0,0),
        la10lo10(to_rad(10),to_rad(10)),
        nullposition(),
        nullposition_init(0.0,0.0),
        northpole(to_rad( 90.0),0.0),
        southpole(to_rad(-90.0),0.0),
        sweden_sw(to_rad(58),to_rad(16)),
        sweden_ne(to_rad(58.5),to_rad(16.5))
  {
    vdirection d = inverse(sweden_sw,sweden_ne);
    sweden_center = direct(sweden_sw,d.bearing1,d.distance/2);
  }

  virtual ~CoordinateGridBasicTest() 
  {
    // Nothing to remove.
  }
};


TEST_F(CoordinateGridBasicTest, TwoPosInitializationCenterIsOk) {
  CoordinateGrid cg(sweden_sw,sweden_ne);
  EXPECT_EQ(cg.getCenter(),sweden_center) << "Center position is off";
}

TEST_F(CoordinateGridBasicTest, TwoPosInitializationCornersAreOk) {
  CoordinateGrid cg(sweden_sw,sweden_ne);
  EXPECT_EQ(sweden_sw,cg.getSW()) << "South west position is off";
  EXPECT_EQ(sweden_ne,cg.getNE()) << "North east position is off";
}

}
