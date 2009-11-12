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

#ifndef __coordinate_grid_h__
#define __coordinate_grid_h__

#include "vincenty.h"

#include <vector>
#include <iostream>

typedef std::vector< vincenty::vposition > coord_vector;
typedef std::vector< coord_vector > coord_grid;

/*!
 * @brief Class for handling a grid of coordinates.
 *
 * <Details here>
 *
 * @todo Consider implementing a rotation method, or at least a construtor
 * which takes rotation as an argument (because rotating the grid means
 * recomputing it).
 */
class CoordinateGrid
{
  public:
   //! Default constructor.
   CoordinateGrid();

   /*! Constructor with two-point-rectangle initialization.
    * @param southwest_position South west position of the rectangle.
    * @param northeast_position North east position of the rectangle.
    */
	CoordinateGrid( const vincenty::vposition& southwest_position,
						 const vincenty::vposition& northeast_position );
   
   /*! Constructor with four-point-rectangle initialization.
    *
    * Allows for creating a grid of any four-side shape, even a triangle will
    * be possible. (In these cases the meaning of grid distance might became
    * undefined, since there will be different grid distances depending on the
    * grid position).
    *
    * @param southwest_position South west position of the rectangle.
    * @param northwest_position North west position of the rectangle.
    * @param northeast_position North east position of the rectangle.
    * @param southeast_position South east position of the rectangle.
    */
	CoordinateGrid( const vincenty::vposition& southwest_position,
                   const vincenty::vposition& northwest_position,
						 const vincenty::vposition& northeast_position,
                   const vincenty::vposition& southeast_position );

   /*! Constructor with center and radius initialization.
    * @param geographical_center_position Center position for the grid.
    * @param radius Distance from center, rectangle side will become 2*radius.
    * @param virtual_grid_size Virtual (interpolated) grid size.
    */
   CoordinateGrid( const vincenty::vposition& geographical_center_position,
                   const double radius,
                   const unsigned int virtual_grid_size = 5 );

   /*! Copy constructor
    * @param initializer CoordinateGrid for initialization.
    */
   CoordinateGrid( const CoordinateGrid& initializer );

   virtual ~CoordinateGrid();

   /*! Set the virtual grid size.
    * @param virtual_grid_size Virtual grid size to set.
    * @return Reference to self.
    */
   CoordinateGrid& setVirtualGridSize( const unsigned int virtual_grid_size );


   //! @return The virtual grid size.
   unsigned int getVirtualGridSize() const;

   //! @return The real grid size.
   unsigned int getRealGridSize() const;

   //! @return The grid distance between real positions.
   double getGridDistance() const;

   //! @return The grid distance between real positions.
   double getGridRadius() const;


   /*! Copies (extracts) a row from the grid and returns it as a new grid.
    * @param row_index Index of the row to copy out.
    * @return A coord_vector containing all values.
    */
   coord_vector copyRow( unsigned int row_index ) const;

   /*! Copies (extracts) a column from the grid and returns it as a new grid.
    * @param col_index Index of the column to copy out.
    * @return A coord_vector containing all values.
    */
   coord_vector copyCol( unsigned int col_index ) const;
   
   /*! Splits real size edges of the grid in half.
    * @return Reference to self.
    */
   CoordinateGrid& split();
   
   /*! Splits real size edges a specific number of times.
    * @param number_of_splits Number of splits to perform.
    * @return Reference to self.
    */
   CoordinateGrid& split( const unsigned int number_of_splits );

   /*! Splits real size edges until a specific grid distance is achived.
    * @param maximum_grid_point_distance Maximum distance between points.
    * @return Reference to self.
    */
   CoordinateGrid& splitUntil( const unsigned int maximum_grid_point_distance );

   /*! Joins two real size edges of the grid (opposite to splitting).
    * @return Reference to self.
    */
   CoordinateGrid& join();
   
   /*! Joins two real size edges a specific number of times.
    * @param number_of_joins Number of joins to perform.
    * @return Reference to self.
    */
   CoordinateGrid& join( const unsigned int number_of_joins );

   /*! Joins two real size edges until a minium grid point distance is achived.
    * @param minimum_grid_point_distance Minimum distance between points.
    * @return Reference to self.
    */
   CoordinateGrid& joinUtil( const unsigned int minimum_grid_point_distance );

   //! Operator for accessing a grid point within the virtual grid.
	vincenty::vposition operator()( unsigned int i, unsigned int j ) const;

   //! Operator for printing a grid to a stream.
   friend std::ostream& operator<<( std::ostream& os, CoordinateGrid& rhs );

  private:

   /*! Constructor with vector initialization.
    * @param init_coord_vector Initializes the grid, with a "line".
    */
   CoordinateGrid( const coord_vector& init_coord_vector );
   
   /*! Constructor with grid initialization.
    * @param init_coord_grid Initializes the grid, with a "grid".
    */
   CoordinateGrid( const coord_grid& init_coord_grid );

   //! Initializes the grid with a default 9 point setup from center.
   void _initialize_from_center();

   //! Initializes the grid with a default 9 point setup from corners.
   void _initialize_from_corners();
   
   //! Real implementation for splitting.
   void _split();

   //! Real implementation for joining.
   void _join();
   
   //! Zeropads a coordinate grid vector-vector structure.
   void _padcopy( coord_grid& dst, const coord_grid& src );

   //! The grid container (vectors of vectors).
   coord_grid _grid;

   /*!
    * Grid center position.
    *
    * @details The grid center is a const and cannot be changed for a
    * CoordinateGrid instance. Changing the center has no real meaning since
    * the grid needs to be fully re-computed if the center is moved. A new grid
    * has to be created. There is also no real memory benefit of reusing an old
    * instance. In contrary, a user might be tempted to constantly move the
    * same instance back and forth instead of computing a fixed number of
    * instances at start. Delete then allocate again!
    */
   const vincenty::vposition _center;

   /*!
    * Grid radius, or effectively a measure of the grid coverage area.
    *
    * @details The radius (same as _center) is const and cannot be changed for
    * a CoordinateGrid instance. Changing the size of the grid is not entierly
    * pointless as with moving it. But the implementation is to
    * boring/difficult/complicated to implement (at the moment). The complexity
    * is the major issue, keeping the code simple and neat is important. A bad
    * implementation could just as well become slower then creating a new
    * instance. Providing such a method to the user is to practice deceit
    */
   const double _radius;

   /*!
    * The virtual grid size, oppose to the real one with allocted positions.
    *
    * @details The virtual grid size can be changed at any time without any
    * performance penelty. The change only has an effect when accessing the
    * grid vid the operator() or extracting data in any other way.
    */
   unsigned int _virtual_grid_size;

   /*!
    * Distance between real positions in the grid.
    *
    * @details This attribute changes when a join or split method is
    * called. (In _initialize_grid() it will become identical to the radius
    * after the 9 points in the grid have been computed).
    */
   double _grid_distance;
};

#endif
