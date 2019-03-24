/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      KRipley.cpp
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

#include <QtCore/qmath.h>
#include <fstream>

#include "KRipley.hpp"
#include "GeneralTools.hpp"
#include "Vec2.hpp"
#include "Geometry.hpp"

KRipley::KRipley(DetectionSet * _dset, const float _w, const float _h) :m_w(_w), m_h(_h)
{
	m_results = m_ks = m_ls = m_ts = NULL;

	m_dset = _dset;
	DetectionPoint * origPoints = _dset->getPoints();
	m_cloud = new KdPointCloud_D();
	m_cloud->m_pts.resize(_dset->nbPoints());
	for (unsigned int n2 = 0; n2 < _dset->nbPoints(); n2++){
		m_cloud->m_pts[n2].m_x = origPoints[n2].x();
		m_cloud->m_pts[n2].m_y = origPoints[n2].y();
	}
	m_tree = new KdTree_2D_double(2, *m_cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */));
	m_tree->buildIndex();

	m_density = (double)m_dset->nbPoints() / (double)(m_w * m_h);
}

KRipley::KRipley(DetectionSet * _dset, const double _minR, const double _maxR, const double _stepR, const float _w, const float _h, const bool _onROIs, const RoiList & _rois) : KRipley(_dset, _w, _h)
{
	/*m_results = m_ks = m_ls = m_ts = NULL;

	m_dset = _dset;
	DetectionPoint * origPoints = _dset->getPoints();
	m_cloud = new KdPointCloud_D();
	m_cloud->m_pts.resize(_dset->nbPoints());
	for (unsigned int n2 = 0; n2 < _dset->nbPoints(); n2++){
		m_cloud->m_pts[n2].m_x = origPoints[n2].x();
		m_cloud->m_pts[n2].m_y = origPoints[n2].y();
	}
	m_tree = new KdTree_2D_double(2, *m_cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10));
	m_tree->buildIndex();

	m_density = ( double )m_dset->nbPoints() / ( double )( m_w * m_h );*/

	computeKRipley( _minR, _maxR, _stepR, _onROIs, _rois );
}

KRipley::~KRipley()
{
	if( m_results != NULL )
		delete [] m_results;
	if (m_ks != NULL)
		delete[] m_ks;
	if (m_ls != NULL)
		delete[] m_ls;
	if (m_ts != NULL)
		delete[] m_ts;
}

void KRipley::computeKRipley(const double _minR, const double _maxR, const double _stepR, const bool _onROIs, const RoiList & _rois)
{
	m_minR = _minR; m_maxR = _maxR; m_stepR = _stepR;
	if( m_results != NULL )
		delete [] m_results;
	if (m_ks != NULL)
		delete[] m_ks;
	if (m_ls != NULL)
		delete[] m_ls;
	if (m_ts != NULL)
		delete[] m_ts;
	m_nbSteps = 0;
	for (double r = m_minR; r <= m_maxR; r += m_stepR, m_nbSteps++);
	//m_nbSteps = ((unsigned int)(((m_maxR - m_minR) / m_stepR) + 0.5)) + 1;
	unsigned int index = 0;
	m_results = new double[m_nbSteps];
	m_ks = new double[m_nbSteps];
	m_ls = new double[m_nbSteps];
	m_ts = new double[m_nbSteps];

	//Generation of the localization set inside the ROIs, if not onROIs selected or there is no ROIs, the whole localization set is selected
	m_pointsInROIs.clear();
	DetectionPoint * points = m_dset->getPoints();
	if (!_onROIs || (_onROIs && _rois.empty())){
		m_pointsInROIs.resize(m_dset->nbPoints());
		for (unsigned int n = 0; n < m_dset->nbPoints(); n++)
			m_pointsInROIs[n] = &points[n];
	}
	else{
		for (unsigned int n = 0; n < m_dset->nbPoints(); n++){
			bool found = false;
			for (RoiList::const_iterator it = _rois.begin(); it != _rois.end() && !found; it++)
				found = it->inside(points[n].x(), points[n].y());
			if (found)
				m_pointsInROIs.push_back(&points[n]);
		}
	}

	unsigned int nbForUpdate = m_nbSteps / 100., cpt = 0;
	if (nbForUpdate == 0) nbForUpdate = 1;
	printf("Computing KRipley: %.2f %%", (0. / m_nbSteps * 100.));

	MyTimer timer;
	for( double r = m_minR; r <= m_maxR; r += m_stepR, index++ ){
		if (cpt++ % nbForUpdate == 0) printf("\rComputing KRipley: %.2f %%", ((double)cpt / m_nbSteps * 100.));
		m_ts[index] = r;
		double val = computeRipleyFunction( r );
		m_ks[index] = val;
		val = sqrt( val / M_PI ) - r;
		//std::cout << "For radius = " << r << "), k-function ripley value = " << val << ", t = " << timer.getTimeElapsed().toAscii().data() << std::endl;
		m_results[index] = val;
		m_ls[index] = val;
	}
	printf("\rComputing KRipley: 100 %%\n");
}

const double KRipley::computeRipleyFunction( const double _r )
{
	double res = 0., divisor = m_density, r2 = _r * _r;// * m_nbPoints;
	double meanNbNeighbors = 0, nbP = m_dset->nbPoints(), trueMean = 0.;
	//DetectionPoint * points = m_dset->getPoints();
	const double search_radius = static_cast<double>(r2);
	std::vector<std::pair<std::size_t, double> > ret_matches;
	nanoflann::SearchParams params;
	std::size_t nMatches;
	//for( int n = 0; n < m_dset->nbPoints(); n++ ){
	for (int n = 0; n < m_pointsInROIs.size(); n++){
		double x = m_pointsInROIs[n]->x(), y = m_pointsInROIs[n]->y();
		bool crossBorder = ( x < _r ) || ( y < _r ) || ( x > ( m_w - _r ) ) || ( y > ( m_h - _r ) );
		double areaDomain = M_PI * _r * _r;
		double factorArea = ( crossBorder ) ? edgeCorrection( x, y, _r ) : areaDomain;
		factorArea = areaDomain / factorArea;

		double sum = 0.;
		//NodeElement ** neighbors = ( NodeElement ** )malloc( sizeArray * sizeof( NodeElement * ) );
		const double queryPt[2] = { x, y };
		nMatches = m_tree->radiusSearch(&queryPt[0], search_radius, ret_matches, params);
		sum = nMatches - 1;
		/*for (int i = 0; i < nMatches; i++){
			if (ret_matches[i].second < r2)
				sum++;
		}*/
		res += ( factorArea * sum ) / divisor;
		trueMean += ( factorArea * sum ) / nbP;
		meanNbNeighbors += (double)nMatches / nbP;
	}
	res /= ( double )m_dset->nbPoints();
	//std::cout << "Mean of the number of neighs -> " << meanNbNeighbors << " / " << trueMean << " and sum value is " << res << std::endl;
	return res;
}

const double KRipley::edgeCorrection( const double _x, const double _y, const double _r )
{
	double areaCircle = M_PI * _r * _r;

	std::vector < Vec2md > intersectionPoints;
	Geometry::circleLineIntersect( 0., 0., m_w, 0., _x, _y, _r, intersectionPoints );
	Geometry::circleLineIntersect(m_w, 0., m_w, m_h, _x, _y, _r, intersectionPoints);
	Geometry::circleLineIntersect(0., m_h, m_w, m_h, _x, _y, _r, intersectionPoints);
	Geometry::circleLineIntersect(0., 0., 0., m_h, _x, _y, _r, intersectionPoints);

	if( intersectionPoints.size() != 2 && intersectionPoints.size() != 4 ){
		//std::cout << "Problem on the number of intersections -> " << intersectionPoints.size() << std::endl;
		return areaCircle;
	}
	else{
		int nbCornersInsideCircle = 0;
		if( Geometry::distance( _x, _y, 0., 0. ) < _r )
			nbCornersInsideCircle++;
		if (Geometry::distance(_x, _y, 0., m_h) < _r)
			nbCornersInsideCircle++;
		if (Geometry::distance(_x, _y, m_w, m_h) < _r)
			nbCornersInsideCircle++;
		if (Geometry::distance(_x, _y, m_w, 0.) < _r)
			nbCornersInsideCircle++;

		double areaCircle = M_PI * _r * _r;
		double area = areaCircle;

		for( unsigned int n = 0; n < intersectionPoints.size(); n += 2 ){
			Vec2md p1 = intersectionPoints[n], p2 = intersectionPoints[n+1];
			double areaCircularSegment = Geometry::computeAreaCircularSegment( _x, _y, _r, p1, p2 );
			double a = Geometry::distance( _x, _y, p1.x(), p1.y() );
			double b = Geometry::distance( _x, _y, p2.x(), p2.y() );
			double c = Geometry::distance( p1.x(), p1.y(), p2.x(), p2.y() );
			double areaTriangle = Geometry::computeAreaTriangle( a, b, c );

			if( nbCornersInsideCircle == 0 )
				area -= areaCircularSegment;
			else if( nbCornersInsideCircle == 1 )
				area -= ( areaCircularSegment + areaTriangle );
		}
		return area;
	}
}

void KRipley::exportResults( const std::string & _filename )
{
	std::ofstream fs( _filename.c_str() );
	fs << "Radius\tValue" << std::endl;
	unsigned int index = 0;
	for( double r = m_minR; r <= m_maxR; r += m_stepR, index++ ){
		fs << r << "\t" << m_results[index] << std::endl;
	}
	fs.close();
}
