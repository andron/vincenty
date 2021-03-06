// -*- mode:c++; indent-tabs-mode:nil; -*-

#include "vincenty/vincenty.h"

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
 * Testing class for basic tests, non-verifiable tests, i.e. tests which only
 * tests "expected" behaviour and sane values like non-negative distances and
 * that jumping back and forth between positions results in the same distance
 * and that bearings are reversed etc.
 */
class VincentyBasicTest : public testing::Test
{
 protected:
  const vposition la00lo00;
  const vposition la10lo10;
  const vposition nullposition;
  const vposition nullposition_init;
  const vposition northpole;
  const vposition southpole;
  const vposition sweden;
  const vposition p1;
  const vposition p2;
  const vdirection dir025_15000;
  const vdirection dir030_90000;
  const vdirection dir240_60000;

  VincentyBasicTest()
      : la00lo00(0,0),
        la10lo10(to_rad(10),to_rad(10)),
        nullposition(),
        nullposition_init(0.0,0.0),
        northpole(to_rad( 90.0),0.0),
        southpole(to_rad(-90.0),0.0),
        sweden(to_rad(58),to_rad(16)),
        p1(to_rad(50.111),to_rad(11.111)),
        p2(to_rad(60.111),to_rad(18.111)),
        dir025_15000(to_rad(25), 15000),
        dir030_90000(to_rad(30), 90000),
        dir240_60000(to_rad(240),60000)
  {
  }

  virtual ~VincentyBasicTest()
  {
    // Nothing to remove.
  }
};


// Test converts from and to degrees and radians.
TEST_F(VincentyBasicTest, ConvertersDegreesRadians) {
  EXPECT_FLOAT_EQ( 3.14159265358979L, to_rad(180.0));
  EXPECT_FLOAT_EQ( 1.57079632679490L, to_rad( 90.0));
  EXPECT_FLOAT_EQ( 1.36135681655558L, to_rad( 78.0));
  EXPECT_FLOAT_EQ(-1.36135681655558L, to_rad(-78.0));
  EXPECT_FLOAT_EQ( 0.0, to_rad( 0.0));
  EXPECT_FLOAT_EQ(-0.0, to_rad(-0.0));

  EXPECT_FLOAT_EQ(180.0, to_deg(3.14159265358979L));
  EXPECT_FLOAT_EQ( 90.0, to_deg(1.57079632679490L));
  EXPECT_FLOAT_EQ( 78.0, to_deg(1.36135681655558L));
  EXPECT_FLOAT_EQ(-56.0, to_deg( to_rad(-56.0) ));
  EXPECT_FLOAT_EQ(  0.0, to_deg( 0.0));
  EXPECT_FLOAT_EQ( -0.0, to_deg(-0.0));
}


TEST_F(VincentyBasicTest, ConvertersDegreesRadiansReciproc)
{
  for (int i=0; i<10; ++i) {
    double t1 = i+1;
    double t2 = i+5;
    double t3 = i*5;
    double t4 = i*10;
    double t5 = i*30;
    EXPECT_FLOAT_EQ(t1, to_deg(to_rad(t1)));
    EXPECT_FLOAT_EQ(t2, to_deg(to_rad(t2)));
    EXPECT_FLOAT_EQ(t3, to_deg(to_rad(t3)));
    EXPECT_FLOAT_EQ(t4, to_deg(to_rad(t4)));
    EXPECT_FLOAT_EQ(t5, to_deg(to_rad(t5)));
  }
}


TEST_F(VincentyBasicTest, VpositionDegreeRadianConverterTenths) {
  for (int i=0; i<5; ++i) {
    const double val = 10*i;
    int deg = vposition::deg(to_rad(val));
    int min = vposition::min(to_rad(val));
    int sec = vposition::sec(to_rad(val));
    EXPECT_EQ(val, deg)
        << "Value: " << val << " Index: " << i;
    EXPECT_EQ(0,   min)
        << "Value: " << val << " Index: " << i;
    EXPECT_EQ(0,   sec)
        << "Value: " << val << " Index: " << i;
  }
}


TEST_F(VincentyBasicTest, VpositionDegreeRadianConverterHalfs) {
  for (int i=0; i<5; ++i) {
    const double val = 10*i + 0.5;
    int deg = vposition::deg(to_rad(val));
    int min = vposition::min(to_rad(val));
    int sec = vposition::sec(to_rad(val));
    EXPECT_EQ(int(val),deg)
        << "Value: " << val << " Index: " << i;
    EXPECT_EQ(30      ,min)
        << "Value: " << val << " Index: " << i;
    EXPECT_EQ(0       ,sec)
        << "Value: " << val << " Index: " << i;
  }
}


TEST_F(VincentyBasicTest, VpositionDegreeRadianConverterTenthsPlusOne) {
  for (int i=0; i<5; ++i) {
    const double val = 10*i + 1;
    int deg = vposition::deg(to_rad(val));
    int min = vposition::min(to_rad(val));
    int sec = vposition::sec(to_rad(val));
    EXPECT_EQ(int(val),deg)
        << "Value: " << val << " Index: " << i;
    EXPECT_EQ(0       ,min)
        << "Value: " << val << " Index: " << i;
    EXPECT_EQ(0       ,sec)
        << "Value: " << val << " Index: " << i;
  }
}


// The default constructor shall initialize with 0 (zero), and initializing
// with zero shall set values to zero.
TEST_F(VincentyBasicTest, DefaultConstructorsInitializeZero) {
  EXPECT_FLOAT_EQ(0.0, nullposition.coords.a[0]);
  EXPECT_FLOAT_EQ(0.0, nullposition.coords.a[1]);
  EXPECT_FLOAT_EQ(0.0, nullposition_init.coords.a[0]);
  EXPECT_FLOAT_EQ(0.0, nullposition_init.coords.a[1]);
}


// Test positions of the poles.
TEST_F(VincentyBasicTest, InitializingPolesIsOk) {
  EXPECT_FLOAT_EQ( to_rad( 90.0), northpole.coords.a[0]);
  EXPECT_FLOAT_EQ( to_rad(  0.0), northpole.coords.a[1]);
  EXPECT_FLOAT_EQ( to_rad(-90.0), southpole.coords.a[0]);
  EXPECT_FLOAT_EQ( to_rad(  0.0), southpole.coords.a[1]);
}


// Default ctor
TEST_F(VincentyBasicTest, VdirectionDefaultConstructor) {
  const vdirection d;
  EXPECT_FLOAT_EQ(0.0, d.bearing1)
      << "Default vdirection ctor bearing1 shall be 0.0!";
  EXPECT_FLOAT_EQ(0.0, d.bearing2)
      << "Default vdirection ctor bearing2 shall be 0.0!";
  EXPECT_FLOAT_EQ(0.0, d.distance)
      << "Default vdirection ctor distance shall be 0.0!";
}


// Default ctor
TEST_F(VincentyBasicTest, VpositionDefaultConstructor) {
  const vposition p;
  EXPECT_FLOAT_EQ(0.0, p.coords.a[0])
      << "Default vposition ctor coord shall be 0.0!";
  EXPECT_FLOAT_EQ(0.0, p.coords.a[1])
      << "Default vposition ctor coord shall be 0.0!";
}


// Test movement of zero meters for direct().
TEST_F(VincentyBasicTest, ZeroDistance) {
  // Travel zero distance.
  const vposition a = direct(p1,  0.0, 0.0);
  const vposition b = direct(p1, 30.0, 0.0);
  const vposition c = direct(p1, 90.0, 0.0);

  // Expect the position to be exactly the same.
  EXPECT_FLOAT_EQ(p1.coords.a[0], a.coords.a[0]) << "Traveling 0.0m must not change position!";
  EXPECT_FLOAT_EQ(p1.coords.a[1], a.coords.a[1]) << "Traveling 0.0m must not change position!";
  EXPECT_FLOAT_EQ(p1.coords.a[0], b.coords.a[0]) << "Traveling 0.0m must not change position!";
  EXPECT_FLOAT_EQ(p1.coords.a[1], b.coords.a[1]) << "Traveling 0.0m must not change position!";
  EXPECT_FLOAT_EQ(p1.coords.a[0], c.coords.a[0]) << "Traveling 0.0m must not change position!";
  EXPECT_FLOAT_EQ(p1.coords.a[1], c.coords.a[1]) << "Traveling 0.0m must not change position!";
}


// Compare identical positions (testing operator==)
TEST_F(VincentyBasicTest, IdenticalPositionsCompareOperator) {
  EXPECT_TRUE(northpole == northpole)
      << "Compare operator must return true for identical positions!";

  EXPECT_TRUE(p1 == p1)
      << "Compare operator must return true for identical positions!";

  EXPECT_FALSE(northpole == southpole)
      << "Compare operator must NOT return true for different positions!";

  EXPECT_FALSE(p1 == p2)
      << "Compare operator must NOT return true for different positions!";
}


// Compare identical positions (testing operator==)
TEST_F(VincentyBasicTest, IdenticalDirectionsCompareOperator) {

  const vdirection d1 = inverse(northpole, southpole);
  const vdirection d2 = inverse(southpole, northpole);
  const vdirection d3 = inverse(sweden, p1);

  const std::string error_true = "Compare operator must return true for identical directions!";
  const std::string error_false = "Compare operator must NOT return true for different directions!";

  EXPECT_TRUE(d1 == d1) << error_true;
  EXPECT_TRUE(d2 == d2) << error_true;
  EXPECT_TRUE(d3 == d3) << error_true;
  EXPECT_TRUE(p1 == p1) << error_true;
  EXPECT_TRUE(dir025_15000 == dir025_15000) << error_true;
  EXPECT_TRUE(dir030_90000 == dir030_90000) << error_true;

  EXPECT_FALSE(d1 == d2) << error_false;
  EXPECT_FALSE(d2 == d3) << error_false;
  EXPECT_FALSE(d3 == d1) << error_false;
  EXPECT_FALSE(p1 == p2) << error_false;
  EXPECT_FALSE(dir025_15000 == dir030_90000) << error_false;

  const vdirection x1(0.0,10,0.0);
  const vdirection x2(1.0,10,1.0);
  EXPECT_FALSE(x1 == x2) << error_false;
}


TEST_F(VincentyBasicTest, ShortenOperator) {
  const vdirection d1(10,10);
  const vdirection d2 = d1/2;
  EXPECT_FLOAT_EQ(d1.distance, d2.distance*2.0)
      << "Divide operator should change the distance!";
  EXPECT_FLOAT_EQ(d1.bearing1, d2.bearing1)
      << "Divide operator must NOT change bearing!";
}


TEST_F(VincentyBasicTest, LengthenOperator) {
  const vdirection d1(10,10);
  const vdirection d2 = d1*2;
  EXPECT_FLOAT_EQ(d1.distance, d2.distance/2.0)
      << "Multiply operator should change the distance!";
  EXPECT_FLOAT_EQ(d1.bearing1, d2.bearing1)
      << "Multiply operator must NOT change bearing!";
}


TEST_F(VincentyBasicTest, NonNegativeDistances) {
  const vdirection d1 = inverse(la00lo00,northpole);
  const vdirection d2 = inverse(la10lo10,northpole);
  const vdirection d3 = inverse(la10lo10,la00lo00);
  const vdirection d4 = inverse(sweden,northpole);
  const vdirection d5 = inverse(sweden,southpole);
  EXPECT_GT( d1.distance, 0.0 ) << "Distance cannot be negative!";
  EXPECT_GT( d2.distance, 0.0 ) << "Distance cannot be negative!";
  EXPECT_GT( d3.distance, 0.0 ) << "Distance cannot be negative!";
  EXPECT_GT( d4.distance, 0.0 ) << "Distance cannot be negative!";
  EXPECT_GT( d5.distance, 0.0 ) << "Distance cannot be negative!";
}

// Two non-equal positions shall result in a non-negative distance.
TEST_F(VincentyBasicTest, NonNegativeDistancesRandomized) {
  srand48(123456789);
  const unsigned int loops = 50;
  for ( unsigned int i=0; i<loops; ++i ) {
    const double lat1 = 2*M_PI * ( drand48() - 0.5 );
    const double lon1 =   M_PI * ( drand48() - 0.5 );
    const double lat2 = 2*M_PI * ( drand48() - 0.5 );
    const double lon2 =   M_PI * ( drand48() - 0.5 );
    vdirection dir = inverse(lat1,lon1,lat2,lon2);
    EXPECT_GT( dir.distance, 0.0 )
        << "Two non-equal positions resulted in a negative distance!";
  }
}


TEST_F(VincentyBasicTest, CompareOperatorsPosition) {
  srand48(123456789);
  const unsigned int loops = 50;
  for ( unsigned int i=0; i<loops; ++i ) {
    const double lat = 2*M_PI * ( drand48() - 0.5 );
    const double lon =   M_PI * ( drand48() - 0.5 );
    vposition p1(lat,lon);
    vposition p2(lat,lon);
    ASSERT_TRUE(p1 == p2);
    EXPECT_FLOAT_EQ(p1.coords.a[0],p2.coords.a[0]);
    EXPECT_FLOAT_EQ(p1.coords.a[1],p2.coords.a[1]);
  }
}


TEST_F(VincentyBasicTest, CompareOperatorsDirection) {
  srand48(123456789);
  const unsigned int loops = 50;
  for ( unsigned int i=0; i<loops; ++i ) {
    const double lat1 = 2*M_PI * ( drand48() - 0.5 );
    const double lon1 =   M_PI * ( drand48() - 0.5 );
    const double lat2 = 2*M_PI * ( drand48() - 0.5 );
    const double lon2 =   M_PI * ( drand48() - 0.5 );
    vdirection d1 = inverse(lat1,lon1,lat2,lon2);
    vdirection d2 = inverse(lat1,lon1,lat2,lon2);
    ASSERT_TRUE(d1 == d2);
    EXPECT_FLOAT_EQ(d1.bearing1,d2.bearing1);
    EXPECT_FLOAT_EQ(d1.bearing2,d2.bearing2);
    EXPECT_FLOAT_EQ(d1.distance,d2.distance);
  }
}


// Null distances.
TEST_F(VincentyBasicTest, ZeroDistances) {
  const vposition p1(0,0);
  const vposition p2(0,0);

  const vdirection d11 = inverse(p1,p1);
  const vdirection d12 = inverse(p1,p2);
  const vdirection d21 = inverse(p2,p1);
  const vdirection d22 = inverse(p2,p2);

  EXPECT_FLOAT_EQ(0.0,d11.distance);
  EXPECT_FLOAT_EQ(0.0,d12.distance);
  EXPECT_FLOAT_EQ(0.0,d21.distance);
  EXPECT_FLOAT_EQ(0.0,d22.distance);
}


// Null distances, randomized
TEST_F(VincentyBasicTest, ZeroDistancesRandomized) {
  srand48(123456789);
  const unsigned int loops = 50;
  for ( unsigned int i=0; i<loops; ++i ) {
    const double lat1 = 2*M_PI * ( drand48() - 0.5 );
    const double lon1 =   M_PI * ( drand48() - 0.5 );
    vdirection dir = inverse(lat1,lon1,lat1,lon1);
    EXPECT_FLOAT_EQ( 0.0, dir.distance )
        << "Distance to self must be zero" << dir;
  }
}


// Going a distance in two oppisite directions shall result in a distance twice
// the size between the two resulting positions.
TEST_F(VincentyBasicTest, OppositeDirectionNEWSResultsInDoubleDistance) {
  srand48(123456789);
  const unsigned int loops = 50;
  const unsigned int dist = 5000;
  for ( unsigned int p=1; p<loops; ++p ) {
    const double lat1 = 2*M_PI * ( drand48() - 0.5 );
    const double lon1 =   M_PI * ( drand48() - 0.5 );
    vposition pos(lat1,lon1);
    for ( unsigned int d=1; d<9; ++d ) {
      for ( unsigned int i=1; i<9; ++i ) {
        // North, south, east and west.
        vposition pN, pS, pE, pW;

        pN = direct(pos,direction::n,dist*d);
        pS = direct(pos,direction::s,dist*d);
        pE = direct(pos,direction::e,dist*d);
        pW = direct(pos,direction::w,dist*d);

        vdirection dNS = inverse(pN,pS);
        vdirection dEW = inverse(pE,pW);

        EXPECT_FLOAT_EQ(2*dist*d,dNS.distance)
            << "Opposite direction did not result in double distance!";
        EXPECT_FLOAT_EQ(2*dist*d,dEW.distance)
            << "Opposite direction did not result in double distance!";
      }
    }
  }
}


TEST_F(VincentyBasicTest, AritmeticOperatorsAreOk) {
  vposition pa = p1 + dir240_60000;
  vposition pb = p2 + dir030_90000;

  vposition px = p1 ^ p2;
  vposition py = pa ^ pb;

  vdirection d1 = px - p1;
  vdirection d2 = py - px;
  vdirection d3 = pb - py;
  vdirection d4 = p2 - p1;
  vdirection d5 = pb - p2;
  vdirection d6 = pa - pb;
  vdirection d7 = p1 - pa;

  // Hmm, wonder if this really fits the criteria of a "sane" test. If the
  // ostream output is equal we assume the positions are equal, or at least
  // equal enough.
  std::stringstream str1,str2,str3,str4;

  str1.precision(5);
  str2.precision(str1.precision());
  str3.precision(str1.precision());
  str4.precision(str1.precision());

  str1 << p1+d4+d5;
  str2 << p1+d1+d2+d3;
  EXPECT_EQ(str1.str(),str2.str())
      << "Traveling along two paths should have resulted in same position!";

  str3 << p1;
  str4 << pb+d6+d7;
  EXPECT_EQ(str3.str(),str4.str())
      << "Traveling along two paths should have resulted in same position!";
}

// ---------------------------------------------------------------------------

/**
 * Testing class for more specific tests, real distances and verification to
 * correct reference points etc.
 */
class VincentyVerificationTest : public testing::Test
{
 protected:

  // Fixed positions and directions, and distances and bearings back and forth
  // from these.

  vposition northpole1;
  vposition northpole2;
  vposition southpole1;
  vposition southpole2;
  vposition magneticpole;

  vposition gmtequator;
  vposition linkping;
  vposition uddevalla;
  vposition stockholm;
  vposition karlstad;
  vposition alcatraz;
  vposition lady_liberty_statue;
  vposition saab_runway_north;
  vposition saab_runway_south;

  std::vector<vposition> positions;

  VincentyVerificationTest()
      : northpole1( M_PI/2,0),
        northpole2( M_PI/2,M_PI/2),
        southpole1(-M_PI/2,0),
        southpole2(-M_PI/2,M_PI/2),
        magneticpole(to_rad(82.7),to_rad(-114.4)),
        gmtequator(0,0),

        // From maps.google.com
        linkping( to_rad(58.415755),to_rad(15.625419)),
        uddevalla(to_rad(58.355630),to_rad(11.938019)),
        stockholm(to_rad(59.335991),to_rad(18.064270)),
        karlstad( to_rad(59.381901),to_rad(13.504128)),
        alcatraz( to_rad(37.826663),to_rad(-122.423015)),
        lady_liberty_statue(to_rad(40.689526),to_rad(-74.044837)),
        saab_runway_north(to_rad(58.409693),to_rad(15.66272)),
        saab_runway_south(to_rad(58.402195),to_rad(15.69706)),
        positions()
  {
    positions.push_back(northpole1);
    positions.push_back(southpole1);
    positions.push_back(magneticpole);
    positions.push_back(gmtequator);
    positions.push_back(linkping);
    positions.push_back(uddevalla);
    positions.push_back(stockholm);
    positions.push_back(alcatraz);
    positions.push_back(lady_liberty_statue);
    positions.push_back(saab_runway_north);
    positions.push_back(saab_runway_south);
  }

  virtual void SetUp() {
    // Setup things, which might throw.
  }

  virtual void TearDown() {
    // Remove things setup by SetUp.
  }
};


TEST_F(VincentyVerificationTest, AccuracyChangesAreOk) {
  std::vector<double> dists;
  dists.push_back(1e1);
  dists.push_back(1e2);
  dists.push_back(1e3);
  dists.push_back(1e4);
  dists.push_back(1e5);
  dists.push_back(1e6);
  dists.push_back(1e7);

  std::vector<double>::const_iterator it = dists.begin();
  for ( ; it != dists.end(); ++it ) {
    const vposition  pos1dflt = direct(gmtequator,direction::north,*it);
    const vposition  pos2dflt = direct(gmtequator,direction::south,*it);
    const vdirection dir1dflt = inverse(pos1dflt, gmtequator);
    const vdirection dir2dflt = inverse(pos2dflt, gmtequator);

    const vposition  pos1accu = direct(gmtequator,direction::north,*it,1e-16);
    const vposition  pos2accu = direct(gmtequator,direction::south,*it,1e-16);
    const vdirection dir1accu = inverse(pos1accu,gmtequator,1e-16);
    const vdirection dir2accu = inverse(pos2accu,gmtequator,1e-16);

    // Accept only a millimeter error for increased accuracy.
    EXPECT_NEAR( *it, dir1accu.distance, 1e-3 );
    EXPECT_NEAR( *it, dir2accu.distance, 1e-3 );
    // Expect default accuracy not be be more then a centimeter off from the a
    // higher accuracy.
    EXPECT_NEAR( dir1dflt.distance, dir1accu.distance, 1e-2 );
    EXPECT_NEAR( dir2dflt.distance, dir2accu.distance, 1e-2 );
  }
}


// Compare all positions against each other and, no negative distances, no null
// distances and no distances which are unreasonable.
TEST_F(VincentyVerificationTest, DistancesAreSane) {
  std::vector<vposition>::const_iterator it1 = positions.begin();
  for ( ; it1 != positions.end(); ++it1 ) {
    std::vector<vposition>::const_iterator it2 = positions.begin();
    for ( ; it2 != positions.end(); ++it2 ) {
      if ( it1 == it2 ) {
        break;
      }
      const vdirection dir1 = inverse(*it1,*it2);
      const vdirection dir2 = inverse(*it2,*it1);

      // No positions in the test shall be closer than 1 meter.
      EXPECT_GT( dir1.distance, 1.0 )
          << "Distances must be greater than 1m";

      // The greatest distance which can separate two positions on the earth
      // must be half of the largest circle which encloses the earth.
      EXPECT_LT( dir1.distance, 6378137*M_PI )
          << "No distance can be larger then half a sphere arc";

      // Expect the reverse distance to be the same. (Already tested prior to
      // this but not with real positions.
      EXPECT_FLOAT_EQ( dir1.distance, dir2.distance );

      // Expect opposite bearings to be the same. Skip comparing the poles
      // though, where directions are multiples of pi.
      if ( ! fabs( 2*M_PI - dir1.bearing2 ) < 1e-6 &&
           ! fabs( 2*M_PI - dir2.bearing2 ) < 1e-6 ) {
        EXPECT_NEAR( dir1.bearing1, dir2.bearing2, 1e-6 );
        EXPECT_NEAR( dir2.bearing1, dir1.bearing2, 1e-6 );
      }
    }
  }
}

TEST_F(VincentyVerificationTest, MiddlePositionReciprocity) {
  // From A to B
  const vdirection d1 = inverse(uddevalla,stockholm);
  const vposition  p_d1_b1 = direct(uddevalla,d1.bearing1,d1.distance/2);
  const vposition  p_d1_b2 = direct(stockholm,d1.bearing2,d1.distance/2);

  // From B to A
  const vdirection d2 = inverse(stockholm,uddevalla);
  const vposition  p_d2_b1 = direct(stockholm,d2.bearing1,d2.distance/2);
  const vposition  p_d2_b2 = direct(uddevalla,d2.bearing2,d2.distance/2);

  // Expect distances to be the same
  EXPECT_FLOAT_EQ(d1.distance,d2.distance)
      << "Distance A->B is not the same as B->A!";

  // Expect bearings to be the same, from the two computations.
  EXPECT_FLOAT_EQ(d1.bearing1,d2.bearing2)
      << "Bearing A->B is not equal to *reverse* bearing B->A!";
  EXPECT_FLOAT_EQ(d2.bearing1,d1.bearing2)
      << "Bearing B->A is not equal to *reverse* bearing A->B!";

  // Expect all positions to be the same.
  EXPECT_FLOAT_EQ(p_d1_b1.coords.a[0],p_d1_b2.coords.a[0]);
  EXPECT_FLOAT_EQ(p_d1_b1.coords.a[1],p_d1_b2.coords.a[1]);
  EXPECT_FLOAT_EQ(p_d1_b1.coords.a[0],p_d2_b1.coords.a[0]);
  EXPECT_FLOAT_EQ(p_d1_b1.coords.a[1],p_d2_b1.coords.a[1]);
  EXPECT_FLOAT_EQ(p_d2_b1.coords.a[0],p_d2_b2.coords.a[0]);
  EXPECT_FLOAT_EQ(p_d2_b1.coords.a[1],p_d2_b2.coords.a[1]);
}

TEST_F(VincentyVerificationTest, InverseReciprocity) {
  // From A to B
  const vdirection us = inverse(uddevalla,stockholm);
  const vdirection su = inverse(stockholm,uddevalla);
  const vdirection uk = inverse(uddevalla,karlstad);
  const vdirection ku = inverse(karlstad,uddevalla);
  const vdirection sk = inverse(stockholm,karlstad);
  const vdirection ks = inverse(karlstad,stockholm);

  // Expect the forward bearing to be identical to the reverse bearing when
  // computing A->B and B->A. The same goes for the reverse bearing.
  EXPECT_FLOAT_EQ(us.bearing1,su.bearing2)
      << "Bearing A->B is not equal to *reverse* bearing B->A!";
  EXPECT_FLOAT_EQ(su.bearing1,us.bearing2)
      << "Bearing A->B is not equal to *reverse* bearing B->A!";
  EXPECT_FLOAT_EQ(uk.bearing1,ku.bearing2)
      << "Bearing A->B is not equal to *reverse* bearing B->A!";
  EXPECT_FLOAT_EQ(ku.bearing1,uk.bearing2)
      << "Bearing A->B is not equal to *reverse* bearing B->A!";
  EXPECT_FLOAT_EQ(sk.bearing1,ks.bearing2)
      << "Bearing A->B is not equal to *reverse* bearing B->A!";
  EXPECT_FLOAT_EQ(ks.bearing1,sk.bearing2)
      << "Bearing A->B is not equal to *reverse* bearing B->A!";

  // Expect all distances to be the same.
  EXPECT_FLOAT_EQ(us.distance,su.distance) << "Distance A->B is not the same as B->A!";
  EXPECT_FLOAT_EQ(uk.distance,ku.distance) << "Distance A->B is not the same as B->A!";
  EXPECT_FLOAT_EQ(sk.distance,ks.distance) << "Distance A->B is not the same as B->A!";
}



/**
 * Testing class for pure performance estimates.
 */
class VincentyPerformanceTest : public testing::Test
{
 protected:
  vposition northpole;
  vposition southpole;

  vposvec positions;
  vdirvec directions;

  VincentyPerformanceTest()
      : northpole( M_PI/2,0),
        southpole(-M_PI/2,0),
        positions(),
        directions()
  {
  }

  virtual void SetUp() {
    // Setup things, which might throw.
  }

  virtual void TearDown() {
    // Remove things setup by SetUp.
  }
};


TEST_F(VincentyPerformanceTest,PerformanceTest) {
  // Workout the amount of memory we have available.
  const long numpages = sysconf(_SC_PHYS_PAGES);
  const long pagesize = sysconf(_SC_PAGE_SIZE);

  // Approx amount of memory needed.
  const unsigned int datasize = sizeof(vposition);

  // Large amount of positions?
  size_t numpositions = 2e5;

  unsigned long memsize;
  if ( numpages > 0 && pagesize > 0 ) {
    std::cout << " -- Detected " << ((numpages*pagesize)>>20)
              << "MiB of memory" << std::endl;
    memsize = numpages*pagesize;
  } else {
    std::cout << " -- Could not detect available memory!"
              << " Assuming there is at least 128MiB" << std::endl;
    memsize = 128*1024*1024;
  }

  if ( numpositions*datasize > memsize/2 ) {
    numpositions = memsize/datasize;
    std::cout << " -- Available memory not enough for 2e6 datapoints!" << std::endl;
  }

  std::cout << " -- Using " << numpositions << " datapoints "
            << "(" << (numpositions*datasize>>20) << "MiB)" << std::endl;

  // Reserve space.
  positions.reserve(numpositions);

  // Generate a lot of random positions.
  srand48(123456789);
  for ( size_t i=0; i<numpositions; ++i ) {
    const double lat = 2*M_PI * ( drand48() - 0.5 );
    const double lon =   M_PI * ( drand48() - 0.5 );
    positions.push_back(vposition(lat,lon));
  }

  // Assert that we have enough positions.
  ASSERT_GT( positions.size(), 1 );

  // Compute "sibling" vdirection using inverse formula. Use the distance
  // computed for a calculation so that the compiler does not optimize away
  // anything.
  struct timeval tm_start, tm_stop;
  double avg_distance = 0.0;
  std::vector<vposition>::const_iterator first, second;
  size_t i;
  gettimeofday( &tm_start, 0 );
  for ( first = positions.begin(), second = positions.begin()+1, i=1;
        second != positions.end();
        ++first, ++second, ++i )
  {
    const vdirection dir = inverse(*first,*second);
    avg_distance = ( avg_distance * i + dir.distance ) / ( i + 1 );
  }
  gettimeofday( &tm_stop, 0 );

  const double inv_seconds =
      ( ( tm_stop.tv_sec  - tm_start.tv_sec  ) +
        ( tm_stop.tv_usec - tm_start.tv_usec ) / 1.e6 );


  const double dirs[2] = {direction::east,direction::west};

  // Compute a "jump" and use the vposition.
  double avg_latitude = 0.0;
  double avg_longitude = 0.0;
  gettimeofday( &tm_start, 0 );
  for ( first = positions.begin(), i=1;
        first != positions.end();
        ++first, ++i )
  {
    const vposition pos = direct(*first,dirs[i%2],1e5);
    avg_latitude  = ( avg_latitude  * i + pos.coords.a[0] ) / ( i + 1 );
    avg_longitude = ( avg_longitude * i + pos.coords.a[1] ) / ( i + 1 );
  }
  gettimeofday( &tm_stop, 0 );

  const double dir_seconds =
      ( ( tm_stop.tv_sec  - tm_start.tv_sec  ) +
        ( tm_stop.tv_usec - tm_start.tv_usec ) / 1.e6 );

  const double inv_performance = (numpositions-1)/(inv_seconds*1000);
  const double dir_performance = (numpositions  )/(dir_seconds*1000);

  std::cout.setf(std::ios::fixed,std::ios::floatfield);
  std::cout.precision(3);
  std::cout
      << " -- inverse()/sec: " << std::setw(8) << inv_performance << "k" << std::endl
      << " -- direct()/sec:  " << std::setw(8) << dir_performance << "k" << std::endl
      << " -- Inverse time:  " << std::setw(8) << inv_seconds << "s" << std::endl
      << " -- Direct time:   " << std::setw(8) << dir_seconds << "s" << std::endl
      << " -- Avg dist:      " << std::setw(8) << avg_distance/1000 << "km" << std::endl
      << " -- Avg lat:       " << std::setprecision(8) << avg_latitude << std::endl
      << " -- Avg lon:       " << std::setprecision(8) << avg_longitude << std::endl;

  // This is highly machine dependent and should perhaps not be performed as a
  // test like this.
  //EXPECT_LT(750 ,inv_performance)
  //    << "Performance is suspiciously low! Check accuracy or other anomalies.";
  //EXPECT_LT(1200,dir_performance)
  //    << "Performance is suspiciously low! Check accuracy or other anomalies.";
}

} // namespace end
