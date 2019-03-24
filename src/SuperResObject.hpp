/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      SuperResObject.hpp
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

#ifndef SuperResObject_h__
#define SuperResObject_h__

#include "Vec4.hpp"
#include "ObjectInterface.hpp"
#include "NeuronObject.hpp"

class DetectionSet;
class WrapperVoronoiDiagram;
class DetectionCleaner;
class Camera2D;
class KRipley;
class DBScan;

class SuperResObject{
public:
	enum TypeColor{ OBJECT_SHAPE = 0, OBJECT_OUTLINE = 1, OBJECT_ELLIPSE = 2, CLUSTER_SHAPE = 3, CLUSTER_OUTLINE = 4, CLUSTER_ELLIPSE = 5 };

	SuperResObject();
	SuperResObject( const std::string &, const QString &, const double, const double );
	~SuperResObject();

	void draw( Camera2D * ) const;
	void drawNeuronObjects( Camera2D * ) const;
	void drawRois( Camera2D * ) const;

	const std::string & getDir() const;
	void addNeuronObjects( const NeuronObjectList & );
	void exportStats( const double = -1., const int = -1 ) const;
	void addPointToRoi( const double, const double );
	bool addRoiToList();
	const Roi & getRoi( const int ) const;
	void discardRoi( const int );
	void setDetectionSet(DetectionSet *);
	void setDetectionCleaner(DetectionCleaner *);
	void setVoronoiDiagram( WrapperVoronoiDiagram * _wrapper );
	void removeObjectsInsideROIs( const RoiList & );

	void exportIDLocalizations(const bool) const;

	Color4D & getColor(const int);
	void setColor(const int, const Color4D &);
	void changeColor(const int);

	void transferColorVoronoiObjsToLocs();
	void transferColorVoronoiClustersToLocs();

	inline DetectionSet * getDetectionSet() {return m_dset;}
	inline DetectionSet * getDetectionSetCleaned() {return m_dsetCleaner;}
	inline DetectionCleaner * getDetectionCleaner() {return m_dcleaner;}
	inline WrapperVoronoiDiagram * getVoronoiDiagram(){return m_voronoiDiagram;}
	inline const NeuronObjectList & getNeuronObjects() const {return m_voronoiObjects;}
	inline NeuronObjectList & getNeuronObjects() {return m_voronoiObjects;}
	inline int nbNeuronObjects() const {return m_voronoiObjects.size();}
	inline KRipley * getKRipley() const { return m_ripley; }
	inline DBScan * getDBSCAN() const { return m_dbscan; }

	inline const RoiList & getRois() const {return m_rois;}
	inline int nbRois() const {return m_rois.size();}
	inline void discardAllRois(){m_rois.clear();}
	inline void addRoiToList( const Roi & _roi ){m_rois.push_back( _roi );}

	inline void toggleDisplayLabelRoi( const bool _val ){m_displayLabelRoi = _val;}
	inline const QString & getName() const {return m_name;}
	inline const float getWidth() const {return m_w;}
	inline const float getHeight() const {return m_h;}

	inline const Color4D & getColorObjShape() const { return m_colorObjsShape; }
	inline const Color4D & getColorObjOutline() const { return m_colorObjsOutline; }
	inline const Color4D & getColorObjEllipse() const { return m_colorObjsEllipse; }
	inline const Color4D & getColorClusterShape() const { return m_colorClustersShape; }
	inline const Color4D & getColorClusterOutline() const { return m_colorClustersOutline; }
	inline const Color4D & getColorClusterEllipse() const { return m_colorClustersEllipse; }

	inline void setColorObjShape(const Color4D & _c){ m_colorObjsShape = _c; }
	inline void setColorObjOutline(const Color4D & _c){ m_colorObjsOutline = _c; }
	inline void setColorObjEllipse(const Color4D & _c){ m_colorObjsEllipse = _c; }
	inline void setColorClusterShape(const Color4D & _c){ m_colorClustersShape = _c; }
	inline void setColorClusterOutline(const Color4D & _c){ m_colorClustersOutline = _c; }
	inline void setColorClusterEllipse(const Color4D & _c){ m_colorClustersEllipse = _c; }


protected:
	Roi m_currentRoi;
	RoiList m_rois;
	bool m_displayLabelRoi;
	
	DetectionSet * m_dset, * m_dsetCleaner;
	WrapperVoronoiDiagram * m_voronoiDiagram;
	DetectionCleaner * m_dcleaner;
	NeuronObjectList m_voronoiObjects;

	KRipley * m_ripley;
	DBScan * m_dbscan;

	QString m_name;
	std::string m_dir;
	double m_w, m_h;

	Color4D m_colorObjsShape, m_colorObjsOutline, m_colorObjsEllipse, m_colorClustersShape, m_colorClustersOutline, m_colorClustersEllipse;
};

#endif // SuperResObject_h__