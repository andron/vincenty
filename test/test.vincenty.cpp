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
    vposition p(lat1,lon1);
    for ( unsigned int d=1; d<9; ++d ) {
      for ( unsigned int i=1; i<9; ++i ) { 
        // North, south, east and west.
        vposition pN, pS, pE, pW;
        
        pN = direct(p,direction::n,dist*d);
        pS = direct(p,direction::s,dist*d);
        pE = direct(p,direction::e,dist*d);
        pW = direct(p,direction::w,dist*d);

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
  
  // The all positions to be the same.
  EXPECT_FLOAT_EQ(p_d1_b1.coords.a[0],p_d1_b2.coords.a[0]);
  EXPECT_FLOAT_EQ(p_d1_b1.coords.a[1],p_d1_b2.coords.a[1]);
  EXPECT_FLOAT_EQ(p_d1_b1.coords.a[0],p_d2_b1.coords.a[0]);
  EXPECT_FLOAT_EQ(p_d1_b1.coords.a[1],p_d2_b1.coords.a[1]);
  EXPECT_FLOAT_EQ(p_d2_b1.coords.a[0],p_d2_b2.coords.a[0]);
  EXPECT_FLOAT_EQ(p_d2_b1.coords.a[1],p_d2_b2.coords.a[1]);
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

  // Use we use 2 million positions?
  size_t numpositions = 3e6;

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

  EXPECT_LT(750 ,inv_performance)
      << "Performance is suspiciously low! Check accuracy or other anomalies."; 
  EXPECT_LT(1200,dir_performance)
      << "Performance is suspiciously low! Check accuracy or other anomalies."; 
}

} // namespace end



int
main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
