/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      SuperResObject.cpp
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

#include <fstream>
#include <QFileDialog>
#include <QColorDialog>

#include "SuperResObject.hpp"
#include "DetectionSet.hpp"
#include "WrapperVoronoiDiagram.hpp"
#include "Camera2D.hpp"
#include "DetectionCleaner.hpp"
#include "KRipley.hpp"
#include "DBScan.hpp"

SuperResObject::SuperResObject() :m_displayLabelRoi(true), m_colorObjsShape(0.3, 0.5, 1., 1.), m_colorObjsOutline(1., 0, 0, 1.), m_colorObjsEllipse(1., 1., 0, 1.), m_colorClustersShape(0.4, 0.8, 0.02, 1), m_colorClustersOutline(1, 0, 0, 1), m_colorClustersEllipse(1, 1, 0, 1)
{
	m_dset = m_dsetCleaner = NULL;
	m_voronoiDiagram = NULL;
	m_dcleaner = NULL;
	m_ripley = NULL;
	m_dbscan = NULL;
}

SuperResObject::SuperResObject(const std::string & _dir, const QString & _name, const double _w, const double _h) :m_dir(_dir), m_name(_name), m_w(_w), m_h(_h), m_displayLabelRoi(true), m_colorObjsShape(0.3, 0.5, 1., 1.), m_colorObjsOutline(1., 0, 0, 1.), m_colorObjsEllipse(1., 1., 0, 1.), m_colorClustersShape(0.4, 0.8, 0.02, 1), m_colorClustersOutline(1, 0, 0, 1), m_colorClustersEllipse(1, 1, 0, 1)
{
	m_dset = m_dsetCleaner = NULL;
	m_voronoiDiagram = NULL;
	m_dcleaner = NULL;
	m_ripley = NULL;
	m_dbscan = NULL;
}

SuperResObject::~SuperResObject()
{
	if( m_dset != NULL )
		delete m_dset;
	if( m_dsetCleaner != NULL )
		delete m_dsetCleaner;
	if( m_voronoiDiagram != NULL )
		delete m_voronoiDiagram;
	if( m_dcleaner != NULL )
		delete m_dcleaner;
	for( unsigned int n = 0; n < m_voronoiObjects.size(); n++ )
		delete m_voronoiObjects[n];
	m_voronoiObjects.clear();
	if (m_ripley != NULL)
		delete m_ripley;
	if (m_dbscan != NULL)
		delete m_dbscan;
}

void SuperResObject::draw( Camera2D * _camera ) const
{
	glPushMatrix();

	if( m_dset != NULL )
		m_dset->draw();

	if( m_dsetCleaner != NULL )
		m_dsetCleaner->draw();

	if( m_voronoiDiagram != NULL )
		m_voronoiDiagram->draw();

	drawNeuronObjects( _camera );

	drawRois( _camera );

	glPopMatrix();
}

void SuperResObject::drawNeuronObjects( Camera2D * _camera ) const
{
	for( NeuronObjectList::const_iterator it = m_voronoiObjects.begin(); it != m_voronoiObjects.end(); it++ ){
		NeuronObject * nobj = *it;
		nobj->draw(m_colorObjsShape, m_colorObjsOutline, m_colorObjsEllipse, m_colorClustersShape, m_colorClustersOutline, m_colorClustersEllipse);
	}
}

void SuperResObject::drawRois( Camera2D * _camera ) const
{
	double w = this->getWidth(), h = this->getHeight();
	QFont font( "Times", 10, QFont::Bold );
	QFontMetrics fm( font );

	glColor3f( 1.f, 0., 0. );
	glBegin( GL_LINE_STRIP );
	for( Roi::const_iterator it = m_currentRoi.begin(); it != m_currentRoi.end(); it++ )
		glVertex2f( it->x() / w, it->y() / h );
	glEnd();
	int cpt = 1;
	for( RoiList::const_iterator it = m_rois.begin(); it != m_rois.end(); it++, cpt++ ){
		glBegin( GL_LINE_STRIP );
		double cx = 0., cy = 0., size = it->size();
		for( Roi::const_iterator it2 = it->begin(); it2 != it->end(); it2++ ){
			glVertex2f( it2->x() / w, it2->y() / h );
			cx += it2->x() / size;
			cy += it2->y() / size;
		}
		glVertex2f( it->begin()->x() / w, it->begin()->y() / h );
		glEnd();
		if( m_displayLabelRoi ){
			QString text( "r" + QString::number( cpt ) );
			int textW1 = fm.width( text );
			Vec2mf pos = _camera->getScreenCoordinates( cx / w, cy / h );
			_camera->renderText( pos.x(), pos.y(), text, font );
		}
	}
}

void SuperResObject::addNeuronObjects( const NeuronObjectList & _objects )
{
	m_voronoiObjects.insert( m_voronoiObjects.end(), _objects.begin(), _objects.end() );
}

const std::string & SuperResObject::getDir() const
{
	return m_dir;
}

void SuperResObject::exportStats( const double _factor, const int _nbMinMol ) const
{
	bool withCluster = ( _factor != -1. && _nbMinMol != -1 ) ? true : false;
	QString tmp(getDir().c_str());
	QString nameXls(tmp);
	if (withCluster)
		nameXls.append("/ObjectsAndClustersStats_factor_" + QString::number(_factor) + "_nbMinMolClusters_" + QString::number(_nbMinMol) + ".xls");
	else
		nameXls.append("/ObjectsStats.xls");
	nameXls = QFileDialog::getSaveFileName(NULL, QObject::tr("Save stats..."), nameXls, QObject::tr("Stats files (*.xls)"), 0, QFileDialog::DontUseNativeDialog);
	if (nameXls.isEmpty()) return;
	std::ofstream fs( nameXls.toAscii().data() );
	if (!fs){
		std::cout << "System failed to open " << nameXls.toAscii().data() << std::endl;
		return;
	}
	else
		std::cout << "Saving stats in file " << nameXls.toAscii().data() << std::endl;
	int cpt = 1;
	if (withCluster)
		fs << "Object index\t#roi\t#cluster\tArea\t# detections\tLocal density\tMajor axis\tMinor axis\tCircularity\tDiameter\tBarycenter x\tBarycenter y" << std::endl;
	else
		fs << "Object index\tArea\t# detections\tLocal density\tMajor axis\tMinor axis\tCircularity\tDiameter\tBarycenter x\tBarycenter y" << std::endl;
	for (NeuronObjectList::const_iterator it = m_voronoiObjects.begin(); it != m_voronoiObjects.end(); it++, cpt++){
		NeuronObject * nobj = *it;
		nobj->exportStats( fs, cpt, withCluster );
	}
	fs.close();
}

void SuperResObject::exportIDLocalizations(const bool _inClusters) const
{
	QString tmp(getDir().c_str());
	QString nameXls(tmp);
	if (_inClusters)
		nameXls.append("/ID_localizations_clusters.xls");
	else
		nameXls.append("/ID_localizations_objects.xls");
	nameXls = QFileDialog::getSaveFileName(NULL, QObject::tr("Save locs ID..."), nameXls, QObject::tr("ID files (*.xls)"), 0, QFileDialog::DontUseNativeDialog);
	if (nameXls.isEmpty()) return;
	std::ofstream fs(nameXls.toAscii().data());
	if (!fs){
		std::cout << "System failed to open " << nameXls.toAscii().data() << std::endl;
		return;
	}
	else
		std::cout << "Saving stats in file " << nameXls.toAscii().data() << std::endl;

	if (_inClusters)
		fs << "Localization index\tCluster index\tx\ty" << std::endl;
	else
		fs << "Localization index\tObject index\tx\ty" << std::endl;

	unsigned int * ids = new unsigned int[m_voronoiDiagram->nbMolecules()];
	memset(ids, 0, m_voronoiDiagram->nbMolecules() * sizeof(unsigned int));

	unsigned int cpt = 1;
	if (_inClusters){
		for (NeuronObjectList::const_iterator it = m_voronoiObjects.begin(); it != m_voronoiObjects.end(); it++){
			NeuronObject * nobj = *it;
			nobj->exportIDLocsClusters(cpt, ids);
		}
	}
	else{
		for (NeuronObjectList::const_iterator it = m_voronoiObjects.begin(); it != m_voronoiObjects.end(); it++, cpt++){
			NeuronObject * nobj = *it;
			nobj->exportIDLocsObj(cpt, ids);
		}
	}

	MoleculeInfos * infos = m_voronoiDiagram->getMoleculeInfos();
	for (unsigned int n = 0; n < m_voronoiDiagram->nbMolecules(); n++)
		fs << n << "\t" << ids[n] << "\t" << infos[n].getMolecule()->point().x() << "\t" << infos[n].getMolecule()->point().y() << std::endl;

	delete[] ids;
	fs.close();
}

void SuperResObject::setDetectionCleaner( DetectionCleaner * _dcleaner )
{
	if( m_dcleaner != NULL )
		delete m_dcleaner;
	m_dcleaner = _dcleaner;

	if( m_dsetCleaner != NULL )
		delete m_dsetCleaner;

	double * xs = m_dcleaner->getXS(), * ys = m_dcleaner->getYS();
	unsigned short * ts = m_dcleaner->getTS();
	unsigned int * photons = m_dcleaner->getPhotons();

	m_dsetCleaner = new DetectionSet( xs, ys, ts, photons, m_dset->nbSlices(), m_dcleaner->getNbClean() );
	m_dsetCleaner->createDisplayPoints( m_w, m_h );
}

void SuperResObject::setVoronoiDiagram( WrapperVoronoiDiagram * _wrapper )
{
	if( m_voronoiDiagram != NULL )
		delete m_voronoiDiagram;
	for( unsigned int n = 0; n < m_voronoiObjects.size(); n++ )
		delete m_voronoiObjects[n];
	m_voronoiObjects.clear();

	m_voronoiDiagram = _wrapper;
}

void SuperResObject::removeObjectsInsideROIs( const RoiList & _rois )
{
	for (NeuronObjectList::iterator it = m_voronoiObjects.begin(); it != m_voronoiObjects.end(); it++)
		delete *it;
	m_voronoiObjects.clear();
	/*if( _rois.empty() )
	{
		for( NeuronObjectList::iterator it = m_voronoiObjects.begin(); it != m_voronoiObjects.end(); it++ )
			delete *it;
		m_voronoiObjects.clear();
	}
	else{
		std::vector < NeuronObjectList::iterator > toErase;
		for( NeuronObjectList::iterator it = m_voronoiObjects.begin(); it != m_voronoiObjects.end(); it++ ){
			VoronoiObject * vobj = ( *it )->getObject();
			bool insideROIs = false;
			for( RoiList::const_iterator it2 = _rois.begin(); it2 != _rois.end() && !insideROIs; it2++ ){
				const Vec2md & barycenter = vobj->getBarycenter();
				insideROIs = it2->inside( barycenter.x(), barycenter.y() );
			}
			if( insideROIs ){
				delete *it;
				toErase.push_back( it );
			}
		}
		for( std::vector < NeuronObjectList::iterator >::reverse_iterator it = toErase.rbegin(); it != toErase.rend(); it++ )
			m_voronoiObjects.erase( *it );
	}*/
}

void SuperResObject::addPointToRoi( const double _x, const double _y )
{
	m_currentRoi.push_back( Vec2mf( _x, _y ) );
}

bool SuperResObject::addRoiToList()
{
	if( m_currentRoi.empty() ) return false;
	m_rois.push_back( m_currentRoi ); 
	m_currentRoi.clear();
	return true;
}

const Roi & SuperResObject::getRoi( const int _index ) const
{
	return m_rois.at( _index );
}

void SuperResObject::discardRoi( const int _index )
{
	if( _index >= m_rois.size() ) return;
	RoiList::iterator it = m_rois.begin() + _index;
	m_rois.erase( it );
}

void SuperResObject::setDetectionSet(DetectionSet * _dset)
{
	m_dset = _dset;
	if (m_ripley != NULL)
		delete m_ripley;
	m_ripley = new KRipley(m_dset, m_w, m_h);
	if (m_dbscan != NULL)
		delete m_dbscan;
	m_dbscan = new DBScan(m_dset);
}

Color4D & SuperResObject::getColor(const int _type)
{
	switch (_type){
	case SuperResObject::OBJECT_SHAPE:
		return m_colorObjsShape;
	case SuperResObject::OBJECT_OUTLINE:
		return m_colorObjsOutline;
	case SuperResObject::OBJECT_ELLIPSE:
		return m_colorObjsEllipse;
	case SuperResObject::CLUSTER_SHAPE:
		return m_colorClustersShape;
	case SuperResObject::CLUSTER_OUTLINE:
		return m_colorClustersOutline;
	case SuperResObject::CLUSTER_ELLIPSE:
		return m_colorClustersEllipse;
	}
	return Color4D();
}

void SuperResObject::setColor(const int _type, const Color4D & _c)
{
	switch (_type){
	case SuperResObject::OBJECT_SHAPE:
		m_colorObjsShape = _c;
		break;
	case SuperResObject::OBJECT_OUTLINE:
		m_colorObjsOutline = _c;
		break;
	case SuperResObject::OBJECT_ELLIPSE:
		m_colorObjsEllipse = _c;
		break;
	case SuperResObject::CLUSTER_SHAPE:
		m_colorClustersShape = _c;
		break;
	case SuperResObject::CLUSTER_OUTLINE:
		m_colorClustersOutline = _c;
		break;
	case SuperResObject::CLUSTER_ELLIPSE:
		m_colorClustersEllipse = _c;
		break;
	default:
		break;
	}
}

void SuperResObject::changeColor(const int _type)
{
	Color4D & chosenColor = getColor(_type);
	QColor color = QColorDialog::getColor(QColor((int)(chosenColor[0] * 255), (int)(chosenColor[1] * 255), (int)(chosenColor[2] * 255)));
	if (color.isValid())
		chosenColor.set(color.redF(), color.greenF(), color.blueF(), 1);
}

void SuperResObject::transferColorVoronoiObjsToLocs()
{
	for (NeuronObjectList::const_iterator it = m_voronoiObjects.begin(); it != m_voronoiObjects.end(); it++){
		NeuronObject * nobj = *it;
		nobj->transferColorVoronoiObjsToLocs(m_dset, m_colorObjsShape);
	}
}

void SuperResObject::transferColorVoronoiClustersToLocs()
{
	for (NeuronObjectList::const_iterator it = m_voronoiObjects.begin(); it != m_voronoiObjects.end(); it++){
		NeuronObject * nobj = *it;
		nobj->transferColorVoronoiClustersToLocs(m_dset, m_colorClustersShape);
	}
}