// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2; -*-

#include "vincenty.h"

#include <cstdlib>
#include <unistd.h>
#include <sys/time.h>

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

  VincentyBasicTest()
      : la00lo00(0,0),
        la10lo10(to_rad(10),to_rad(10)),
        nullposition(),
        nullposition_init(0.0,0.0),
        northpole(to_rad( 90.0),0.0),
        southpole(to_rad(-90.0),0.0),
        sweden(to_rad(58),to_rad(16))
  {
  }

  virtual ~VincentyBasicTest() 
  {
    // Nothing to remove.
  }
};


// Test converts from and to degrees and radians.
TEST_F(VincentyBasicTest, ConvertersDegreesRadians) {
  EXPECT_FLOAT_EQ( 3.14159265358979, to_rad(180.0));
  EXPECT_FLOAT_EQ( 1.57079632679490, to_rad( 90.0));
  EXPECT_FLOAT_EQ( 1.36135681655558, to_rad( 78.0));
  EXPECT_FLOAT_EQ(-1.36135681655558, to_rad(-78.0));
  EXPECT_FLOAT_EQ( 0.0, to_rad( 0.0));
  EXPECT_FLOAT_EQ(-0.0, to_rad(-0.0));
  
  EXPECT_FLOAT_EQ(180.0, to_deg(3.14159265358979));
  EXPECT_FLOAT_EQ( 90.0, to_deg(1.57079632679490));
  EXPECT_FLOAT_EQ( 78.0, to_deg(1.36135681655558));
  EXPECT_FLOAT_EQ(-56.0, to_deg( to_rad(-56.0) ));
  EXPECT_FLOAT_EQ(  0.0, to_deg( 0.0));
  EXPECT_FLOAT_EQ( -0.0, to_deg(-0.0));
}


// The default constructor shall initialize with 0 (zero), and initializing
// with zero shall set values to zero.
TEST_F(VincentyBasicTest, DefaultConstructorsAreNull) {
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


TEST_F(VincentyBasicTest, NonNegativeDistances) {
  const vdirection d1 = inverse(la00lo00,northpole);
  const vdirection d2 = inverse(la10lo10,northpole);
  const vdirection d3 = inverse(la10lo10,la00lo00);
  const vdirection d4 = inverse(sweden,northpole);
  const vdirection d5 = inverse(sweden,southpole);
  EXPECT_GT( d1.distance, 0.0 );
  EXPECT_GT( d2.distance, 0.0 );
  EXPECT_GT( d3.distance, 0.0 );
  EXPECT_GT( d4.distance, 0.0 );
  EXPECT_GT( d5.distance, 0.0 );
}


// Two non-equal positions shall result in a non-negative distance.
TEST_F(VincentyBasicTest, NonNegativeDistancesRandomized) {
  srand48(123456789);
  const unsigned int loops = 50;
  std::cout << " -- Testing " << loops 
            << " randomized distances" << std::endl;
  for ( unsigned int i=0; i<loops; ++i ) {
    const double lat1 = 2*M_PI * ( drand48() - 0.5 );
    const double lon1 =   M_PI * ( drand48() - 0.5 );
    const double lat2 = 2*M_PI * ( drand48() - 0.5 );
    const double lon2 =   M_PI * ( drand48() - 0.5 );
    vdirection dir = inverse(lat1,lon1,lat2,lon2);
    ASSERT_GT( dir.distance, 0.0 ) << "Distances cannot be negative";
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
  std::cout << " -- Testing " << loops 
            << " randomized positions" << std::endl;
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
TEST_F(VincentyBasicTest, SeparationDistances) {
  const unsigned int dist = 5000;
  for ( unsigned int d=1; d<6; ++d ) {
    std::cout << " -- Testing " << 2*dist*d << "m" << std::endl;
    for ( unsigned int i=1; i<9; ++i ) { 
      // Centre position, aling the meridian.
      const vposition pC(to_rad(10*i),0);

      // North, south, east and west.
      vposition pN, pS, pE, pW;

      pN = direct(pC,direction::n,dist*d);
      pS = direct(pC,direction::s,dist*d);
      pE = direct(pC,direction::e,dist*d);
      pW = direct(pC,direction::w,dist*d);

      vdirection dNS = inverse(pN,pS);
      vdirection dEW = inverse(pE,pW);

      EXPECT_FLOAT_EQ(2*dist*d,dNS.distance);
      EXPECT_FLOAT_EQ(2*dist*d,dEW.distance);
    }
  }
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
TEST_F(VincentyVerificationTest, RealDistancesAreSane) {
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

      // Expect opposite bearings to be the exactly the same value only
      // opposite direction, represented by a sign.
      EXPECT_FLOAT_EQ( dir1.bearing1, -dir2.bearing2 );
      EXPECT_FLOAT_EQ( dir2.bearing1, -dir1.bearing2 );
    }
  }
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

  long memsize;
  if ( numpages > 0 && pagesize > 0 ) {
    // Lets use up 1/128 of the total memsize.
    memsize = numpages*pagesize >> 7;
  } else {
    // Assume we have at least 2MiB (else just let the bastard swap).
    memsize = 2*1024*1024;
  }

  // How many positions fit? (We do not really care how many it becomes as long
  // as the amount of memory is fairly sane).
  const size_t numpositions = memsize / sizeof(vposition);

  // Reserve space.
  positions.reserve(numpositions);
  directions.reserve(numpositions);

  // Generate a lot of random positions.
  srand48(01010101);
  for ( size_t i=0; i<numpositions; ++i ) {
    const double lat = 2*M_PI * ( drand48() - 0.5 );
    const double lon =   M_PI * ( drand48() - 0.5 );
    positions.push_back(vposition(lat,lon));
  }

  // Assert that we have enough positions.
  ASSERT_GT( positions.size(), 1 );

  std::cout
      << "Num positions:  " << positions.size() << std::endl;
  

  // Compute "sibling" vdirection using inverse formula. Store the vdirection
  // in a vector and use it to compute a value.
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
    directions.push_back(dir);
  }
  gettimeofday( &tm_stop, 0 );
  
  const double inv_seconds =
      ( ( tm_stop.tv_sec  - tm_start.tv_sec  ) +
        ( tm_stop.tv_usec - tm_start.tv_usec ) / 1.e6 );


  // Compute a "jump" and use the vposition.
  double avg_latitude = 0.0;
  double avg_longitude = 0.0;
  gettimeofday( &tm_start, 0 );
  for ( first = positions.begin(), i=1;
        first != positions.end();
        ++first, ++i )
  {
    const vposition pos = direct(*first,direction::east,1e5);
    avg_latitude = ( avg_latitude * i + pos.coords.a[0] ) / ( i + 1 );
    avg_longitude = ( avg_longitude * i + pos.coords.a[1] ) / ( i + 1 );
  }
  gettimeofday( &tm_stop, 0 );

  const double dir_seconds =
      ( ( tm_stop.tv_sec  - tm_start.tv_sec  ) +
        ( tm_stop.tv_usec - tm_start.tv_usec ) / 1.e6 );

  std::cout 
      << "Inverse time:   " << inv_seconds << "s" << std::endl
      << "inverse()/sec:  " << (numpositions-1)/inv_seconds << std::endl
      << "Direct time:    " << dir_seconds << "s" << std::endl
      << "direct()/sec:   " << (numpositions)/dir_seconds << std::endl
      << "  avg dist:     " << avg_distance << std::endl
      << "  avg lat:      " << avg_latitude << std::endl
      << "  avg lon:      " << avg_longitude << std::endl;
}

} // namespace end


int
main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
