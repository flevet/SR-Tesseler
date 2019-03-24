/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      WrapperVoronoiDiagram.hpp
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

#ifndef WrapperVoronoiDiagram_h__
#define WrapperVoronoiDiagram_h__

#include "Vec2.hpp"
#include "Vec3.hpp"
#include "Vec4.hpp"
#include "ObjectInterface.hpp"
#include "VoronoiObject.hpp"
#include "SuperResObject.hpp"
#include "GeneralTools.hpp"
#include "MoleculeInfos.hpp"

class WrapperVoronoiDiagram: public ObjectInterface{
public:
	WrapperVoronoiDiagram( DetectionPoint *, const int, const double, const double );
	~WrapperVoronoiDiagram();

	inline double getData( const int _typeHisto, const int _idx ) const {return m_data[_typeHisto][_idx];}
	inline bool isPolygonFilled() const {return m_filled;}
	inline void setPolygonFilled( const bool _val ){m_filled = _val;}
	inline int nbMolecules() const {return m_nbMolecules;}
	inline double getInfosData( const int _typeHisto, const int _idx ) const {return m_infos[_idx].getData( _typeHisto );}
	inline double getInfosDataLog( const int _typeHisto, const int _idx ) const {return m_infos[_idx].getDataLog( _typeHisto );}
	inline double getAverageDensity() const {return m_avgDensity;}
	inline double getFactorDensity() const {return m_factorDensity;}
	inline void setFactorDensity( const double _val ){m_factorDensity = _val;}
	inline double getArea() const {return m_area;}
	inline int getNbFiniteTriangles() const {return m_nbFiniteTriangles;}
	inline MoleculeInfos * getMoleculeInfos() const {return m_infos;}
	inline const double getWidth() const { return m_originalWidth; }
	inline const double getHeight() const { return m_originalHeight; }

	void draw() const;

	void getHistogramParameters( double &, double &, double &, double &, const int, const bool );
	double * getHistogram( const int, const bool ) const;
	void forceRegenerateSelection();
	void determineSelection( const bool = false );
	void resetDataSelection();
	void regenerateIntensityColorVector();

	NeuronObjectList createVoronoiObjects(const double = 0., const unsigned int = 1, const double = DBL_MAX, const unsigned int = UINT_MAX, const bool = false, const double = DBL_MAX, const bool = true, const bool = false, const double = 60., const double = 60.);
	void iterativeAddCells( FaceHandle, FaceHandle *, int &, bool * );

	const double getMeanDensityFromSelectedLocalizations( unsigned int *, const unsigned int ) const;

	void applyDensityFactorROIs( const double, const bool, const bool, const RoiList & );

protected:
	void generateDisplay();

protected:
	double m_originalWidth, m_originalHeight;
	Delaunay_triangulation_2 m_delau;

	double ** m_data;

	MoleculeInfos * m_infos;
	EdgeCirc * m_edgesVoronoiPolygons;

	int m_nbMolecules, m_nbFiniteTriangles, m_nbOriginalPoints;
	double * m_areaTriangles;
	bool m_filled;

	Vec2mf * m_linesCell;
	int * m_firstVerticesLine, * m_sizeVerticesLine;
	Color4D * m_colorsLine;
	int m_nbVertForLines;

	Vec2mf * m_trianglesCell;
	int * m_firstVerticesTriangle, * m_sizeVerticesTriangle;
	Color4D * m_colorsTriangle;
	int m_nbVertForTriangles;

	double m_avgDensity, m_factorDensity, m_area;

	std::vector < Vec2md > m_ptsLocalMax;

	friend class VoronoiObject;
	friend class VoronoiCluster;
	friend class VoronoiClusterList;
};

#endif // WrapperVoronoiDiagram_h__