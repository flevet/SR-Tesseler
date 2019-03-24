#ifndef Geometry_h__
#define Geometry_h__

/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      Geometry.hpp
*
* Copyright: Florian Levet (2010-2019)
*
* License:   GPL v3
*
* Homepage:  http://www.iins.u-bordeaux.fr/team-sibarita-SR-Tesseler
*
*
* SR-Tesseler is a free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version, provided that this entire notice
* is included in all copies of any software which is or includes a copy
* or modification of this software and in all copies of the supporting
* documentation for such software.
*
* The algorithms that underlie SR-Tesseler have required considerable
* development. They are described in the original SR-Tesseler paper,
* doi:10.1038/nmeth.3579. If you use SR-Tesseler as part of work towards a
* scientific publication, please include a citation to the original paper.
*
* SR-Tesseler is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/

#include "ObjectInterface.hpp"
#include "Vec2.hpp"

class Geometry{
public:
	static double getTriangleArea( VertHandle, VertHandle, VertHandle );
	static double getTriangleArea( const Vec2md &, const Vec2md &, const Vec2md & );
	static double getTriangleArea( const double, const double, const double, const double, const double, const double );
	static double getTriangleMeanDistance( VertHandle, VertHandle, VertHandle );

	static double distance( const double, const double, const double, const double );
	static double distanceSqr( const double, const double, const double, const double );

	static void fitEllipsePCA(Vec2md *, const unsigned int, float *);
	static void fitBoundingEllipse(Vec2md *, const unsigned int, float *);

	static void circleLineIntersect(const double, const double, const double, const double, const double, const double, const double, std::vector < Vec2md > &);
	static double computeAreaCircularSegment(const double, const double, const double, const Vec2md &, const Vec2md &);
	static double computeAreaTriangle(const double, const double, const double); //Variables are the three side lengthes of the triangle
};

#endif // Geometry_h__