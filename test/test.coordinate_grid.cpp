// -*- mode:c++; indent-tabs-mode:nil; -*-

#include "vincenty/coordinate_grid.h"

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

  vposition sw( 0.00,0.00);
  vposition nw( 0.30,0.10);
  vposition ne( 0.30,0.35);
  vposition se(-0.01,0.30);

  /*
    std::cout << format::dd;
    std::cout << sw << std::endl;
    std::cout << nw << std::endl;
    std::cout << ne << std::endl;
    std::cout << se << std::endl;
  */

  CoordinateGrid cg(sw,nw,ne,se);

  // Expect the "middle" point to be within boundries.

  ASSERT_GT(cg.getCenter().coords.a[0],cg.getSW().coords.a[0]);
  ASSERT_GT(cg.getCenter().coords.a[1],cg.getSW().coords.a[1]);

  ASSERT_LT(cg.getCenter().coords.a[0],cg.getNE().coords.a[0]);
  ASSERT_LT(cg.getCenter().coords.a[1],cg.getNE().coords.a[1]);

  ASSERT_LT(cg.getCenter().coords.a[0],cg.getNW().coords.a[0]);
  ASSERT_GT(cg.getCenter().coords.a[1],cg.getNW().coords.a[1]);

  ASSERT_GT(cg.getCenter().coords.a[0],cg.getSE().coords.a[0]);
  ASSERT_LT(cg.getCenter().coords.a[1],cg.getSE().coords.a[1]);
}

}
