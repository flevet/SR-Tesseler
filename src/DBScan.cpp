/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      DBScan.cpp
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

#include <QtCore/QString>

#include "DBScan.hpp"
#include "GeneralTools.hpp"
#include "Geometry.hpp"

DBScanPoint::DBScanPoint()
{

}

DBScanPoint::DBScanPoint( const double _x, const double _y, const unsigned int _origId, const unsigned int _clusterId ) :m_x( _x ), m_y( _y ), m_origID( _origId ), m_clusterID( _clusterId )
{

}

void DBScanPoint::setPoint( const double _x, const double _y, const unsigned int _origId, const unsigned int _clusterId )
{
	m_x = _y; 
	m_y = _y; 
	m_origID = _origId; 
	m_clusterID = _clusterId;
}

const std::string DBScanPoint::toString() const
{
	QString s( "[" + QString::number( m_x ) + ", " + QString::number( m_y ) + ", " + QString::number( m_origID ) + ", " + QString::number( m_clusterID ) + ", " );
	return s.toAscii().data();
}

DBScan::DBScan(DetectionSet * _dset) :m_eps(0.), m_minPts(0.), m_nbMinCluster(0.)
{
	m_sizeClusters = m_majorAxisClusters = m_minorAxisClusters = m_nbLocsClusters = NULL;
	m_centroids = NULL;

	DetectionPoint * origPoints = _dset->getPoints();
	m_nbOriginalPoints = _dset->nbPoints();
	m_unclassifiedId = m_nbOriginalPoints;
	m_noiseId = m_nbOriginalPoints + 1;

	m_cloud = new KdPointCloud_D();
	m_cloud->m_pts.resize(_dset->nbPoints());
	for (unsigned int n2 = 0; n2 < _dset->nbPoints(); n2++){
		m_cloud->m_pts[n2].m_x = origPoints[n2].x();
		m_cloud->m_pts[n2].m_y = origPoints[n2].y();
	}
	m_tree = new KdTree_2D_double(2, *m_cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */));
	m_tree->buildIndex();

	m_points.clear();
	for (unsigned int n = 0; n < m_nbOriginalPoints; n++)
		m_points.push_back(DBScanPoint(origPoints[n].x(), origPoints[n].y(), n, m_unclassifiedId));
}

DBScan::DBScan(DetectionSet * _dset, const double _eps, const unsigned int _minPts, const unsigned int _nbMinCluster) :DBScan(_dset)
{
	execute(_eps, _minPts, _nbMinCluster);
}

DBScan::DBScan(DetectionSet * _dset1, DetectionSet * _dset2)
{
	MyTimer timer;
	unsigned int cpt = 0;
	std::cout << "Construction of colocalized DBScan" << std::endl;

	m_nbOriginalPoints = _dset1->nbPoints() + _dset2->nbPoints();
	m_unclassifiedId = m_nbOriginalPoints;
	m_noiseId = m_nbOriginalPoints + 1;

	m_cloud = new KdPointCloud_D();
	m_cloud->m_pts.resize(m_nbOriginalPoints);
	for (unsigned int i = 0; i < 2; i++){
		DetectionPoint * origPoints = (i == 0) ? _dset1->getPoints() : _dset2->getPoints();
		unsigned int nbPoints = (i == 0) ? _dset1->nbPoints() : _dset2->nbPoints();
		for (unsigned int n2 = 0; n2 < nbPoints; n2++, cpt++){
			m_cloud->m_pts[cpt].m_x = origPoints[n2].x();
			m_cloud->m_pts[cpt].m_y = origPoints[n2].y();
		}
	}
	m_tree = new KdTree_2D_double(2, *m_cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */));
	m_tree->buildIndex();

	m_points.clear();
	for (unsigned int n = 0; n < m_nbOriginalPoints; n++)
		m_points.push_back(DBScanPoint(m_cloud->m_pts[n].m_x, m_cloud->m_pts[n].m_y, n, m_unclassifiedId));

	std::cout << "Time for construction of colocalized DBScan " << timer.getTimeElapsed().toAscii().data() << std::endl;
}

DBScan::~DBScan()
{
	delete m_cloud;
	delete m_tree;
	if (m_sizeClusters != NULL)
		delete[] m_sizeClusters;
	if (m_majorAxisClusters != NULL)
		delete[] m_majorAxisClusters;
	if (m_minorAxisClusters != NULL)
		delete[] m_minorAxisClusters;
	if (m_nbLocsClusters != NULL)
		delete[] m_nbLocsClusters;
	if (m_centroids != NULL)
		delete[] m_centroids;
}

void DBScan::execute(const double _eps, const unsigned int _minPts, const unsigned int _nbMinCluster, const bool _applyPCA)
{
	m_eps = _eps;
	m_minPts = _minPts;
	m_nbMinCluster = _nbMinCluster;
	m_applyPCA = _applyPCA;

	execute();
}

void DBScan::execute()
{
	MyTimer timer;
	std::cout << "Executing DBScan, d = " << m_eps << ", min # locs = " << m_minPts << std::endl;
	
	for (unsigned int n = 0; n < m_nbOriginalPoints; n++)
		m_points[n].setClusterId(m_unclassifiedId);

	unsigned int nbClusters = computeClusters(m_points);
	m_clusters.clear();
	m_clusters.resize(nbClusters);
	for (DBPoints::iterator it = m_points.begin(); it != m_points.end(); it++){
		DBScanPoint * p = &(*it);
		if (p->m_clusterID < m_unclassifiedId)
			m_clusters[p->m_clusterID].push_back(p);
	}

	if (m_sizeClusters != NULL)
		delete[] m_sizeClusters;
	if (m_majorAxisClusters != NULL)
		delete[] m_majorAxisClusters;
	if (m_minorAxisClusters != NULL)
		delete[] m_minorAxisClusters;
	if (m_nbLocsClusters != NULL)
		delete[] m_nbLocsClusters;
	if (m_centroids != NULL)
		delete[] m_centroids;
	unsigned int nbPointsOfClusters = 0, currentCluster = 0;
	m_realNbClusters = 0;
	for (DBClusters::const_iterator it = m_clusters.begin(); it != m_clusters.end(); it++){
		const DBCluster & cluster = *it;
		if (cluster.size() < m_nbMinCluster) continue;
		m_realNbClusters++;
	}
	float * characteristics = new float[8];
	m_sizeClusters = new double[m_realNbClusters];
	m_majorAxisClusters = new double[m_realNbClusters];
	m_minorAxisClusters = new double[m_realNbClusters];
	m_nbLocsClusters = new double[m_realNbClusters];
	m_centroids = new Vec2mf[m_realNbClusters];
	Vec2md * pointsOfCluster = new Vec2md[m_nbOriginalPoints];
	unsigned int cpt = 1;
	//printf("Computing size for object %d / %d", cpt, m_clusters.size());
	for (DBClusters::const_iterator it = m_clusters.begin(); it != m_clusters.end(); it++, cpt++){
		printf("\rComputing size for object %d / %d", cpt, m_clusters.size());
		const DBCluster & cluster = *it;
		if (cluster.size() < m_nbMinCluster) continue;
		nbPointsOfClusters = 0;
		for (DBCluster::const_iterator it2 = cluster.begin(); it2 != cluster.end(); it2++){
			DBScanPoint * p = *it2;
			pointsOfCluster[nbPointsOfClusters++].set(p->m_x, p->m_y);
		}
		if (m_applyPCA)
			Geometry::fitEllipsePCA(pointsOfCluster, nbPointsOfClusters, characteristics);
		else
			Geometry::fitBoundingEllipse(pointsOfCluster, nbPointsOfClusters, characteristics);
		m_sizeClusters[currentCluster] = (characteristics[6] + characteristics[7]) / 2.f;
		m_majorAxisClusters[currentCluster] = characteristics[6];
		m_minorAxisClusters[currentCluster] = characteristics[7];
		m_nbLocsClusters[currentCluster] = nbPointsOfClusters;
		m_centroids[currentCluster++].set(characteristics[0], characteristics[1]);
	}
	printf("\rComputing size for object %d / %d\n", m_clusters.size(), m_clusters.size());
	delete[] characteristics;

	std::cout << "Time for executing DBScan " << timer.getTimeElapsed().toAscii().data() << std::endl;
}

const unsigned int DBScan::computeClusters( DBPoints & _points )
{
	double nbs = _points.size();
	unsigned int nbForUpdate = nbs / 100., cpt = 0;
	if (nbForUpdate == 0) nbForUpdate = 1;
	printf("Computing DBSCAN: %.2f %%", (0. / nbs * 100.));

	double epsSq = m_eps * m_eps;
	unsigned int clusterId = 0, nbPoints = _points.size();
	for( DBPoints::iterator it = _points.begin(); it != _points.end(); it++ ){
		if (cpt++ % nbForUpdate == 0) printf("\rComputing DBSCAN: %.2f %%", ((double)cpt / nbs * 100.));
		//std::cout << "Point " << cpt++ << " / " << nbPoints << std::endl;
		DBScanPoint * p = &(*it);
		if( p->m_clusterID == m_unclassifiedId )
			if( expandCluster( _points, p, clusterId, epsSq ) )
				clusterId++;
	}
	printf("\rComputing DBSCAN: 100 %%\n");
	return clusterId;
}

const bool DBScan::expandCluster( DBPoints & _points, DBScanPoint * _p, const unsigned int _clusterId, const double _epsSq )
{
	DBRegion seeds = getRegion( _points, _p, _epsSq );
	if( seeds.size() < m_minPts ){
		_p->m_clusterID = m_noiseId;
		return false;
	}
	else{
		for( DBRegion::iterator it = seeds.begin(); it != seeds.end(); it++ ){
			DBScanPoint * p = *it;
			p->m_clusterID = _clusterId;
		}
		DBRegion::iterator elem = std::find( seeds.begin(), seeds.end(), _p );
		if( elem != seeds.end() )
			seeds.erase( elem );
		while( !seeds.empty() ){
			DBRegion results = getRegion( _points, seeds.front(), _epsSq );
			if( results.size() >= m_minPts ){
				for( DBRegion::iterator it2 = results.begin(); it2 != results.end(); it2++ ){
					DBScanPoint * p = *it2;
					if( p->m_clusterID == m_unclassifiedId || p->m_clusterID == m_noiseId ){
						if( p->m_clusterID == m_unclassifiedId )
							seeds.push_back( p );
						p->m_clusterID = _clusterId;
					}
				}
			}
			seeds.erase( seeds.begin() );	
		}
		/*for( DBRegion::iterator it = seeds.begin(); it != seeds.end(); it++ ){
			DBRegion results = getRegion( _points, *it, _epsSq );
			if( results.size() >= m_minPts ){
				for( DBRegion::iterator it2 = results.begin(); it2 != results.end(); it2++ ){
					DBScanPoint * p = *it2;
					if( p->m_clusterID == m_unclassifiedId || p->m_clusterID == m_noiseId ){
						if( p->m_clusterID == m_unclassifiedId )
							seeds.push_back( p );
						p->m_clusterID = _clusterId;
					}
				}
			}
		}*/
	}
	return true;
}

const DBRegion DBScan::getRegion( DBPoints & _points, DBScanPoint * _p, const double _epsSq )
{
	const double search_radius = static_cast<double>(_epsSq);
	std::vector<std::pair<std::size_t, double> > ret_matches;
	nanoflann::SearchParams params;
	std::size_t nMatches;
	const double queryPt[2] = { _p->m_x, _p->m_y };
	nMatches = m_tree->radiusSearch(&queryPt[0], search_radius, ret_matches, params);
	//distances[largestSet + i] = Geometry::distance(m_clouds[otherId]->m_pts[ret_index[0]].m_x, m_clouds[otherId]->m_pts[ret_index[0]].m_y, m_clouds[otherId]->m_pts[allSeeds[i]].m_x, m_clouds[otherId]->m_pts[allSeeds[i]].m_y);

	DBRegion region;
	for (unsigned int n = 0; n < nMatches; n++)
		region.push_back(&_points[ret_matches[n].first]);


	/*std::list < NodeElement * > * neighbors = m_quadT->getNeighboringCells( _p->m_x, _p->m_y, sqrt( _epsSq ) );
	DBRegion region;
	for( std::list < NodeElement * >::iterator it = neighbors->begin(); it != neighbors->end(); it++ ){
		NodeElement * ne = *it;
		DBScanPoint * p = _points[ne->getIndex()];
		double d = Geometry::distanceSqr( p->m_x, p->m_y, _p->m_x, _p->m_y );
		if( d <= _epsSq ) region.push_back( p );
	}
	delete neighbors;*/
	return region;
	/*DBRegion region;
	for( DBPoints::iterator it = _points.begin(); it != _points.end(); it++ ){
		DBScanPoint * p = *it;
		double d = Geometry::distanceSqr( p->m_x, p->m_y, _p->m_x, _p->m_y );
		if( d <= _epsSq ) region.push_back( *it );
	}
	return region;*/
}

Vec2md * DBScan::generateVertices() const
{
	Vec2md * vertices = new Vec2md[m_nbOriginalPoints];
	for (unsigned int n = 0; n < m_nbOriginalPoints; n++)
		vertices[n].set(m_cloud->m_pts[n].m_x, m_cloud->m_pts[n].m_y);
	return vertices;
}

bool * DBScan::locsOfClustersSelected() const
{
	bool * selection = new bool[m_nbOriginalPoints];
	memset(selection, 0, m_nbOriginalPoints * sizeof(bool));
	for (DBClusters::const_iterator it = m_clusters.begin(); it != m_clusters.end(); it++){
		const DBCluster & cluster = *it;
		for (DBCluster::const_iterator it2 = cluster.begin(); it2 != cluster.end(); it2++){
			DBScanPoint * p = *it2;
			selection[p->m_origID] = true;
		}
	}
	return selection;
}

unsigned int * DBScan::getColorLocsSelected(unsigned int & _size, const unsigned int _nbMinCluster) const
{
	unsigned int * indexes = new unsigned int[m_nbOriginalPoints];
	_size = 0;
	for (DBClusters::const_iterator it = m_clusters.begin(); it != m_clusters.end(); it++){
		const DBCluster & cluster = *it;
		if (cluster.size() < _nbMinCluster) continue;
		for (DBCluster::const_iterator it2 = cluster.begin(); it2 != cluster.end(); it2++){
			DBScanPoint * p = *it2;
			indexes[_size++] = p->m_origID;
		}
	}
	return indexes;
}
Color4D * DBScan::getColorPerClusters(const unsigned int _nbMinCluster) const
{
	Color4D * colors = new Color4D[m_nbOriginalPoints];
	for (unsigned int n = 0; n < m_nbOriginalPoints; n++) colors[n].set(0, (float)170 / (float)255, (float)127 / (float)255, 1.f);

	for (DBClusters::const_iterator it = m_clusters.begin(); it != m_clusters.end(); it++){
		const DBCluster & cluster = *it;
		if (cluster.size() < _nbMinCluster) continue;
		float r = ((float)rand() / (float)RAND_MAX);
		float g = ((float)rand() / (float)RAND_MAX);
		float b = ((float)rand() / (float)RAND_MAX);
		for (DBCluster::const_iterator it2 = cluster.begin(); it2 != cluster.end(); it2++){
			DBScanPoint * p = *it2;
			colors[p->m_origID].set(r, g, b, 1.f);
		}
	}

	return colors;
}

