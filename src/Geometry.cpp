/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      Geometry.cpp
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

#include <qmath.h>

#include "Geometry.hpp"

double Geometry::getTriangleArea( VertHandle _v1, VertHandle _v2, VertHandle _v3 )
{
	double x1 = _v1->point().x(), y1 = _v1->point().y(), x2 = _v2->point().x(), y2 = _v2->point().y(), x3 = _v3->point().x(), y3 = _v3->point().y();
	double area = ( ( x2 * y1  ) - ( x1 * y2 ) ) + ( ( x3 * y2  ) - ( x2 * y3 ) ) + ( ( x1 * y3  ) - ( x3 * y1 ) );
	return abs( area / 2. );
}

double Geometry::getTriangleArea( const Vec2md & _v1, const Vec2md & _v2, const Vec2md & _v3 )
{
	double x1 = _v1.x(), y1 = _v1.y(), x2 = _v2.x(), y2 = _v2.y(), x3 = _v3.x(), y3 = _v3.y();
	double area = ( ( x2 * y1  ) - ( x1 * y2 ) ) + ( ( x3 * y2  ) - ( x2 * y3 ) ) + ( ( x1 * y3  ) - ( x3 * y1 ) );
	return abs( area / 2. );
}

double Geometry::getTriangleArea( const double _x1, const double _y1, const double _x2, const double _y2, const double _x3, const double _y3 )
{
	double area = ( ( _x2 * _y1  ) - ( _x1 * _y2 ) ) + ( ( _x3 * _y2  ) - ( _x2 * _y3 ) ) + ( ( _x1 * _y3  ) - ( _x3 * _y1 ) );
	return abs( area / 2. );
}

double Geometry::getTriangleMeanDistance( VertHandle _v1, VertHandle _v2, VertHandle _v3 )
{
	double x1 = _v1->point().x(), y1 = _v1->point().y(), x2 = _v2->point().x(), y2 = _v2->point().y(), x3 = _v3->point().x(), y3 = _v3->point().y();
	double d1 = sqrt( ( x2 - x1 ) * ( x2 - x1 ) + ( y2 - y1 ) * ( y2 - y1 ) );
	double d2 = sqrt( ( x3 - x2 ) * ( x3 - x2 ) + ( y3 - y2 ) * ( y3 - y2 ) );
	double d3 = sqrt( ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) );
	return ( d1 + d2 + d3 ) / 3.;
}

double Geometry::distance( const double _x0, const double _y0, const double _x1, const double _y1 )
{
	return sqrt( distanceSqr( _x0, _y0, _x1, _y1 ) );
}

double Geometry::distanceSqr( const double _x0, const double _y0, const double _x1, const double _y1 )
{
	double x = _x1 - _x0, y = _y1 - _y0;
	return x*x + y*y;
}

void Geometry::fitEllipsePCA(Vec2md * _points, const unsigned int _nbPoints, float * _ptr)
{
	if (_nbPoints <= 1) return;

	double xc, yc, angle, axisX, axisY, diameter = 0., circ = 0., slope;
	Vec2md p0, p1;
	std::vector < KernelPoint > points;
	for (int n = 0; n < _nbPoints; n++)
		points.push_back(KernelPoint(_points[n].x(), _points[n].y()));
	KernelPoint centroid = CGAL::centroid(points.begin(), points.end(), CGAL::Dimension_tag < 0 >());
	KernelLine majorAxis;
	CGAL::linear_least_squares_fitting_2(points.begin(), points.end(), majorAxis, CGAL::Dimension_tag < 0 >());
	slope = -(majorAxis.a() / majorAxis.b());
	KernelLine minorAxis = majorAxis.perpendicular(centroid);
	double * distanceToMajorAxis = new double[points.size()];
	double * distanceToMinorAxis = new double[points.size()];
	double meanDMajor = 0., meanDMinor = 0., nb = points.size();
	p0.set(centroid.x(), centroid.y());
	p1.set(centroid.x(), centroid.y());
	double d0 = 0., d1 = 0., dpositive = 0., dnegative = 0.;
	for (int n = 0; n < points.size(); n++){
		KernelPoint proj = majorAxis.projection(points[n]);
		distanceToMajorAxis[n] = sqrt(CGAL::squared_distance(proj, points[n]));
		if (majorAxis.has_on_positive_side(points[n])){
			if (distanceToMajorAxis[n] > dpositive)
				dpositive = distanceToMajorAxis[n];
		}
		else{
			if (distanceToMajorAxis[n] > dnegative)
				dnegative = distanceToMajorAxis[n];
		}
		meanDMajor += (distanceToMajorAxis[n] / nb);
		if (minorAxis.has_on_positive_side(proj)){
			double d = sqrt(CGAL::squared_distance(proj, centroid));
			if (d > d0){
				p0.set(proj.x(), proj.y());
				d0 = d;
			}
		}
		else{
			double d = sqrt(CGAL::squared_distance(proj, centroid));
			if (d > d1){
				p1.set(proj.x(), proj.y());
				d1 = d;
			}
		}
		proj = minorAxis.projection(points[n]);
		distanceToMinorAxis[n] = sqrt(CGAL::squared_distance(proj, points[n]));
		meanDMinor += (distanceToMinorAxis[n] / nb);
	}
	ArrayStatistics statsMajor = GeneralTools::generateArrayStatistics(distanceToMajorAxis, points.size());
	ArrayStatistics statsMinor = GeneralTools::generateArrayStatistics(distanceToMinorAxis, points.size());
	Vec2mf v1 = p1 - p0, v2(1.f, 0.f);
	v1.normalize();
	float dot = v1.dot(v2);
	angle = acos(dot);
	angle = (angle * (180 / M_PI));
	KernelLine axis(KernelPoint(0., 0.), KernelPoint(1., 0.));
	if (axis.has_on_negative_side(KernelPoint(v1.x(), v1.y())))
		angle = 180. + 180 - angle;

	xc = centroid.x();
	yc = centroid.y();

	double dForRapport = (dpositive > dnegative) ? dnegative : dpositive;
	double rapport = 2.35; //( dForRapport > 2.35 * statsMajor.m_stdDev ) ? dForRapport / statsMajor.m_stdDev : 2.35;
	axisY = rapport * statsMajor.m_stdDev; //( statsMajor.m_mean + statsMajor.m_stdDev );
	axisX = rapport * statsMinor.m_stdDev; //( statsMinor.m_mean + statsMinor.m_stdDev );
	double major, minor;
	if (axisX > 1000 || axisY > 1000)
		diameter = circ = 0.;
	else{
		if (axisX > axisY){
			major = axisX;
			minor = axisY;
		}
		else{
			major = axisY;
			minor = axisX;
		}
		circ = minor / major;
		diameter = ((minor + major) / 2.) * 2.;
	}

	_ptr[0] = (float)xc;
	_ptr[1] = (float)yc;
	_ptr[2] = (float)angle;
	_ptr[3] = (float)axisX;
	_ptr[4] = (float)axisY;
	_ptr[5] = (float)circ;
	double factorAxis = 2.;
	_ptr[6] = (float)(major * factorAxis);
	_ptr[7] = (float)(minor * factorAxis);

	delete[] distanceToMinorAxis;
	delete[] distanceToMajorAxis;
}

void Geometry::fitBoundingEllipse(Vec2md * _points, const unsigned int _nbPoints, float * _ptr)
{
	CartesianPoint * pts = new CartesianPoint[_nbPoints];
	for (int n = 0; n < _nbPoints; n++)
		pts[n] = CartesianPoint(_points[n].x(), _points[n].y());
	Min_ellipse me2(pts, pts + _nbPoints, true);
	double A, B, C, D, E, F;
	me2.ellipse().double_coefficients(A, C, B, D, E, F);
	double a = A, b = B / 2., c = C, d = D / 2., f = E / 2., g = F;
	double xc = (c*d - b*f) / (b*b - a*c);
	double yc = (a*f - b*d) / (b*b - a*c);
	double phi = 0.;
	if (b == 0 && a < c)
		phi = 0.;
	else if (b == 0 && a > c)
		phi = 0.5 * M_PI;
	else if (b != 0 && a < c){
		double tmp = (a - c) / (2 * b);
		phi = 0.5 * atan(1. / tmp);
	}
	else if (b != 0 && a > c){
		double tmp = (a - c) / (2 * b);
		phi = (M_PI / 2.) + .5 * atan(1. / tmp);
	}
	double angle = (phi * (180 / M_PI));
	double nom = 2. * (a * f * f + c * d * d + g * b * b - 2 * b * d * f - a * c * g);
	double denom = (b * b - a * c) * (sqrt((a - c)*(a - c) + 4 * b*b) - (a + c));
	double axisX = sqrt(nom / denom);
	denom = (b * b - a * c) * (-sqrt((a - c)*(a - c) + 4 * b*b) - (a + c));
	double axisY = sqrt(nom / denom);
	double major, minor, circ, diameter;
	if (axisX > 1000 || axisY > 1000)
		diameter = circ = 0.;
	else{
		if (axisX > axisY){
			major = axisX;
			minor = axisY;
		}
		else{
			major = axisY;
			minor = axisX;
		}
		circ = minor / major;
		diameter = ((minor + major) / 2.) * 2.;
	}

	_ptr[0] = (float)xc;
	_ptr[1] = (float)yc;
	_ptr[2] = (float)angle;
	_ptr[3] = (float)axisX;
	_ptr[4] = (float)axisY;
	_ptr[5] = (float)circ;
	_ptr[6] = (float)(major * 2.);
	_ptr[7] = (float)(minor * 2.);
	//std::cout << axisX << ", " << axisY << ", " << m_data[Diameter] << std::endl;
}

void Geometry::circleLineIntersect(const double _x1, const double _y1, const double _x2, const double _y2, const double _cx, const double _cy, const double _cr, std::vector < Vec2md > & _points)
{
	double dx = _x2 - _x1;
	double dy = _y2 - _y1;
	double a = dx * dx + dy * dy;
	double b = 2 * (dx * (_x1 - _cx) + dy * (_y1 - _cy));
	double c = _cx * _cx + _cy * _cy;
	c += _x1 * _x1 + _y1 * _y1;
	c -= 2 * (_cx * _x1 + _cy * _y1);
	c -= _cr * _cr;
	double bb4ac = b * b - 4 * a * c;

	//println(bb4ac);

	if (bb4ac < 0) {  // Not intersecting
		return;
	}
	else {

		double mu = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
		double ix1 = _x1 + mu * (dx);
		double iy1 = _y1 + mu * (dy);
		mu = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
		double ix2 = _x1 + mu * (dx);
		double iy2 = _y1 + mu * (dy);

		/*double t1 = -1., t2 = -1.;
		if( dx != 0. ){
		t1 = ( ix1 - _x1 ) / dx;
		t2 = ( ix2 - _x1 ) / dx;
		}
		if( dy != 0. ){
		t1 = ( iy1 - _y1 ) / dy;
		t2 = ( iy2 - _y1 ) / dy;
		}
		if( 0 <= t1 && t1 <= 1. )
		_points.push_back( Vec2md( ix1, iy1 ) );
		if( 0 <= t2 && t2 <= 1. )
		_points.push_back( Vec2md( ix2, iy2 ) );*/
		if (_x1 <= ix1 && ix1 <= _x2 && _y1 <= iy1 && iy1 <= _y2)
			_points.push_back(Vec2md(ix1, iy1));
		if (_x1 <= ix2 && ix2 <= _x2 && _y1 <= iy2 && iy2 <= _y2)
			_points.push_back(Vec2md(ix2, iy2));
	}
}

double Geometry::computeAreaCircularSegment(const double _cx, const double _cy, const double _r, const Vec2md & _p1, const Vec2md & _p2){
	double x = (_p1.x() + _p2.x()) / 2., y = (_p1.y() + _p2.y()) / 2.;
	double smallR = Geometry::distance(x, y, _cx, _cy);
	double h = _r - smallR;

	double area = (_r * _r) * acos((_r - h) / _r) - ((_r - h) * sqrt((2. * _r * h) - (h * h)));
	return area;
}

//Computed using the Heron's formula
double Geometry::computeAreaTriangle(const double _a, const double _b, const double _c)
{
	double semiPerimeter = (_a + _b + _c) / 2.;
	return sqrt(semiPerimeter * (semiPerimeter - _a) * (semiPerimeter - _b) * (semiPerimeter - _c));
}