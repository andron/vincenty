// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2; -*-

/*
  Copyright (C) 2009, Anders Ronnbrant - andro@lysator.liu.se
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  You should have received a copy of the FreeBSD license, if not see:
  <http://www.freebsd.org/copyright/freebsd-license.html>.
*/

#include "vincenty.h"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <string>


// This shit shall not be visible outside the library, hide all symbols.
#pragma GCC visibility push(internal)
const double a  = 6378137.0000;
const double b  = 6356752.3142;
const double f  = (a-b)/a;
const double _f = ((a*a) / (b*b)) - 1;

inline double A_full_precision( const double ) __attribute__ ((pure));
inline double B_full_precision( const double ) __attribute__ ((pure));

inline double deltasigma_full_precision(
    const double,
    const double,
    const double,
    const double ) __attribute__ ((pure));
#pragma GCC visibility pop


namespace vincenty
{
// Direct formula
// ------------------------------------------------------------------------
vposition direct( const double lat,
                  const double lon,
                  const double alpha1,
                  const double s,
                  const double accuracy ) {
  // If equal return immediately.
  if ( ulpcmp(0,s) ) {
    return vposition(lat,lon);
  }
  const double tan_U1     = (1-f) * tan(lat);
  const double cos_U1     = 1 / sqrt( (1 + tan_U1 * tan_U1) );
  const double sin_U1     = tan_U1 * cos_U1;
  const double cos_alpha1 = cos(alpha1);
  const double sigma1     = atan2( tan_U1, cos_alpha1 );
  const double sin_alpha1 = sin(alpha1);
  const double sin_alpha  = cos_U1 * sin_alpha1;
  const double cos2_alpha = 1 - sin_alpha*sin_alpha;
  const double u2         = cos2_alpha * _f;

  const double A          = A_full_precision(u2);
  const double B          = B_full_precision(u2);

  double sigma            = s / ( b * A );

  double _sigma;
  double sin_sigma;
  double cos_sigma;
  double cos_2sigmam;

  // Prevent loop deadlock.
  unsigned int i = 10;
  do {
    sin_sigma   = sin(sigma);
    cos_sigma   = cos(sigma);
    cos_2sigmam = cos( 2*sigma1 + sigma );
      
    const double delta_sigma = 
        deltasigma_full_precision(B,sin_sigma,cos_sigma,cos_2sigmam);

    _sigma = sigma;
    sigma = s / (b*A) + delta_sigma;
  } while ( fabs(sigma-_sigma) > accuracy && --i );
   
  const double C = 
      f/16*cos2_alpha * ( 4 + f*(4-3*cos2_alpha) );

  const double lambda = 
      atan2( sin_sigma*sin_alpha1,
             cos_U1*cos_sigma - sin_U1*sin_sigma*cos_alpha1 );
   
  const double L = 
      lambda - 
      (1-C)*f*sin_alpha * 
      ( sigma + 
        C*sin_sigma * ( cos_2sigmam +
                        C*cos_sigma * ( -1 +
                                        2*cos_2sigmam*cos_2sigmam) ) );
   
  const double tmp = 
      sin_U1*sin_sigma - cos_U1*cos_sigma*cos_alpha1;
   
  const double lat2 = 
      atan2( sin_U1*cos_sigma + cos_U1*sin_sigma*cos_alpha1, 
             (1-f)*sqrt( sin_alpha*sin_alpha + tmp*tmp ) );

  /*
    Skip computing the reversed bearing, the returned position does not
    have a member to return the value. The implementation of how the
    bearing is computed is keept as reference.
  */
  //const double bearing_reversed = atan2(-sin_alpha, tmp);
   
  return vposition(lat2, lon+L);
}

vposition direct( const vposition& pos,
                  const double bearing,
                  const double distance,
                  const double accuracy ) {
  return direct( pos.coords.a[0],
                 pos.coords.a[1],
                 bearing,
                 distance,
                 accuracy );
}

vposition direct( const vposition& pos,
                  const vdirection& dir,
                  const double accuracy ) {
  return direct( pos.coords.a[0],
                 pos.coords.a[1],
                 dir.bearing1,
                 dir.distance,
                 accuracy );
}

   
// Inverse formula
// ------------------------------------------------------------------------
vdirection inverse( const double lat1,
                    const double lon1,
                    const double lat2,
                    const double lon2,
                    const double accuracy ) {
  // If equal return immediately.
  if ( ulpcmp(lat1,lat2) &&
       ulpcmp(lon1,lon2) ) {
    return vdirection(0.0,0.0,0.0);
  }
#define U1 atan( (1-f) * tan(lat1) )
#define U2 atan( (1-f) * tan(lat2) )
  const double sin_U1 = sin(U1);
  const double cos_U1 = cos(U1);
  const double sin_U2 = sin(U2);
  const double cos_U2 = cos(U2);
#undef U1
#undef U2
  const double L = lon2-lon1;
  double lambda  = L;

  double sin_lambda;
  double cos_lambda;

  double sin_sigma;
  double cos_sigma;

  double cos2_alpha;
  double cos_2sigmam;
  double sigma;
  double _lambda;

  // Prevent loop deadlock.
  unsigned int i = 10;
  do {
    sin_lambda =
        sin(lambda);
    cos_lambda =
        cos(lambda);

    sin_sigma = 
        sqrt( pow(cos_U2*sin_lambda,2) + 
              pow(cos_U1*sin_U2 - sin_U1*cos_U2*cos_lambda,2) );
    cos_sigma =
        sin_U1*sin_U2 + cos_U1*cos_U2*cos_lambda;
     
    const double sin_alpha = 
        cos_U1*cos_U2*sin_lambda/sin_sigma;
     
    cos2_alpha =
        1 - pow(sin_alpha,2);
     
    double C;
    if ( ulpcmp(cos2_alpha,0.0,16) ) {
      cos_2sigmam = 0;
      C = 0;
    } else {
      cos_2sigmam = cos_sigma - 2*sin_U1*sin_U2/cos2_alpha;
      C = f/16 * cos2_alpha * ( 4 + f * (4 - 3*cos2_alpha) );
    }
     
    sigma = 
        atan2( sin_sigma, cos_sigma );
    
    _lambda = lambda;
    lambda = 
        L + (1-C) * f * sin_alpha * 
        ( sigma + C * sin_sigma * 
          ( cos_2sigmam + C * cos_sigma * 
            ( -1 + 2 * cos_2sigmam*cos_2sigmam ) ) );
  } while ( fabs(lambda-_lambda) > accuracy && --i );
  
  const double u2 = cos2_alpha * _f;

  const double delta_sigma = 
      deltasigma_full_precision( B_full_precision(u2),
                                 sin_sigma,
                                 cos_sigma,
                                 cos_2sigmam );
  
  const double atany1 = cos_U2*sin_lambda;
  const double atanx1 = cos_U1*sin_U2 - sin_U1*cos_U2*cos_lambda;
  double p1p2 = atan2( atany1 , atanx1 );

  if ( p1p2 < 0 ) {
    p1p2 = p1p2 + 2*M_PI;
  }

  const double atany2 = cos_U1*sin_lambda;
  const double atanx2 = -sin_U1*cos_U2 + cos_U1*sin_U2*cos_lambda;

  // Scary, but the reverse bearing seems to need a "180 degree turn". At
  // least to be correct with the intervall [0,2*M_PI].
  double p2p1 = atan2( atany2 , atanx2 ) + M_PI;

  const double s = b * A_full_precision(u2) * ( sigma - delta_sigma );

  return vdirection(p1p2,s,p2p1);
}

vdirection inverse( const vposition& pos1,
                    const vposition& pos2,
                    const double accuracy ) {
  return inverse( pos1.coords.a[0],
                  pos1.coords.a[1],
                  pos2.coords.a[0],
                  pos2.coords.a[1],
                  accuracy );
}


// Simple functions.
// ------------------------------------------------------------------------

// Distance
double get_distance( const vposition& pos1,
                     const vposition& pos2 ) {
  const vdirection foo = inverse( pos1, pos2 );
  return foo.distance;
}

double get_distance( const double lat1,
                     const double lon1,
                     const double lat2,
                     const double lon2 ) {
  const vdirection foo = inverse( lat1, lon1, lat2, lon2 );
  return foo.distance;
}

// Bearing
double get_bearing( const vposition& pos1,
                    const vposition& pos2 ) {
  const vdirection foo = inverse( pos1, pos2 );
  return foo.bearing1;
}

double get_bearing( const double lat1,
                    const double lon1,
                    const double lat2,
                    const double lon2 ) {
  const vdirection foo = inverse( lat1, lon1, lat2, lon2 );
  return foo.bearing1;
}


// Convert between radians and degrees.
double to_rad( const double degrees )
{
  return ( degrees / 180.0 ) * M_PI;
}

double to_deg( const double radians )
{
  return ( radians * 180.0 ) / M_PI;
}


// ULP compare of doubles.
inline bool
ulpcmp( const double x, const double y, const uint64_t ulpdiff ) {
  assert( ulpdiff <= 1024*1024 );
  typedef uint64_t __attribute__((__may_alias__)) alias_t;
  const uint64_t bits = *(alias_t*)&x - *(alias_t*)&y;
  if ( bits > ulpdiff || -bits > ulpdiff ) {
    return false;
  } else {
    return true;
  }
}

} // namespace end


// Inlined functions for readability.
// ------------------------------------------------------------------------
inline double
A_full_precision( const double u2 ) {
  return 1 + u2/16384 * ( 4096 + u2*( -768 + u2*(320 - 175*u2) ) );
}

inline double
B_full_precision( const double u2 ) {
  return u2/1024 * ( 256 + u2*( -128 + u2*(74 - 47*u2) ) );
}

inline double
deltasigma_full_precision( const double B,
                           const double sin_sigma,
                           const double cos_sigma,
                           const double cos_2sigmam ) {
  return
      B * sin_sigma * 
      ( cos_2sigmam + 
        B/4 * ( cos_sigma * ( -1 + 2*cos_2sigmam*cos_2sigmam ) - 
                B/6 * cos_2sigmam * 
                ( -3+4*sin_sigma*sin_sigma ) * 
                ( -3+4*cos_2sigmam*cos_2sigmam ) ) );
}
// ------------------------------------------------------------------------
