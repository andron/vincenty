// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2; -*-

/*
  Copyright (C) 2009 by Anders Ronnbrant - andro@lysator.liu.se
  
  This file is part of libvincenty

  libvincenty is free software: you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation, either version 3 of the License, or (at your option) any
  later version.

  You should have received a copy of the GNU Lesser General Public License
  along with libvincenty.  If not, see <http://www.gnu.org/licenses/>.

  Contributors:
  # Anders Ronnbrant - Initial implementation.
*/

#ifndef __vincenty_h__
#include "vincenty.h"
#endif

#include <iomanip>

namespace vincenty
{
// Geographical position
// ------------------------------------------------------------------------

//! Constructor, defaults latitude and longitude to zero (0).
vposition::vposition()
    : coords()
{
  coords.a[0] = 0;
  coords.a[1] = 0;
}

//! Constructor taking two doubles for initialization.
vposition::vposition( double _lat, double _lon )
    : coords()
{
  coords.a[0] = _lat;
  coords.a[1] = _lon;
}


//! Integer degrees from float radian.
int vposition::deg( const double rad )
{
  return int( to_deg(rad) );
}

//! Extracts integer minutes from float radian.
int vposition::min( const double rad )
{
  return int( ( degf(rad) - deg(rad) ) * 60 );
}

//! Extracts integer seconds from float radian.
int vposition::sec( const double rad )
{
  return int( ( minf(rad) - min(rad) ) * 60 );
}


//! Converts radians to degrees.
double vposition::degf( const double rad )
{
  return to_deg(rad);
}

//! Extracts decimal part minutes from float radian.
double vposition::minf( const double rad )
{
  return ( degf(rad) - deg(rad) ) * 60;
}

//! Extracts decimal part seconds from float radian.
double vposition::secf( const double rad )
{
  return ( minf(rad) - min(rad) ) * 60;
}


// Operators for vposition.
bool operator==( const vposition& lhs, const vposition& rhs )
{
  if ( ulpcmp(lhs.coords.a[0], rhs.coords.a[0]) &&
       ulpcmp(lhs.coords.a[1], rhs.coords.a[1]) ) {
    return true;
  } else {
    return false;
  }
}


// Geographical direction
// ------------------------------------------------------------------------

//! Constructor, defaults bearings and distance to 0 (zero).
vdirection::vdirection()
    : bearing1(0), distance(0), bearing2(0)
{
}

/*!
 * Constructor taking three doubles for initialization.
 *
 * !!OBS!! This constructor allows for invalid settings. I.e. the two bearings
 * combined with the distance might not always be a possible solution for any
 * two points on the geoid.
 *
 * @param _bearing1 The bearing for the direction.
 * @param _distance The distance to travel in the current bearing.
 * @param _bearing2 The reversed bearing for the direction.
 */
vdirection::vdirection( double _bearing1, double _distance, double _bearing2 )
    : bearing1(_bearing1), distance(_distance), bearing2(_bearing2)
{
}


// Operators for vdirection.
bool operator==( const vdirection& lhs, const vdirection& rhs )
{
  if ( ulpcmp(lhs.bearing1, rhs.bearing1) &&
       ulpcmp(lhs.distance, rhs.distance) ) {
    return true;
  } else {
    return false;
  }
}

} // namespace end
