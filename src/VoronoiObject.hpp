/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      VoronoiObject.hpp
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

#ifndef VoronoiObject_h__
#define VoronoiObject_h__

#include <vector>
#include <list>

#include "Vec2.hpp"
#include "Vec4.hpp"
#include "ObjectInterface.hpp"
#include "GeneralTools.hpp"
#include "MoleculeInfos.hpp"
#include "Roi.hpp"

class WrapperVoronoiDiagram;
class Camera2D;

class VoronoiCluster{
public:
	enum DataClusterType2 {LocalDensity = 0, MeanDistance = 1, Area = 2, MajorAxis = 3, MinorAxis = 4, Circularity = 5, Diameter = 6};

	VoronoiCluster();
	VoronoiCluster( WrapperVoronoiDiagram * );
	VoronoiCluster( const VoronoiCluster & );
	~VoronoiCluster();

	void setTriangles( FaceHandle *, const int );
	virtual void setMolecules( unsigned int *, const int );
	void setOutline( const std::vector < Vec2dm > & );
	void fitEllipsePCA();
	void fitBoundingEllipse();

	void draw() const;
	void drawEllipse() const;
	//void selectMoleculesInsideROIs( const Roi &, unsigned int *, unsigned int & ) const;
	void setArea( const double _area );

	inline double getArea() const {return m_data[MoleculeInfos::Area];}
	inline unsigned int * getMolecules() const {return m_molecules;}
	inline int nbMolecules() const {return m_nbMolecules;}
	inline const std::vector < Vec2dm > & getOutlines() const {return m_outlines;}
	inline double getData( const int _typeHisto ) const {return m_data[_typeHisto];}
	inline const Vec2mf & getBarycenter() const {return m_barycenter;}

public:
	static unsigned short NB_DATATYPE;

protected:
	FaceHandle * m_triangles;
	unsigned int * m_molecules;
	int m_nbTriangles, m_nbMolecules;
	double * m_data, m_ellipse[5];

	std::vector < Vec2dm > m_outlines;
	WrapperVoronoiDiagram * m_parent;

	Vec2mf m_barycenter;
	//std::vector < Vec2mf > m_ellipse;
	Vec2fm m_center, m_longestAxis, m_shortestAxis;

	friend class VoronoiObject;
	friend class VoronoiClusterList;
	friend class NeuronObject;
	friend class Roi;
};

class VoronoiClusterList: public std::vector < VoronoiCluster * >{
public:
	VoronoiClusterList();
	~VoronoiClusterList();

	void draw(const Color4D &, const Color4D &, const Color4D &) const;
	void generateDisplay();
	void erase();

	inline void toggleDisplayShape( bool _val ){m_displayShape = _val;}
	inline void toggleDisplayOutline( bool _val ){m_displayOutline = _val;}

public:
	//Creation of several VoronoiClusters defined by all the molecules above a threshold of a particular histogram
	static void determineClusters( VoronoiCluster *, unsigned int *, bool *, FaceHandle *, bool *, const double, const int, VoronoiClusterList & );
	static void determineClusters(WrapperVoronoiDiagram *, bool *, const unsigned int, const double, const unsigned int, const double, VoronoiClusterList &);

protected:
	bool m_displayShape, m_displayOutline;
	int m_nbMolClusters;

	Vec2mf * m_trianglesCell;
	int * m_firstVerticesTriangle, * m_sizeVerticesTriangle;
	int m_nbVertForTriangles;
};

class VoronoiObject: public VoronoiCluster, public ObjectInterface{
public:
	VoronoiObject();
	VoronoiObject( WrapperVoronoiDiagram * );
	VoronoiObject( const VoronoiCluster & );
	VoronoiObject( const VoronoiObject & );
	~VoronoiObject();

	void setMolecules( unsigned int *, const int );
	
	void draw(const Color4D &, const Color4D &, const Color4D &) const;

	void getHistogramParameters( double &, double &, double &, double &, const int, const bool );
	double * getHistogram( const int, const bool ) const;
	void forceRegenerateSelection();
	void determineSelection( const bool = false );
	void resetDataSelection();
	void regenerateIntensityColorVector();

	double getInfosData( const int, const int ) const;
	double getInfosDataLog( const int, const int ) const;

	void setVoronoiObjectFromCluster( VoronoiCluster * );
	
	inline bool isPolygonFilled() const {return m_filled;}
	inline void setPolygonFilled( const bool _val ){m_filled = _val;}
	inline bool isOutlineDisplay() const {return m_outlineDisplay;}
	inline void setOutlineDisplay( const bool _val ){m_outlineDisplay = _val;}
	inline void setEllipseDisplay(const bool _val){ m_ellipseDisplay = _val; }

protected:
	void generateDisplay();
	void generateStats();

protected:
	bool m_filled, m_outlineDisplay, m_ellipseDisplay;

	Vec2mf * m_positionMolecules;

	Vec2mf * m_trianglesCell;
	int * m_firstVerticesTriangle, * m_sizeVerticesTriangle;
	int m_nbVertForTriangles;
};

#endif // VoronoiObject_h__