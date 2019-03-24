/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      ObjectInterface.hpp
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

#ifndef ObjectInterface_h__
#define ObjectInterface_h__

#include <math.h>

// CGAL headers
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Regular_triangulation_euclidean_traits_2.h>
#include <CGAL/Regular_triangulation_2.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/algorithm.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/linear_least_squares_fitting_2.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Min_ellipse_2.h>
#include <CGAL/Min_ellipse_2_traits_2.h>
#include <CGAL/Gmpq.h>
#include <CGAL/Aff_transformation_2.h>

#include "Palette.hpp"
#include "Histogram.hpp"
#include "GeneralTools.hpp"

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point_2;
typedef K::Iso_rectangle_2 Iso_rectangle_2;
typedef K::Segment_2 Segment_2;
typedef K::Ray_2 Ray_2;
typedef K::Line_2 Line_2;
typedef K::Iso_rectangle_2 Iso_rectangle_2;
typedef K::FT FT;
typedef CGAL::Polygon_2<K> Polygon_2;

typedef CGAL::Triangulation_vertex_base_with_info_2 < int, K > Vb;
typedef CGAL::Triangulation_face_base_with_info_2 < int, K > Fb;
typedef CGAL::Triangulation_data_structure_2 < Vb, Fb > Tds;

typedef CGAL::Delaunay_triangulation_2<K, Tds> Delaunay_triangulation_2;

typedef Delaunay_triangulation_2::Face_handle FaceHandle;
typedef Delaunay_triangulation_2::Vertex_handle VertHandle;
typedef Delaunay_triangulation_2::Vertex Vert2;
typedef Delaunay_triangulation_2::Edge_circulator EdgeCirc;

typedef CGAL::Simple_cartesian< double >  Kernel;
typedef Kernel::Line_2 KernelLine;
typedef Kernel::Point_2 KernelPoint;

typedef  CGAL::Gmpq NT;
typedef  CGAL::Cartesian< NT > K3;
typedef  CGAL::Point_2< K3 > CartesianPoint;
typedef  CGAL::Min_ellipse_2_traits_2< K3 > Traits;
typedef  CGAL::Min_ellipse_2< Traits > Min_ellipse;

#define FILE_FORMAT_REVIEW true

class MiscFunction{
public:
	static double log10Custom( double val ){
		return log10( val/* + 1.*/ );
	}
	static float log10Custom( float val ){
		return log10( val /*+ 1.f*/ );
	}
	static double invLog10Custom( double val ){
		return pow( 10, val )/* - 1.*/;
	}
};

class ObjectInterface{
public:
	enum {IntensityHistogram = 0, LengthHistogram = 1, SpeedHistogram = 2, AreaHistogram = 0, MeanDistanceHistogram = 1, MinDistanceHistogram = 2, DensityFactoryHistogram = 3, CircularityHistogram = 4, TemporalHistogram = 5, NbPointClusterHistogram = 1, DensityClusterHistogram = 2};
	enum {ProjFrame = 0, ProjMIP = 1, ProjMean = 2};

	inline ObjectInterface();
	inline ObjectInterface( const ObjectInterface & );
	virtual ~ObjectInterface(){}

	inline void setSelected(const bool _s) {m_selected = _s;}
	inline bool isSelected() const {return m_selected;}
	inline bool isDataSelected( const int index ){return m_selection[index];}
	inline bool * getSelection() const { return m_selection; }
	inline void setPalette( Palette * _palette ){if( m_palette != NULL ) delete m_palette; m_palette = _palette;}
	inline Palette * getPalette() const {return m_palette;}
	inline void determineCorrectColorTime( const int _colorNb );
	inline void setColorTime( const std::string & _ct ) {m_colorTime = _ct;}
	inline const std::string & getColorTime() const {return m_colorTime;}
	inline bool changeZBorders(const int, const int);
	inline void computeHistograms();
	inline void setTypeHistogram( const int _val ){m_typeHistogram = _val;}
	inline Histogram * getHistogram(const int _type){ return m_histograms[_type]; }
	inline void setLogHistogram( const bool );
	inline bool isLogHistogram() const{return m_histograms[m_typeHistogram]->isLog();}
	inline int whatTypeHistogram() const{return m_typeHistogram;}
	inline void setCurrentMin( const double _val ){m_histograms[m_typeHistogram]->setMin( _val );}
	inline void setCurrentMax( const double _val ){m_histograms[m_typeHistogram]->setMax( _val );}
	inline double getCurrentMin()const{return m_histograms[m_typeHistogram]->getMin();}
	inline double getCurrentMax()const{return m_histograms[m_typeHistogram]->getMax();}
	inline double getMinSpecificHistogram( const int _index )const{return m_histograms[_index]->getMin();}
	inline double getMaxSpecificHistogram( const int _index )const{return m_histograms[_index]->getMax();}
	inline uint getTotalNumObjects() const{return m_totalNumObjects;}
	inline bool isHistogramDefined() const {return !( m_histograms == NULL || ( m_histograms != NULL && m_histograms[0] == NULL ) );}
	inline int getNbFiles() const {return m_nbFiles;}
	inline uint getNbSelected() const {return m_nbSelection;}
	inline const ArrayStatistics & getStats( const int _idx ) const {return m_stats[_idx];}

	virtual void getHistogramParameters( double &, double &, double &, double &, const int, const bool ) = 0;
	virtual double * getHistogram( const int, const bool ) const = 0;
	virtual void forceRegenerateSelection() = 0;

public:
	bool m_selected, * m_selection;
	uint m_totalNumObjects, m_nbSelection;
	int m_mode, m_nbFiles, m_nbHisto, m_typeHistogram;
	std::string m_colorTime;

	Palette * m_palette;
	Histogram ** m_histograms;
	ArrayStatistics * m_stats;
};

ObjectInterface::ObjectInterface():m_selected( true ), m_totalNumObjects( 0 ), m_nbSelection( 0 ), m_nbFiles( 0 ), m_nbHisto( 0 )
{
	m_typeHistogram = ObjectInterface::IntensityHistogram;
	m_histograms = NULL;
	m_palette = NULL;
	m_selection = NULL;
	m_stats = NULL;
}

ObjectInterface::ObjectInterface( const ObjectInterface & _o ):m_selected(_o.m_selected), m_totalNumObjects(_o.m_totalNumObjects), m_nbSelection(_o.m_nbSelection), m_mode(_o.m_mode), m_colorTime(_o.m_colorTime), m_nbFiles(_o.m_nbFiles), m_nbHisto(_o.m_nbHisto), m_typeHistogram(_o.m_typeHistogram)
{
	m_palette = new Palette( *(_o.m_palette) );
	m_histograms = new Histogram*[m_nbHisto];
	for( int n = 0; n < m_nbHisto; n++ )
		m_histograms[n] = new Histogram( *_o.m_histograms[n] );
}

void ObjectInterface::computeHistograms()
{
	for( int i = 0; i < m_nbHisto; i++ ){
		if( m_histograms[i] != NULL )
			delete m_histograms[i];
		m_histograms[i] = new Histogram( this, Histogram::NORMAL, i );
	}
}

void ObjectInterface::setLogHistogram( const bool _val )
{
	for( int i = 0; i < m_nbHisto; i++ )
		m_histograms[i]->setLog( _val );
}

#endif // ObjectInterface_h__