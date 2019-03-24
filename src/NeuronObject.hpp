/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      NeuronObject.hpp
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

#ifndef NeuronObject_h__
#define NeuronObject_h__

#include "VoronoiObject.hpp"

class DetectionSet;

class NeuronObject{
public:
	NeuronObject( VoronoiObject * );
	~NeuronObject();

	void draw(const Color4D &, const Color4D &, const Color4D &, const Color4D &, const Color4D &, const Color4D &) const;
	void exportStats( std::ofstream &, const int, const bool ) const;
	void clearClusters();
	void addCluster( VoronoiCluster * );

	void toggleDisplayShapeObjs( bool );
	void toggleDisplayOutlineObjs( bool );
	void toggleDisplayEllipseObjs(bool);
	void toggleDisplayShapeClusts(bool);
	void toggleDisplayOutlineClusts( bool );

	void exportIDLocsObj(const unsigned int, unsigned int *) const;
	void exportIDLocsClusters(unsigned int &, unsigned int *) const;

	void transferColorVoronoiObjsToLocs(DetectionSet *, const Color4D &);
	void transferColorVoronoiClustersToLocs(DetectionSet *, const Color4D &);

	inline VoronoiObject * getObject() const {return m_object;}
	inline const VoronoiClusterList & getClusters() {return m_clusters;}
	inline void generateDisplayClusters(){m_clusters.generateDisplay();}
	inline const unsigned int nbClusters() const {return m_clusters.size();}
	inline VoronoiCluster * getCluster( const unsigned int _index ){return m_clusters[_index];}

protected:
	VoronoiObject * m_object;
	VoronoiClusterList m_clusters;
};

typedef std::vector < NeuronObject * > NeuronObjectList;

#endif // NeuronObject_h__