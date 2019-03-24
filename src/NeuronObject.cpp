/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      NeuronObject.cpp
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

#include "NeuronObject.hpp"
#include "VoronoiWidget.hpp"
#include "DetectionSet.hpp"

#include <fstream>
#include <QTime>

NeuronObject::NeuronObject( VoronoiObject * _obj )
{
	m_object = _obj;
}

NeuronObject::~NeuronObject()
{
	delete m_object;
	clearClusters();
}

void NeuronObject::draw(const Color4D & _objsShapeColor, const Color4D & _objsOutlineColor, const Color4D & _objsEllipseColor, const Color4D & _clustersShapeColor, const Color4D & _clustersOutlineColor, const Color4D & _clustersEllipseColor) const
{
	m_object->draw(_objsShapeColor, _objsOutlineColor, _objsEllipseColor);
	m_clusters.draw(_clustersShapeColor, _clustersOutlineColor, _clustersEllipseColor);
}
void NeuronObject::toggleDisplayShapeObjs( bool _val )
{
	m_object->setSelected( _val );
}

void NeuronObject::toggleDisplayOutlineObjs( bool _val )
{
	m_object->setOutlineDisplay( _val );
}

void NeuronObject::toggleDisplayEllipseObjs(bool _val)
{
	m_object->setEllipseDisplay(_val);
}

void NeuronObject::toggleDisplayShapeClusts( bool _val )
{
	m_clusters.toggleDisplayShape( _val );
}

void NeuronObject::toggleDisplayOutlineClusts( bool _val )
{
	m_clusters.toggleDisplayOutline( _val );
}

void NeuronObject::exportStats( std::ofstream & _fs, const int _indexObj, const bool _withClusters ) const
{
	double area = m_object->getData( MoleculeInfos::Area ), areaClusters = 0.;
	int nbMolecules = m_object->nbMolecules(), nbMoleculesClusters = 0;
	if (_withClusters){
		_fs << _indexObj << "\t-\t-\t" << area << "\t" << nbMolecules << "\t" << m_object->getData(MoleculeInfos::LocalDensity) << "\t" << m_object->getData(VoronoiCluster::MajorAxis) << "\t" << m_object->getData(VoronoiCluster::MinorAxis) << "\t" << m_object->getData(VoronoiCluster::Circularity) << "\t" << m_object->getData(VoronoiCluster::Diameter) << "\t" << m_object->getBarycenter().x() << "\t" << m_object->getBarycenter().y() << std::endl;
		int cpt = 1;
		for (VoronoiClusterList::const_iterator it = m_clusters.begin(); it != m_clusters.end(); it++, cpt++){
			VoronoiCluster * cluster = *it;
			_fs << _indexObj << "\t-\t" << cpt << "\t" << cluster->getData(VoronoiCluster::Area) << "\t" << cluster->nbMolecules() << "\t" << cluster->getData(VoronoiCluster::LocalDensity) << "\t" << cluster->getData(VoronoiCluster::MajorAxis) << "\t" << cluster->getData(VoronoiCluster::MinorAxis) << "\t" << cluster->getData(VoronoiCluster::Circularity) << "\t" << cluster->getData(VoronoiCluster::Diameter) << "\t" << cluster->getBarycenter().x() << "\t" << cluster->getBarycenter().y() << std::endl;
			areaClusters += cluster->getData(VoronoiCluster::Area);
			nbMoleculesClusters += cluster->nbMolecules();
		}
	}
	else{
		_fs << _indexObj << "\t" << area << "\t" << nbMolecules << "\t" << m_object->getData(MoleculeInfos::LocalDensity) << "\t" << m_object->getData(VoronoiCluster::MajorAxis) << "\t" << m_object->getData(VoronoiCluster::MinorAxis) << "\t" << m_object->getData(VoronoiCluster::Circularity) << "\t" << m_object->getData(VoronoiCluster::Diameter) << "\t" << m_object->getBarycenter().x() << "\t" << m_object->getBarycenter().y() << std::endl;
	}
}

void NeuronObject::exportIDLocsObj(const unsigned int _id, unsigned int * _locs) const
{
	unsigned int * locsObj = m_object->getMolecules();
	for (unsigned int n = 0; n < m_object->nbMolecules(); n++)
		_locs[locsObj[n]] = _id;
}

void NeuronObject::exportIDLocsClusters(unsigned int & _idCluster, unsigned int * _locs) const
{
	for (VoronoiClusterList::const_iterator it = m_clusters.begin(); it != m_clusters.end(); it++){
		VoronoiCluster * cluster = *it;
		unsigned int * locsCluster = cluster->getMolecules();
		for (unsigned int n = 0; n < cluster->nbMolecules(); n++)
			_locs[locsCluster[n]] = _idCluster;
		_idCluster++;
	}
}

void NeuronObject::clearClusters()
{
	m_clusters.erase();
}

void NeuronObject::addCluster( VoronoiCluster * _cluster )
{
	m_clusters.push_back( _cluster );
}

void NeuronObject::transferColorVoronoiObjsToLocs(DetectionSet * _dset, const Color4D & _c)
{
	_dset->colorLocsOfObject(m_object->getMolecules(), m_object->nbMolecules(), _c);
}

void NeuronObject::transferColorVoronoiClustersToLocs(DetectionSet * _dset, const Color4D & _c)
{
	for (VoronoiClusterList::const_iterator it = m_clusters.begin(); it != m_clusters.end(); it++){
		VoronoiCluster * cluster = *it;
		_dset->colorLocsOfObject(cluster->getMolecules(), cluster->nbMolecules(), _c);
	}
}