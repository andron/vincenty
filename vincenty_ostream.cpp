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

#include <iomanip>

namespace vincenty
{
/*!
 * @brief Implementation of ostream format manipulators affecting Positions.
 *
 * "Standard" way of implementing a format manipulator. A map is used to store
 * a os.xalloc value for the key coordinate_format which is "something". The
 * value (from xalloc) is then assigned one of the enum values which
 * represents the type of manipulator just found in the stream.
 *
 * What happens later, how the manipulator is implemented and formats the
 * stream is up to the operator<<( std::ostream& os, const Position& rhs
 * ). The operator<< just fetches the xalloc value and looks at the enum
 * value.
 */
std::ostream& format::dms(std::ostream& os)
{
  if ( _format.find("coordinate_format") == _format.end() ) {
    _format["coordinate_format"] = os.xalloc();
  }
  os.iword(_format["coordinate_format"]) = DMS;
  return os;
}

std::ostream& format::dm(std::ostream& os)
{
  if ( _format.find("coordinate_format" ) == _format.end() ) {
    _format["coordinate_format"] = os.xalloc();
  }
  os.iword(_format["coordinate_format"]) = DM;
  return os;
}

std::ostream& format::dd(std::ostream& os)
{
  if ( _format.find("coordinate_format" ) == _format.end() ) {
    _format["coordinate_format"] = os.xalloc();
  }
  os.iword(_format["coordinate_format"]) = DD;
  return os;
}


std::ostream& format::deg(std::ostream& os)
{
  if ( _format.find("direction_format" ) == _format.end() ) {
    _format["direction_format"] = os.xalloc();
  }
  os.iword(_format["direction_format"]) = DEG;
  return os;
}

std::ostream& format::rad(std::ostream& os)
{
  if ( _format.find("direction_format" ) == _format.end() ) {
    _format["direction_format"] = os.xalloc();
  }
  os.iword(_format["direction_format"]) = RAD;
  return os;
}


long format::coordtype()
{
  return _format["coordinate_format"];
}

long format::dirtype()
{
  return _format["direction_format"];
}


//! format::_format is static and initialized as "nothing".
std::map<const std::string,long> format::_format;


/*!
 * @brief Output operator for vdirections.
 *
 * Uses fixed output and a precision of 2 decimals. The format is not a
 * standardized format for printing bearings and distance. The operator is
 * merely implemented as a debug tool to verify the computations.
 *
 * The prior ostream format is saved and should disrupt any other outputs.
 */
std::ostream& operator<<( std::ostream& os, const vdirection& rhs )
{
  const std::ios_base::fmtflags fmt = os.flags();
  const std::streamsize prec = os.precision();
  os.setf( std::ios::fixed );
  os.fill(' ');
  if ( os.iword(format::dirtype()) == format::DEG ) {
    os << "{" 
       << std::setw(prec+5) << rhs.bearing1*180/M_PI << "," 
       << std::setw(prec)   << rhs.distance << "," 
       << std::setw(prec+5) << rhs.bearing2*180/M_PI << "}";
  } else {
    os << "{" 
       << std::setw(prec+3) << rhs.bearing1 << "," 
       << std::setw(prec)   << rhs.distance << "," 
       << std::setw(prec+3) << rhs.bearing2 << "}";
  }
  os.setf( fmt );
  return os;
}

/*!
 * @brief Output operator for vpositions.
 *
 * Rather complex output operator for printing a position in different
 * formats. As with the vdirection::operator<< it is implemented merely as a
 * debugging tool. The iword stored by the manipulator (formats::dms,
 * formats::dm and formats::dd) is compared to the known values of
 * coordinate_format which then decides what type of formating is done.
 *
 * Takes three types of stream manipulators:
 * @li @c format::DMS - degrees, minutes, seconds (double)
 * @li @c format::DM - degrees, minutes (double)
 * @li @c format::D - degrees (double)
 *
 * The ostream format is saved and restored so this should not disrupt any
 * other outputs.
 */
std::ostream& operator<<( std::ostream& os, const vposition& rhs )
{
  const std::ios_base::fmtflags fmt = os.flags();
  os.setf( std::ios::fixed );
  if ( os.iword(format::coordtype()) == format::DMS ) {
    os << "["
       << std::setw(2) << vposition::deg(rhs.coords.a[0]) << "°" 
       << std::setw(2) << vposition::min(rhs.coords.a[0]) << "'" 
       << std::setw(std::cout.precision()+3)
       << vposition::secf(rhs.coords.a[0]) << "\"" << ","
       << std::setw(2) << vposition::deg(rhs.coords.a[1]) << "°" 
       << std::setw(2) << vposition::min(rhs.coords.a[1]) << "'" 
       << std::setw(std::cout.precision()+3)
       << vposition::secf(rhs.coords.a[1]) << "\"]";
  } else if ( os.iword(format::coordtype()) == format::DM ) {
    os << "["
       << std::setw(2) << vposition::deg(rhs.coords.a[0]) << "°" 
       << std::setw(std::cout.precision()+3)
       << vposition::minf(rhs.coords.a[0]) << "'" << ","
       << std::setw(2) << vposition::deg(rhs.coords.a[1]) << "°" 
       << std::setw(std::cout.precision()+3)
       << vposition::minf(rhs.coords.a[1]) << "']";
  } else if ( os.iword(format::coordtype()) == format::DD ) {
    os << "[" 
       << std::setw(std::cout.precision()+3)
       << vposition::degf(rhs.coords.a[0]) << ","
       << std::setw(std::cout.precision()+3)
       << vposition::degf(rhs.coords.a[1]) << "]";
  } else {
    os << "[" << rhs.coords.a[0] << "," << rhs.coords.a[1] << "]";
  }
  os.setf( fmt );
  return os;
}

} // namespace end
