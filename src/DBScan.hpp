/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      DBScan.hpp
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

#ifndef DBScan_h__
#define DBScan_h__

#include <list>
#include <vector>
#include <string>

#include "DetectionSet.hpp"
#include "nanoflann.hpp"
#include "Vec2.hpp"
#include "Vec4.hpp"

class DBScanPoint{
public:
	DBScanPoint();
	DBScanPoint( const double, const double, const unsigned int, const unsigned int );

	void setPoint( const double, const double, const unsigned int, const unsigned int );
	const std::string toString() const;

	inline void setClusterId(const unsigned int _val){ m_clusterID = _val; }

protected:
	double m_x, m_y;
	unsigned int m_clusterID, m_origID;

	friend class DBScan;
};

typedef std::vector < DBScanPoint > DBPoints;
typedef std::vector < DBScanPoint * > DBRegion;
typedef std::vector < DBScanPoint * > DBCluster;
typedef std::vector < DBCluster > DBClusters;

class DBScan{
public:
	DBScan(DetectionSet *);
	DBScan( DetectionSet *, const double, const unsigned int, const unsigned int );
	DBScan(DetectionSet *, DetectionSet *);
	~DBScan();

	void execute(const double, const unsigned int, const unsigned int, const bool = true);
	void execute();
	const unsigned int computeClusters( DBPoints & );
	const bool expandCluster( DBPoints &, DBScanPoint *, const unsigned int, const double );
	const DBRegion getRegion( DBPoints &, DBScanPoint *, const double );

	Vec2md * generateVertices() const;

	bool * locsOfClustersSelected() const;
	unsigned int * getColorLocsSelected(unsigned int &, const unsigned int) const;
	Color4D * getColorPerClusters(const unsigned int) const;

	inline const DBClusters & getClusters() const{ return m_clusters;}
	inline void setParameters(const double _eps, const unsigned int _minNb){ m_eps = _eps; m_minPts = _minNb; }
	inline const unsigned int nbVertices() const { return m_nbOriginalPoints; }

	inline double * getSizeClusters() const { return m_sizeClusters; }
	inline double * getMajorAxisClusters() const { return m_majorAxisClusters; }
	inline double * getMinorAxisClusters() const { return m_minorAxisClusters; }
	inline double * getNbLocsClusters() const { return m_nbLocsClusters; }
	inline const unsigned int getNbClusters() const { return m_realNbClusters; }
	inline Vec2mf * getCentroids() const { return m_centroids; }

protected:
	DBClusters m_clusters;
	DBPoints m_points;
	double m_eps;
	unsigned int m_minPts, m_unclassifiedId, m_noiseId, m_nbOriginalPoints, m_nbMinCluster;
	bool m_applyPCA;

	KdPointCloud_D * m_cloud;
	KdTree_2D_double * m_tree;

	double * m_sizeClusters, * m_majorAxisClusters, * m_minorAxisClusters, * m_nbLocsClusters;
	unsigned int m_realNbClusters;
	Vec2mf * m_centroids;
};
#endif // DBScan_h__