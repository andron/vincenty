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

#include "vincenty/coordinate_grid.h"

#include "vincenty/vincenty.h"

using namespace vincenty;

// Hidden anonymous namespace to hide symbols which shall not be published
// outside the library.
#pragma GCC visibility push(hidden)
namespace {

const double sqrt2 = sqrt(2.0);

/**
 * @addtogroup cg_helpers Helper functions in CoordinateGrid.
 * 
 * Helper functions for CoordinateGrid which should not be part of the class
 * itself and not part of the interface.
 */

/**@{*/

/**
 * Standard 2D interpolation.
 */
double __attribute__((unused))
interpolate2d( const double A,
               const double B,
               const double C,
               const double D,
               const double dx,
               const double dy )
{
  return 
      A * ( 1 - dx ) * ( 1 - dy ) +
      B * (     dx ) * ( 1 - dy ) +
      C * ( 1 - dx ) * (     dy ) +
      D * (     dx ) * (     dy );
}

/**
 * Standard 2D interpolation for vposition.
 *
 * Longitude and latitude values are interpolated separately.
 */
void
interpolate_position( const vposition& A,
                      const vposition& B,
                      const vposition& C,
                      const vposition& D,
                      const double dx,
                      const double dy,
                      vposition& dest )
{
  /** 
   * If we have a GCC-compiler version 4 or later and optimization is turned
   * on two vector operands are used to speed things up. (It is not proven
   * that this is faster then the simpler for-loop used in any other case. But
   * doing it this way is much cooler, and should be faster).
   */
#if __GNUC__ > 3 && __OPTIMIZE__
  // Initialize two vector operands
  v2df_u vdx, _vdx;
  vdx.a[0] = dx;
  vdx.a[1] = dx;
  _vdx.a[0] = 1-dx;
  _vdx.a[1] = 1-dx;

  v2df_u vdy, _vdy;
  vdy.a[0] = dy;
  vdy.a[1] = dy;
  _vdy.a[0] = 1-dy;
  _vdy.a[1] = 1-dy;

  // Compute both values at the same time using MMX or SSE instructions or
  // whatever the compiler can do on the current arch.
  dest.coords.v =
      A.coords.v * ( _vdx.v ) * ( _vdy.v ) +
      B.coords.v * (  vdx.v ) * ( _vdy.v ) +
      C.coords.v * ( _vdx.v ) * (  vdy.v ) +
      D.coords.v * (  vdx.v ) * (  vdy.v );
#else
  for ( int i=0; i<2; ++i ) {
    dest.coords.a[i] =
        A.coords.a[i] * ( 1 - dx ) * ( 1 - dy ) +
        B.coords.a[i] * (     dx ) * ( 1 - dy ) +
        C.coords.a[i] * ( 1 - dx ) * (     dy ) +
        D.coords.a[i] * (     dx ) * (     dy );
  }
#endif
}

}
#pragma GCC visibility pop

/**@}*/

namespace coordinate {

// I am lazy
#define DEFAULT_INIT                            \
  _grid(),                                      \
    _virtual_grid_size(0),                      \
    _grid_distance(0),                          \
    _virtual_grid_distance(0)

CoordinateGrid::CoordinateGrid()
    : DEFAULT_INIT
{
}


CoordinateGrid::CoordinateGrid( const vincenty::vposition& southwest_position,
                                const vincenty::vposition& northeast_position )
    : DEFAULT_INIT
{
  coord_vector cv(3,vposition(0,0));
  _grid = coord_grid(3,cv);

  // Set the two corners.
  _grid[2][0] = southwest_position;
  _grid[0][2] = northeast_position;

  // Compute middle point and use as center. Starting position is the south
  // west position.
  const vdirection a = inverse(_grid[2][0],_grid[0][2]);
  _grid[1][1] = direct(_grid[2][0],a.bearing1,a.distance/2.0);

  // To compute the south east and north west positions we use the newly
  // computed center position as starting point, and go in respective
  // direction, half the distance between the user given corners.
  _grid[0][0] = direct(_grid[1][1],direction::northwest,a.distance/2);
  _grid[2][2] = direct(_grid[1][1],direction::southeast,a.distance/2);

  // Compute _grid_distance to use as base distance for N,E,W,S.
  _grid_distance = a.distance / ( 2*sqrt2 );

  // Initialize N,E,W,S from the center position.
  _initialize_news_from_center();
}


CoordinateGrid::CoordinateGrid( const vincenty::vposition& southwest_position,
                                const vincenty::vposition& northwest_position,
                                const vincenty::vposition& northeast_position,
                                const vincenty::vposition& southeast_position )
    : DEFAULT_INIT
{
  coord_vector cv(3,vposition(0,0));
  _grid = coord_grid(3,cv);

  // Setup the four corners.
  _grid[2][0] = southwest_position;
  _grid[0][0] = northwest_position;
  _grid[0][2] = northeast_position;
  _grid[2][2] = southeast_position;

  _initialize_center_from_corners();
  _initialize_news_from_center();
}


CoordinateGrid::CoordinateGrid( const vposition& center,
                                const double radius,
                                const unsigned int virtual_grid_size )
    : _grid(),
      _virtual_grid_size(virtual_grid_size),
      _grid_distance(radius),
      _virtual_grid_distance(0)
{
  coord_vector cv(3,vposition(0,0));
  _grid = coord_grid(3,cv);

  _grid[1][1] = center;
  _virtual_grid_distance = 2*radius/_virtual_grid_size;
  _initialize_corners_from_center();
  _initialize_news_from_center();
}


CoordinateGrid::CoordinateGrid( const CoordinateGrid& vg )
    : _grid(vg._grid),
      _virtual_grid_size(vg._virtual_grid_size),
      _grid_distance(vg._grid_distance),
      _virtual_grid_distance(vg._virtual_grid_distance)
{
}


CoordinateGrid::~CoordinateGrid()
{
  // Delete the coord_grid vector-vector here if neccessary.
}



/*
 * Private
 */
CoordinateGrid::CoordinateGrid( const coord_vector& init_coord_vector )
    : DEFAULT_INIT
{
  _grid.push_back( init_coord_vector );
}

CoordinateGrid::CoordinateGrid( const coord_grid& init_coord_grid )
    : DEFAULT_INIT
{
  _grid = init_coord_grid;
}



std::ostream&
operator<<( std::ostream& os, CoordinateGrid& rhs )
{
  /*!
   * @todo Consider using a coord_vector ostream operator for
   * this. I.e. implement a coord_vector ostream operator and then use it
   * here. It would be nice to be able to print a coord_vector as well.
   */
  coord_grid::const_iterator it1 = rhs._grid.begin();
  for ( ; it1 != rhs._grid.end() ; ++it1 ) {
    coord_vector::const_iterator it2 = (*it1).begin();
    for ( ; it2 != (*it1).end() ; ++it2 ) {
      os << (*it2);
    }
    os << std::endl;
  }
  return os;
}


/*!
 * @details Sets the initial size of the grid to 3x3 and fills it with
 * vposition(0,0). Postion [1,1] is the center and is assigned from attribute
 * _center. The four edge positions are computed by jumping from the center
 * directly north, east, south and west by distance "_radius". The corners are
 * also computed from the center, but in north east, north west directions
 * etc. The distance is \f$r \sqrt{2}\f$.
 */
void
CoordinateGrid::_initialize_news_from_center()
{
  _grid[0][1] = direct( _grid[1][1], direction::north, _grid_distance );
  _grid[1][2] = direct( _grid[1][1], direction::east , _grid_distance );
  _grid[2][1] = direct( _grid[1][1], direction::south, _grid_distance );
  _grid[1][0] = direct( _grid[1][1], direction::west , _grid_distance );
}

void
CoordinateGrid::_initialize_corners_from_center()
{
  _grid[0][2] = direct( _grid[1][1], direction::northeast, sqrt2*_grid_distance );
  _grid[2][2] = direct( _grid[1][1], direction::southeast, sqrt2*_grid_distance );
  _grid[2][0] = direct( _grid[1][1], direction::southwest, sqrt2*_grid_distance );
  _grid[0][0] = direct( _grid[1][1], direction::northwest, sqrt2*_grid_distance );
}

void
CoordinateGrid::_initialize_center_from_corners()
{
  // Compute the center as the "average" position of the four corners. There
  // is no guarantee that the corners forms a perfect square.
  _grid[1][1] = vposition( (_grid[2][0].coords.a[0] +
                            _grid[0][2].coords.a[0] +
                            _grid[0][0].coords.a[0] +
                            _grid[2][2].coords.a[0] ) / 4.0 ,
                           (_grid[2][0].coords.a[1] +
                            _grid[0][2].coords.a[1] +
                            _grid[0][0].coords.a[1] +
                            _grid[2][2].coords.a[1] ) / 4.0 );
  
  // Compute the grid distance by averaging the distance from the center to
  // the four corners.
  const vdirection da = inverse(_grid[1][1],_grid[2][0]);
  const vdirection db = inverse(_grid[1][1],_grid[0][2]);
  const vdirection dc = inverse(_grid[1][1],_grid[0][0]);
  const vdirection dd = inverse(_grid[1][1],_grid[2][2]);

  _grid_distance = ( da.distance +
                     db.distance +
                     dc.distance +
                     dd.distance ) / (4*sqrt2);
}


CoordinateGrid&
CoordinateGrid::setVirtualGridSize( const unsigned int virtual_grid_size )
{
  _virtual_grid_size = virtual_grid_size;
  return (*this);
}

unsigned int
CoordinateGrid::getVirtualGridSize() const
{
  return _virtual_grid_size;
}

unsigned int
CoordinateGrid::getGridSize() const
{
  return _grid.size();
}

double
CoordinateGrid::getGridDistance() const
{
  return _grid_distance;
}

double
CoordinateGrid::getVirtualGridDistance() const
{
  return _virtual_grid_distance;
}


coord_vector
CoordinateGrid::copyRow( unsigned int idx ) const
{
  if ( _grid.empty() ) {
    return coord_vector();
  }
   
  if ( _grid.size() < idx ) {
    /*!
     * If the user over addresses, the last row is returned because that was
     * probably what the user wanted. (To get a f-ing row somewhere at the
     * end!).
     */
    return _grid[_grid.size()-1];
  } else {
    return _grid[idx];
  }
}

coord_vector
CoordinateGrid::copyCol( unsigned int idx ) const
{
  coord_vector cvec;
  if ( _grid.size() > 0 ) {
    if ( _grid[0].size() < idx ) {
      idx = _grid[0].size() - 1;
    }
    coord_grid::const_iterator it = _grid.begin();
    for ( ; it != _grid.end(); ++it ) {
      cvec.push_back( (*it)[idx] );
    }
  }
  return cvec;
}


CoordinateGrid&
CoordinateGrid::split()
{
  _split();
  return (*this);
}


CoordinateGrid&
CoordinateGrid::split( const unsigned int number_of_splits )
{
  for ( unsigned int i=0; i<number_of_splits; ++i ) {
    _split();
  }
  return (*this);
}


CoordinateGrid&
CoordinateGrid::splitUntil( const unsigned int maximum_grid_point_distance )
{
  while ( _grid_distance > maximum_grid_point_distance ) {
    _split();
  }
  return (*this);
}


/*!
 * @addtogroup cg_positions_accessor CoordinateGrid Positions Accessors.
 */

//!@{
vposition
CoordinateGrid::getCenter() const
{
  const unsigned int i = _grid.size();
  const unsigned int j = _grid[0].size();
  return _grid[i/2][j/2];
}

vposition
CoordinateGrid::getNW() const
{
  return _grid[0][0];
}

vposition
CoordinateGrid::getNE() const
{
  return _grid[0][_grid[0].size()-1];
}

vposition
CoordinateGrid::getSW() const
{
  return _grid[_grid.size()-1][0];
}

vposition
CoordinateGrid::getSE() const
{
  return _grid[_grid.size()-1][_grid[0].size()-1];
}

vposition
CoordinateGrid::getUL() const
{
  return getNW();
}

vposition
CoordinateGrid::getUR() const
{
  return getNE();
}

vposition
CoordinateGrid::getDL() const
{
  return getSW();
}

vposition
CoordinateGrid::getDR() const
{
  return getSE();
}
//!@{


/*!
 * @details Copies src to dst in a stretched manner.
 */
void
CoordinateGrid::_padcopy( coord_grid& dst, const coord_grid& src )
{
  // Assume the sizes are of the sort 2^n+1, as should when the grids are
  // created, then this formula should give an even number.
  const unsigned int A = (dst.size()-1) / (src.size()-1);
  for ( size_t m=0; m<src.size(); ++m ) { 
    for ( size_t n=0; n<src.size(); ++n ) { 
      dst[m*A][n*A] = src[m][n];
    }
  }
}


/*!
 * @detail The private split-method is the only split method doing any real
 * work and it is therefore slightly more complex then the others. The basic
 * principle is that the all "squares" in the grid are split in half generating
 * 4 new edges within each square.
 */

void
CoordinateGrid::_split()
{
  /*!
   * @todo Consider implementing another _split which can split to arbitrary
   * sizes on one single allocation. And processes "even" and "odd" lines at
   * the same time. (New odd lines are those in between two older ones).  Such
   * a loop, which does not visit the same position twice, will become a
   * little complex to read though. The small, not certain, speed gain might
   * not be worth such an effort.
   */

  // The new size is always twice the old one minus 1 to keep the size odd.
  // (Sizes: 2^(n+1)+1, or (x-1)*2^m+1).
  const unsigned int new_grid_size = (_grid.size() - 1) * 2 + 1;

  // The new grid is initalized with the new size and an empty coord_vector
  // (only containing positions at 0,0).
  coord_grid grid( new_grid_size, coord_vector( new_grid_size ) );

  // Variables i and j loops columns and rows repectively in the new grid,
  // which is supposed to be filled with new data.  Indexes m and n handles
  // the column indexing while u and v are for row indexing in the old
  // grid. Index m and u are the "smaller" indexes, i.e. upper most and left
  // most.
  for ( unsigned int i=0; i<new_grid_size; ++i ) {
    const unsigned int m = i/2;
    const unsigned int n = (i+1)/2;
    for ( unsigned int j=0; j<new_grid_size; ++j ) {
      const unsigned int u = j/2;
      const unsigned int v = (j+1)/2;
      if ( i%2 == 0 ) {
        if ( j%2 == 0 ) {
          // When both i and j are even we are located on an "old" point, or a
          // point corresponding to a point in the old grid. Just copy the
          // point.  grid[i][j] = grid[m][u];
          grid[i][j] = _grid[m][u];
        } else {
          // If only i is even, use the m index to get the column in the old
          // grid (both m and n will have the same value in this case) and
          // then u and v to get the row. Index u and v will have different
          // values here which is why we must find a point between them.
               
          // Create a vdirection between two old points and then use the
          // bearing from that calculation to find the point exactly between
          // the two points. The distance between grid points for the new grid
          // is the old size / 2.
          const vdirection a = inverse(_grid[m][u],_grid[m][v]);
          grid[i][j] = direct(_grid[m][u],a.bearing1,a.distance/2.0);
        }
      } else {
        if ( j%2 == 0 ) {
          // Same case as with even i-index.
          const vdirection a = inverse(_grid[m][u],_grid[n][u]);
          grid[i][j] = direct(_grid[m][u],a.bearing1,a.distance/2.0);
        } else {
          // When both index i and j are odd we have point which is not on an
          // old edge but rather in the middle of the old square. Create a
          // diagonal line and find the middle point.
          const vdirection a = inverse(_grid[m][u],_grid[n][v]);
          grid[i][j] = direct(_grid[m][u],a.bearing1,a.distance/2.0);
        }
      }
    }
  }
  // Just assume the grid distance has halfed.
  _grid_distance /= 2;
  _grid.swap( grid );
}


CoordinateGrid&
CoordinateGrid::join()
{
  _join();
  return (*this);
}

CoordinateGrid&
CoordinateGrid::join( const unsigned int number_of_joins )
{
  for ( unsigned int i=0; i<number_of_joins; ++i ) {
    _join();
  }
  return (*this);
}

CoordinateGrid&
CoordinateGrid::joinUtil( const unsigned int minimum_grid_point_distance )
{
  while ( _grid_distance < minimum_grid_point_distance ) {
    _join();
  }
  return (*this);
}


void
CoordinateGrid::_join()
{
  // Create a new grid, half the size (more or less) of the old one.
  const unsigned int new_grid_size = (_grid.size()-1)/2+1;
  coord_grid grid( new_grid_size, coord_vector(new_grid_size, vposition(0,0)) );

  // Loop and copy every other position from old to new.
  for ( unsigned int i=0; i<new_grid_size; ++i ) {
    for ( unsigned int j=0; j<new_grid_size; ++j ) {
      grid[i][j] = _grid[i*2][j*2];
    }
  }
   
  _grid_distance *= 2;
  _grid.swap( grid );
}


vposition
CoordinateGrid::operator()( unsigned int i, unsigned int j ) const
{
  // Assume we always operate on square sized grids.
  const unsigned int N = _grid.size() - 1;
  const unsigned int n = _virtual_grid_size;

  // A simple check to prevent annoying segfaults when the user addresses
  // indexes outside the grid.
  if ( i > n )
    i = n;
  if ( j > n )
    j = n;
   
  // Compute the offset in fractions from the closest corner grid position.
  // {i,j}_0 is the {upper,left} most grid position. {i,j}_1 is just the next
  // one. d{i,j} is the fractional offset from the real grid position to the
  // virtual grid position.
  const unsigned int i_0 = ( 1 + i * N ) / n;
  const unsigned int i_1 = i_0 + 1;
  const double di = double(N)/n * (0.5+i) - int(double(i*N)/n);

  const unsigned int j_0 = ( 1 + j * N ) / n;
  const unsigned int j_1 = j_0 + 1;
  const double dj = double(N)/n * (0.5+j) - int(double(j*N)/n);

  const vposition A = _grid[i_0][j_0];
  const vposition B = _grid[i_0][j_1];
  const vposition C = _grid[i_1][j_0];
  const vposition D = _grid[i_1][j_1];
   
  // Interpolate between the four corners A,B,C and D at position dj,di. The
  // result is stored in pos.
  vposition pos;
  interpolate_position( A, B, C, D, dj, di, pos );
  return pos;
}


coord_vector
CoordinateGrid::getPositions( index_vector iv ) const
{
  coord_vector cv(iv.size());
  index_vector::const_iterator it = iv.begin();
  for ( ; it != iv.end() ; ++it ) {
    cv.push_back( (*this)((*it)[0],(*it)[1]) );
  }
  return cv;
}

} // namespace
