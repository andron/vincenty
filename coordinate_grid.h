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

namespace coordinate {

typedef std::vector< vincenty::vposition > coord_vector;
typedef std::vector< coord_vector > coord_grid;
typedef std::vector< unsigned int[2] > index_vector;

/*!
 * @brief Class for handling a grid of coordinates.
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
   *
   * South west (lower left) and north east (upper right) point
   * initialization.
   */
  CoordinateGrid(
      //! South west position of the rectangle.
      const vincenty::vposition& southwest_position,
      //! North east position of the rectangle.
      const vincenty::vposition& northeast_position );
   
  /*! Constructor with four-point-rectangle initialization.
   *
   * Allows for creating a grid of any four-side shape, even a triangle will
   * be possible. (In these cases the meaning of grid distance becomes
   * undefined, since there will be different grid distances depending on the
   * grid position).
   */
  CoordinateGrid( 
      //! South west position of the rectangle.
      const vincenty::vposition& southwest_position,
      //! North west position of the rectangle.
      const vincenty::vposition& northwest_position,
      //! North east position of the rectangle.
      const vincenty::vposition& northeast_position,
      //! South east position of the rectangle.
      const vincenty::vposition& southeast_position );

  /*! Constructor with center and radius initialization.
   */
  CoordinateGrid(
      //! Center position for the grid.
      const vincenty::vposition& geographical_center_position,
      //! Distance from center, rectangle side will become 2*radius.
      const double radius,
      //! Virtual (interpolated) grid size.
      const unsigned int virtual_grid_size = 5 );

  /*! Copy constructor
   */
  CoordinateGrid(
      //! CoordinateGrid for initialization.
      const CoordinateGrid& initializer );

  virtual ~CoordinateGrid();

  /*! Set the virtual grid size.
   * @return Reference to self.
   */
  CoordinateGrid& setVirtualGridSize(
      //! Virtual grid size to set.
      const unsigned int virtual_grid_size );


  //! @return The virtual grid size.
  unsigned int getVirtualGridSize() const;

  //! @return The virtual grid distance.
  double getVirtualGridDistance() const;


  //! @return The real grid size.
  unsigned int getGridSize() const;

  //! @return The grid distance between real positions.
  double getGridDistance() const;


  //! @return The grid distance between real positions.
  double getGridRadius() const;


  /*! Copies (extracts) a row from the grid and returns it as a new grid.
   * @return A coord_vector containing all values.
   */
  coord_vector copyRow(
      //! Index of the row to copy out.
      unsigned int row_index ) const;

  /*! Copies (extracts) a column from the grid and returns it as a new grid.
   * @return A coord_vector containing all values.
   */
  coord_vector copyCol(
      //! Index of the column to copy out.
      unsigned int col_index ) const;
   
  /*! Splits real size edges of the grid in half.
   * @return Reference to self.
   */
  CoordinateGrid& split();
   
  /*! Splits real size edges a specific number of times.
   * @return Reference to self.
   */
  CoordinateGrid& split(
      //! Number of splits to perform.
      const unsigned int number_of_splits );

  /*! Splits real size edges until a specific grid distance is achived.
   * @return Reference to self.
   */
  CoordinateGrid& splitUntil(
      //! Maximum distance between points.
      const unsigned int maximum_grid_point_distance );

  /*! Joins two real size edges of the grid (opposite to splitting).
   * @return Reference to self.
   */
  CoordinateGrid& join();
   
  /*! Joins two real size edges a specific number of times.
   * @return Reference to self.
   */
  CoordinateGrid& join(
      //! Number of joins to perform.
      const unsigned int number_of_joins );

  /*! Joins two real size edges until a minium grid point distance is achived.
   * @return Reference to self.
   */
  CoordinateGrid& joinUtil(
      //!  Minimum distance between points.
      const unsigned int minimum_grid_point_distance );

  //! Operator for accessing a grid index within the virtual grid.
	vincenty::vposition operator()(
      //! Row counted from up down.
      unsigned int i,
      //! Column from left to right.
      unsigned int j ) const;

  //! Gets positions via a vector of indexes.
  coord_vector getPositions(
      //! A vector containing indexes as an array[2].
      index_vector ) const;
  
  //! Operator for accessing a vector of indexes.
  //vincenty::vposition_vector operator(std::vector<int,int>);

  //! Operator for printing a grid to a stream.
  friend std::ostream& operator<<( std::ostream& os, CoordinateGrid& rhs );


  // Real position accessors.
  vincenty::vposition getCenter() const;

  vincenty::vposition getNW() const;
  vincenty::vposition getNE() const;
  vincenty::vposition getSW() const;
  vincenty::vposition getSE() const;

  vincenty::vposition getUL() const;
  vincenty::vposition getUR() const;
  vincenty::vposition getDL() const;
  vincenty::vposition getDR() const;
  // ----------------------------------------------------------------------

 private:

  /*! Constructor with vector initialization.
   */
  CoordinateGrid(
      //! Initializes the grid, with a "line".
      const coord_vector& init_coord_vector );
   
  /*! Constructor with grid initialization.
   */
  CoordinateGrid(
      //! Initializes the grid, with a "grid".
      const coord_grid& init_coord_grid );

  //! Initializes n.e.w.s from center.
  void _initialize_news_from_center();

  //! Initializes corners from center.
  void _initialize_corners_from_center();

  //! Initializes center from corners.
  void _initialize_center_from_corners();

  //! Real implementation for splitting.
  void _split();

  //! Real implementation for joining.
  void _join();

  //! Zeropads a coordinate grid vector-vector structure.
  void _padcopy( coord_grid& dst, const coord_grid& src );


  //! The grid container (vectors of vectors).
  coord_grid _grid;

  /*!
   * The virtual grid size, oppose to the real one with allocted positions.
   *
   * @details The virtual grid size can be changed at any time without any
   * performance penelty. The change only has an effect when accessing the
   * grid vid the ()-operator or extracting data in any other way.
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

  /*!
   * Distance between virtual posistions in the grid.
   *
   * The attribute changes when the virtual grid size changes.
   */
  double _virtual_grid_distance;
};
  
}

#endif
