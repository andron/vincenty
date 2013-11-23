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

#include "vincenty/vincenty.h"

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

//! Accessor for latitude component.
double vposition::latitude() const
{
  return coords.a[0];
}

//! Accessor for longitude component.
double vposition::longitude() const
{
  return coords.a[1];
}

//! Integer degrees from float radian.
int vposition::deg( const double rad )
{
  return int( degf(rad) );
}

//! Extracts integer minutes from float radian.
int vposition::min( const double rad )
{
  return int( minf(rad) );
}

//! Extracts integer seconds from float radian.
int vposition::sec( const double rad )
{
  return int( secf(rad) );
}


//! Converts radians to degrees.
double vposition::degf( const double rad )
{
  return to_deg(rad);
}

//! Extracts decimal part minutes from float radian.
double vposition::minf( const double rad )
{
  float const deg = to_deg(rad);
  return ( deg - floor(deg) ) * 60.0;
}

//! Extracts decimal part seconds from float radian.
double vposition::secf( const double rad )
{
  return ( minf(rad) - min(rad) ) * 60.0;
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

vposition vposition::operator+( const vdirection& rhs ) const
{
  return direct((*this),rhs);
}

vposition vposition::operator-( const vdirection& rhs ) const
{
  return direct((*this),rhs.bearing2,rhs.distance);
}

vdirection vposition::operator-( const vposition& rhs ) const
{
  return inverse(rhs,(*this));
}

vposition vposition::operator^( const vposition& rhs ) const
{
  vdirection d = inverse(rhs,(*this));
  return direct((*this),d.bearing2,d.distance/2.0);
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

vdirection vdirection::operator/( const double rhs ) const
{
  return vdirection((*this).bearing1,(*this).distance/rhs);
}

vdirection vdirection::operator*( const double rhs ) const
{
  return vdirection((*this).bearing1,(*this).distance*rhs);
}

} // namespace end
