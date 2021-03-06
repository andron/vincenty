// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2; -*-

/*
  Copyright (C) 2009, 2010, 2011, 2012, 2013, anders.ronnbrant@gmail.com
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

#ifndef __vincenty_h__
#define __vincenty_h__

#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <vector>

//! Defines uint64_t, and more. <cstdint> in next standard.
#include <stdint.h>

typedef double v2df __attribute__((vector_size(16)));
typedef float  v4sf __attribute__((vector_size(16)));

/*! 
 * @brief
 * Namespace with all relevant methods and subclasses for computing vincenty's
 * direct and inverse formulas.
 *
 * The namespace encapsulates the two functions for solving vincentys direct
 * and inverse formulas, the two classes vposition and vdirection which are
 * used as return values as well as parameters to the functions.  Some
 * simplified functions (conveniance functions) are also available for quickly
 * getting bearings and distance between positions..
 */
namespace vincenty {

class vposition;
class vdirection;

/*! @addtogroup vector_operands Vector operands
 * @brief Unions for holding vector operands.
 *
 * The union construction is necessary to for initializing a vector from any of
 * the corresponding base types.
 */
//!@{
union v2df_u {
  v2df v;
  double a[2];
};

union v4sf_u {
  v4sf v;
  float a[4];
};
//!@}


/*!
 * @var static const double default_accuracy
 *
 * Decides the maximum error in the calculations of a position. A value of
 * \f$10^{-10}\f$ gives and error of \f$<0.1\f$ [m].
 *
 * Has an impact on performance!!
 */
static const double default_accuracy = 1.0e-11;


/*!
 * @brief A set of fixed directions.
 *
 * A set of "standard" directions which can be nifty to use here and there. All
 * values are in radians. Should work well together with vdirection.
 */
namespace direction {
const double north = 0*M_PI/4;
const double east  = 2*M_PI/4;
const double south = 4*M_PI/4;
const double west  = 6*M_PI/4;
const double northeast = 1*M_PI/4;
const double southeast = 3*M_PI/4;
const double southwest = 5*M_PI/4;
const double northwest = 7*M_PI/4;
const double n  = north;
const double e  = east;
const double s  = south;
const double w  = west;
const double ne = northeast;
const double se = southeast;
const double sw = southwest;
const double nw = northwest;
}


/*!
 * @brief Stream modifiers for printing vposition and vdirection to ostreams.
 */
class format
{
  format();
  ~format();

  //! Map to hold ostream xalloc values.
  static std::map<const std::string,long> _format; 

 public:
  //! Enum values representing the different formats stored in the format-map.
  enum { DMS, DM, DD, DEG, RAD };
  
  //! Format coordinates in degrees, minutes and decimal seconds.
  static std::ostream& dms(std::ostream& os);

  //! Format coordinates in degrees and decimal minutes.
  static std::ostream& dm(std::ostream& os);
      
  //! Format coordinates in decimal degrees.
  static std::ostream& dd(std::ostream& os);

  //! Format direction in degrees.
  static std::ostream& deg(std::ostream& os);

  //! Format direction in radians.
  static std::ostream& rad(std::ostream& os);
      
  /*!
   * @brief Returns the current coordinate format value.
   * @return ostream xalloc value.
   */
  static long coordinate_type();

  /*!
   * @brief Returns the current direction format value.
   * @return ostream xalloc value.
   */
  static long direction_type();
};


/*!
 * @brief Class for handling a position.
 *
 * The class holds two aligned doubles representing latitude and longitude in
 * radians. The array is ordered [lat,lon]. Different methods for converting
 * radians into degress, minutes etc are provided. (The methods are primarily
 * used by the operator<<).
 */
class vposition
{
  union v2df_u {
    v2df v;
    double a[2];
  };
   
 public:
  vposition();
  vposition( double lat, double lon );

  //! Accessors
  double latitude() const;
  double longitude() const;

  //! Holds latitude and longitude values.
  v2df_u coords;
      
  static int deg( const double radians );
  static int min( const double radians );
  static int sec( const double radians );
  static double degf( const double radians );
  static double minf( const double radians );
  static double secf( const double radians );

  friend std::ostream& operator<<( std::ostream& os, const vposition& rhs );
  friend bool operator==( const vposition& lhs, const vposition& rhs );

  //! Position from "this" in the "rhs" direction.
  vposition operator+( const vdirection& rhs ) const;
  //! Position from "this" in the opposite direction of "rhs".
  vposition operator-( const vdirection& rhs ) const;
  //! Direction (vector definition) from rhs to "this".
  vdirection operator-( const vposition& rhs ) const;
  //! Computes the midpoint between two positions.
  vposition operator^( const vposition& rhs ) const;
};

//! Vector of vpositions.
typedef std::vector<vposition> vposition_vector;

/*!
 * @brief Class for handling a direction.
 *
 * The class holds three doubles (not array), bearing1, distance and
 * bearing2. The value of bearing1 is the direction in radians from the first
 * position (or lat,lon pair) given to vincenty_inverse. The distance is of
 * course the distance between the points, and bearing2 is the reversed
 * direction bearing. I.e. from the second position to the first.
 *
 * @li @c bearing1 [radians]
 * @li @c bearing2 [radians]
 * @li @c distance [m]
 */
class vdirection
{
 public:
  vdirection();
  vdirection( double bearing1, double distance, double bearing2 = 0 );

  //! Bearing from the "first" position to the "second".
  double bearing1;

  //! The distance between two positions.
  double distance;

  //! Bearing from the "second" positions to the "first".
  double bearing2;

  friend std::ostream& operator<<( std::ostream& os, const vdirection& rhs );
  friend bool operator==( const vdirection& lhs, const vdirection& rhs );

  vdirection operator/( const double rhs ) const;
  vdirection operator*( const double rhs ) const;
};

//! Vector of vdirections.
typedef std::vector<vdirection> vdirection_vector;


// ------------------------------------------------------------------------

/**
 * @defgroup vincenty_functions Vincenty functions
 * @brief Vincenty's two main functions, the inverse and direct formula.
 */

//!@{

/*!
 * @brief Main function for Vincenty's direct formula.
 *
 * Vincenty's direct formula takes a position, a direction and a distance as
 * input and then computes the destination position.
 *
 * @param lat      Latitude of the current position [radians].
 * @param lon      Longitude of the current position [radians].
 * @param bearing  Direction in which to move [radians].
 * @param distance Distance to move [m].
 * @param accuracy Maximum error for the computation [-].
 *
 * @return A vposition which holds the destination position.
 *
 * @callergraph
 */
vposition direct(
    const double lat,
    const double lon,
    const double bearing,
    const double distance,
    const double accuracy = default_accuracy ) __attribute__ ((pure));

/*!
 * @brief Main function for Vincenty's inverse formula.
 *
 * Vincenty's inverse formula takes two positions and computes the distance
 * between them and the bearing from each position towards the other. (In
 * vdirection bearing1 is the bearing from position1 towards position2, and
 * bearing2 the opposite).
 *
 * @param lat1     Latitude of the first position [radians].
 * @param lon1     Longitude of the first position [radians].
 * @param lat2     Latitude of the second position [radians].
 * @param lon2     Longitude of the second position [radians].
 * @param accuracy Maximum error for the computation [-].
 *
 * @return A vdirection which holds the direction and distance information
 * between the two positions.
 *
 * @callergraph
 */
vdirection inverse(
    const double lat1,
    const double lon1,
    const double lat2,
    const double lon2,
    const double accuracy = default_accuracy ) __attribute__ ((pure));

//!@}
// ------------------------------------------------------------------------

   
/*!
 * @brief Derived direct function.
 *
 * Derived function that takes a vposition as parameter instead of doubles.
 *
 * @param pos Source position.
 * @param bearing Direction in which to travel [radians].
 * @param distance Distance to travel [m].
 * @param accuracy Maximum error for the computation [-].
 *
 * @return vposition struct which holds the destination position.
 */
vposition direct(
    const vposition& pos,
    const double bearing,
    const double distance,
    const double accuracy = default_accuracy ) __attribute__ ((pure));


/*!
 * @brief Derived direct function.
 *
 * Derived function that takes a vposition and a vdirection as parameters
 * instead of doubles.
 *
 * @param pos Source position.
 * @param dir Direction in which to travel, vdirection holds both bearing and
 * distance. (Attribute bearing1 is used, bearing2 have no effect).
 * @param accuracy Maximum error for the computation [-].
 *
 * @return vposition struct which holds the destination position.
 */
vposition direct(
    const vposition& pos,
    const vdirection& dir,
    const double accuracy = default_accuracy ) __attribute__ ((pure));


/*!
 * @brief Derived inverse function.
 *
 * Derived function that takes two vpositions as input.
 *
 * @param pos1 First position.
 * @param pos2 Second position.
 * @param accuracy Maximum error for the computation [-].
 *
 * @return vdirection struct.
 */
vdirection inverse(
    const vposition& pos1,
    const vposition& pos2,
    const double accuracy = default_accuracy ) __attribute__ ((pure));


/*!
 * @addtogroup vincenty_derived_functions Vincenty simplified functions
 *
 * @brief Simple functions for various computations.
 *
 * @details Simplified functions to compute only the distance, bearing or
 * latitude/longitude, one at the time. All uses the inverse or direct
 * function internally. Uses only the default accuracy value, which should be
 * enough.
 */

/**@{*/

/*!
 * @brief Get distance between two positions.
 *
 * @param pos1 First position.
 * @param pos2 Second position.
 * @return Distance between the positions [m].
 */
double get_distance(
    const vposition& pos1,
    const vposition& pos2 ) __attribute__ ((pure));

/*! 
 * @brief Get distance between two positions given by a pair of lat,lon
 * doubles.
 *
 * @param lat1 First position latitude.
 * @param lon1 First position longitude.
 * @param lat2 Second position latitude.
 * @param lon2 Second position longitude.
 * @return Distance between the positions [m].
 */
double get_distance(
    const double lat1,
    const double lon1,
    const double lat2,
    const double lon2 ) __attribute__ ((pure));

/*!
 * @brief Get bearing between two positions.
 *
 * @param pos1 Source position.
 * @param pos2 Destination position.
 * @return Bearing from pos1 towards pos2 [radians].
 */
double get_bearing(
    const vposition& pos1,
    const vposition& pos2 ) __attribute__ ((pure));

/*! 
 * @brief Get bearing between two positions given by a pair of lat,lon doubles.
 *
 * @param lat1 First position latitude.
 * @param lon1 First position longitude.
 * @param lat2 Second position latitude.
 * @param lon2 Second position longitude.
 * @return Bearing from pos1 towards pos2 [radians].
 */
double get_bearing(
    const double lat1,
    const double lon1,
    const double lat2,
    const double lon2 ) __attribute__ ((pure));

//!@}

/*!
 * @brief Convert to radians.
 */
double to_rad( const double degrees ) __attribute__ ((pure));

/*!
 * @brief Convert to degrees.
 */
double to_deg( const double radians ) __attribute__ ((pure));


/*!
 * @brief Compare doubles by "unit in last place", ULPs.
 *
 * Default to allow for a difference of 8-bit positions, a total range of
 * 16-bit positions.
 */
bool ulpcmp( const double x,
             const double y,
             const uint64_t ulpdiff = 8 ) __attribute__ ((pure));

/*!
 * @brief Compare doubles by "unit in last place", ULPs. Inlined version.
 */
bool ulpcmp_inline( const double x,
                    const double y,
                    const uint64_t ulpdiff = 8 ) __attribute__ ((pure));

} // namespace end

#endif
