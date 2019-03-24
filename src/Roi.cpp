/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      Roi.cpp
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

#include <Windows.h>
#include <gl/GL.h>
#include <QtCore/qmath.h>

#include "Roi.hpp"
#include "Geometry.hpp"
#include "VoronoiObject.hpp"

Vec2mf * Roi::CIRCLE_POINTS = NULL;
int Roi::NB_CIRCLE_POINTS = 41;

void Roi::createUnitCircle()
{
	Roi::CIRCLE_POINTS = new Vec2mf[Roi::NB_CIRCLE_POINTS];
	int cpt = 0;
	for (double t = 0; t < 2 * M_PI; t += M_PI / 20, cpt++){
		double x = 0 + 1. * cos(t) * 1. + 0. * sin(t) * 1.;
		double y = 0 + 0. * cos(t) * 1. + 1. * sin(t) * 1.;
		Roi::CIRCLE_POINTS[cpt].set(x, y);
	}
	double x = 0 + 1. * cos(0.) * 1. + 0. * sin(0.) * 1.;
	double y = 0 + 0. * cos(0.) * 1. + 1. * sin(0.) * 1.;
	Roi::CIRCLE_POINTS[cpt].set(x, y);
}

void Roi::destroyUnitCircle()
{
	delete[] Roi::CIRCLE_POINTS;
}

void Roi::drawUnitCircle()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, Roi::CIRCLE_POINTS);
	glDrawArrays(GL_LINE_STRIP, 0, Roi::NB_CIRCLE_POINTS);
	glDisableClientState(GL_VERTEX_ARRAY);
}

Roi::Roi()
{

}

Roi::~Roi()
{

}

const bool Roi::inside( const double _x, const double _y ) const
{
	double epsilon = 0.00001;
	for( std::vector < Vec2md >::const_iterator it = this->begin(); it != this->end(); it++ ){
		double d = Geometry::distance( _x, _y, it->x(), it->y() );
		if( d < epsilon )
			return true;
	}

	int cn = 0;// the  crossing number counter

	std::vector < Vec2md >::const_iterator prec = this->end();
	prec--;
	// loop through all edges of the polygon
	for( std::vector < Vec2md >::const_iterator current = this->begin(); current != this->end(); current++ ){
		if( ( ( prec->y() <= _y ) && ( current->y() > _y ) )     // an upward crossing
			|| ( ( prec->y() > _y ) && ( current->y() <=  _y ) ) ) { // a downward crossing
				// compute  the actual edge-ray intersect x-coordinate
				float vt = ( float )( _y  - prec->y() ) / ( current->y() - prec->y() );
				if( _x <  prec->x() + vt * ( current->x() - prec->x() ) ) // P.x < intersect
					++cn;   // a valid crossing of y=P.y right of P.x
		}
		prec = current;
	}
	return (cn&1) == 1;    // 0 if even (out), and 1 if  odd (in)
}

const bool Roi::intersect( VoronoiCluster * _cluster ) const
{
	bool pointInside = false;
	for( std::vector < Vec2md >::const_iterator it = _cluster->m_outlines.begin(); it != _cluster->m_outlines.end() && !pointInside; it++ ){
		pointInside = this->inside( it->x(), it->y() );
	}
	return pointInside;
}
