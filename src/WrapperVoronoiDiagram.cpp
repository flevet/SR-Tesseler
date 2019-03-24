/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      WrapperVoronoiDiagram.cpp
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

#include <windows.h>
#include <GL/GL.h>
#include <QTime>
#include <QDir>
#include <math.h>
#include <fstream>
#include <algorithm>
#include <CGAL/ch_graham_andrew.h>
#include <qmath.h>
#include <QTime>

#include "WrapperVoronoiDiagram.hpp"
#include "SuperResObject.hpp"
#include "NeuronObject.hpp"
#include "Geometry.hpp"
#include "ImageViewer.hpp"
#include "nanoflann.hpp"

unsigned short MoleculeInfos::NB_DATATYPE = 3;

bool sortVoronoiObjects( VoronoiObject * _v1, VoronoiObject * _v2 ){
	return _v1->getArea() > _v2->getArea();
}

bool sortNeuronbjects( NeuronObject * _o1, NeuronObject * _o2 ){
	return _o1->getObject()->getArea() > _o2->getObject()->getArea();
}

WrapperVoronoiDiagram::WrapperVoronoiDiagram( DetectionPoint * _ps, const int _nb, const double _w, const double _h ):m_nbMolecules( _nb ), m_originalWidth( _w ), m_originalHeight( _h ), m_factorDensity( 2. ), m_filled( false )
{
	std::cout << "Beginning creation of the voronoi diagram" << std::endl;
	double areaImage = _w * _h;
	double nbMolecules = _nb;
	m_avgDensity = nbMolecules / areaImage;

	m_linesCell = m_trianglesCell = NULL;
	m_firstVerticesLine = m_sizeVerticesLine = m_firstVerticesTriangle = m_sizeVerticesTriangle = NULL;
	m_infos = NULL;

	m_stats = NULL;

	QTime timer;
	timer.start();
	int time2 = timer.elapsed();

	std::vector < std::pair< Point_2, int > > points;
	std::list < Point_2 > pointsHull;
	//std::cout << "# pts for voronoi: " << _nb << std::endl;
	m_nbOriginalPoints = _nb;
	points.reserve( _nb );
	for( int n = 0; n < _nb; n++ ){
		Point_2 tmp( _ps[n].x(), _ps[n].y() );
		points.push_back( std::make_pair( tmp, n ) );
		pointsHull.push_back( tmp );
	}
	m_delau.insert( points.begin(), points.end() );
	for( Delaunay_triangulation_2::All_faces_iterator it = m_delau.all_faces_begin(); it != m_delau.all_faces_end(); it++ )
		it->info() = -1;
	generateDisplay();
	/*std::ofstream fs("d:/test_locs.txt");
	for (unsigned int n = 0; n < _nb; n++)
		fs << m_infos[n].getMolecule()->point().x() << "\t" << m_infos[n].getMolecule()->point().y() << std::endl;
	fs.close();*/
	m_nbHisto = 3;
	m_histograms = new Histogram *[m_nbHisto];
	m_histograms[0] = m_histograms[1] = m_histograms[2] = NULL;
	computeHistograms();
	m_palette = Palette::getStaticLut("InvFire");
	m_palette->setAutoscale( true );
	forceRegenerateSelection();
	int elapsedTime = timer.elapsed();
	timer.restart();
	QTime test = QTime();
	test = test.addMSecs( elapsedTime );
	std::cout << "Ending creation of the voronoi diagram, elapsed time [" << test.hour() << ":" << test.minute() << ":" << test.second() << ":" << test.msec() << "] (h:min:s:ms)" << std::endl;
}

WrapperVoronoiDiagram::~WrapperVoronoiDiagram()
{
	if( m_infos != NULL )
		delete [] m_infos;
	if( m_stats != NULL )
		delete [] m_stats;
	if( m_linesCell != NULL )
		delete [] m_linesCell;
	if( m_firstVerticesLine != NULL )
		delete [] m_firstVerticesLine;
	if( m_sizeVerticesLine != NULL )
		delete [] m_sizeVerticesLine;
	if( m_trianglesCell != NULL )
		delete [] m_trianglesCell;
	if( m_firstVerticesTriangle != NULL )
		delete [] m_firstVerticesTriangle;
	if( m_sizeVerticesTriangle != NULL )
		delete [] m_sizeVerticesTriangle;
	m_stats = NULL;
}

void WrapperVoronoiDiagram::draw() const
{
	glPushMatrix();
	if( m_selected ){
		glDisable( GL_CULL_FACE );
		if( m_filled ){
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			glEnable(GL_BLEND);
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnableClientState( GL_VERTEX_ARRAY );
			glEnableClientState( GL_COLOR_ARRAY );
			glVertexPointer( 2, GL_FLOAT, 0, m_trianglesCell );
			glColorPointer( 4, GL_FLOAT, 0, m_colorsTriangle );
			glDrawArrays( GL_TRIANGLES, 0, m_nbVertForTriangles );
			glDisableClientState( GL_VERTEX_ARRAY );
			glDisableClientState( GL_COLOR_ARRAY );
			glDisable(GL_BLEND);
		}
		else{
			glEnable(GL_BLEND);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnableClientState( GL_VERTEX_ARRAY );
			glEnableClientState( GL_COLOR_ARRAY );
			glVertexPointer( 2, GL_FLOAT, 0, m_linesCell );
			glColorPointer( 4, GL_FLOAT, 0, m_colorsLine );
			glDrawArrays( GL_LINES, 0, m_nbVertForLines );
			glDisableClientState( GL_VERTEX_ARRAY );
			glDisableClientState( GL_COLOR_ARRAY );
			glDisable(GL_BLEND);
		}

		glColor3ub(0, 255, 0);
		glBegin(GL_POINTS);
		for (unsigned int n = 0; n < m_ptsLocalMax.size(); n++){
			glVertex2d(m_ptsLocalMax[n].x() / m_originalWidth, m_ptsLocalMax[n].y() / m_originalHeight);
		}
		glEnd();
	}
	glPopMatrix();
}

void WrapperVoronoiDiagram::generateDisplay()
{
	m_nbMolecules = 0;
	for( Delaunay_triangulation_2::Finite_vertices_iterator it = m_delau.finite_vertices_begin(); it != m_delau.finite_vertices_end(); it++, m_nbMolecules++ )
		it->info() = m_nbMolecules;
	if( m_infos != NULL )
		delete [] m_infos;
	m_infos = new MoleculeInfos[m_nbMolecules];

	unsigned int cptProgressB = 0;
	GeneralTools::m_imw->m_progress->setMaximum( 7 * m_nbMolecules );
	GeneralTools::m_imw->m_progress->setValue( cptProgressB++ );

	double ww = m_originalWidth, hh = m_originalHeight;

	/********* Generation of geometric informations of the molecules ***********************/
	m_delau.infinite_vertex()->info() = -1;
	//for( Delaunay_triangulation_2::All_vertices_iterator it = m_delau.all_vertices_begin(); it != m_delau.all_vertices_end(); it++ )
		//it->info() = -1;
	int cpt = 0, nbNeighbors = 0;
	for( Delaunay_triangulation_2::Finite_vertices_iterator it = m_delau.finite_vertices_begin(); it != m_delau.finite_vertices_end(); it++, cpt++ ){
		GeneralTools::m_imw->m_progress->setValue( cptProgressB++ );
		//it->info() = cpt;
		Delaunay_triangulation_2::Edge_circulator first = m_delau.incident_edges( it ), current = first;
		do{
			nbNeighbors++;
			current++;
		}while( current != first );
	}
	//std::cout << "True nb = " << cpt;
	m_edgesVoronoiPolygons = new EdgeCirc[nbNeighbors];
	int realNbNeighs = 0;
	EdgeCirc * ptrN = m_edgesVoronoiPolygons;
	cpt = 1;
	for( Delaunay_triangulation_2::Finite_vertices_iterator it = m_delau.finite_vertices_begin(); it != m_delau.finite_vertices_end(); it++, cpt++ ){
		GeneralTools::m_imw->m_progress->setValue( cptProgressB++ );
		int nb = 0;
		Delaunay_triangulation_2::Edge_circulator first = m_delau.incident_edges( it ), current = first;
		do{
			Delaunay_triangulation_2::Edge e = *current;
			if( !m_delau.is_infinite( *current ) ){
				ptrN[nb++] = current;
			}
			current++;
		}while( current != first );
		unsigned int index = it->info();
		m_infos[index].setNeighborsInfos(ptrN, nb);
		realNbNeighs += nb;
		ptrN += nb;
	}

	int trueTotal = 0;
	Iso_rectangle_2 bbox( 0, 0, ww, hh );
	int * firstVertexVoronoi = new int[m_nbMolecules];
	int * sizeVerticesVoronoi = new int[m_nbMolecules];

	m_selection = new bool[m_nbMolecules];
	for( int n = 0; n < m_nbMolecules; n++ )
		m_selection[n] = true;

	Vec2mf * verticesTmp = new Vec2mf[4 * ( 3*m_nbMolecules-6 )];
	int initialSizeArrayForLines = 2 * 4 * ( 3*m_nbMolecules-6 );
	Vec2mf * ptr = verticesTmp;
	int nbTotalVertexVoronoi = 0;
	m_nbFiniteTriangles = m_delau.number_of_faces();
	m_areaTriangles = new double[m_nbFiniteTriangles];
	cpt = 0;
	for( Delaunay_triangulation_2::Finite_faces_iterator it = m_delau.finite_faces_begin(); it != m_delau.finite_faces_end(); it++, cpt++ ){
		GeneralTools::m_imw->m_progress->setValue( cptProgressB++ );
		it->info() = cpt;
		m_areaTriangles[cpt] = Geometry::getTriangleArea( it->vertex( 0 ), it->vertex( 1 ), it->vertex(2 ) );
	}
	cpt = 0;
	double maxArea = 0., maxMeanD = 0.;
	for( Delaunay_triangulation_2::Finite_vertices_iterator it = m_delau.finite_vertices_begin(); it != m_delau.finite_vertices_end(); it++/*, cpt++*/ ){
		GeneralTools::m_imw->m_progress->setValue( cptProgressB++ );
		cpt = it->info();
		MoleculeInfos * info = &m_infos[cpt];
		firstVertexVoronoi[cpt] = nbTotalVertexVoronoi;
		int nbFaces = 0;
		const K::Segment_2 * segment_ptr;
		float area = 0.f, meanDistance = 0.;
		for( int n = 0; n < m_infos[cpt].nbEdges(); n++ ){
			CGAL::Object dual = m_delau.dual( *info->getEdge( n ) );
			if ( ( segment_ptr = CGAL::object_cast < typename K::Segment_2 >( &dual ) ) ){
				if( !bbox.has_on_bounded_side( segment_ptr->source() ) || ! bbox.has_on_bounded_side( segment_ptr->target() ) ){
					CGAL::Object obj = CGAL::intersection( *segment_ptr, bbox );
					const Segment_2 * s = CGAL::object_cast < Segment_2 >( &obj );
					if( s ){
						( *ptr++ ).set( s->target().x(), s->target().y() );
						trueTotal++;
						nbFaces++;
					}
				}
				else{
					( *ptr++ ).set( segment_ptr->target().x(), segment_ptr->target().y() );
					trueTotal++;
					nbFaces++;
				}
			}
		}
		sizeVerticesVoronoi[cpt] = nbFaces;
		nbTotalVertexVoronoi += nbFaces;
		double xc = it->point().x(), yc = it->point().y();
		for( int n = 0; n < sizeVerticesVoronoi[cpt]; n++ ){
			int index1 = firstVertexVoronoi[cpt] + n, index2 = firstVertexVoronoi[cpt] + ( ( n + 1 ) % sizeVerticesVoronoi[cpt] );
			area += Geometry::getTriangleArea( xc, yc, verticesTmp[index1].x(), verticesTmp[index1].y(), verticesTmp[index2].x(), verticesTmp[index2].y() );
			meanDistance += sqrt( ( verticesTmp[index1].x() - xc ) * ( verticesTmp[index1].x() - xc ) + ( verticesTmp[index1].y() - yc ) * ( verticesTmp[index1].y() - yc ) );
		}
		meanDistance /= ( float )sizeVerticesVoronoi[cpt];
		info->setData( MoleculeInfos::Area, area );
		info->setData( MoleculeInfos::MeanDistance, meanDistance );
		info->setDataLog( MoleculeInfos::Area, MiscFunction::log10Custom( area ) );
		info->setDataLog( MoleculeInfos::MeanDistance, MiscFunction::log10Custom( meanDistance ) );
		info->setMolecule( it );
		if( area > maxArea )
			maxArea = area;
		if( meanDistance > maxMeanD )
			maxMeanD = meanDistance;
	}
	m_area = 0.;
	for( Delaunay_triangulation_2::Finite_vertices_iterator it = m_delau.finite_vertices_begin(); it != m_delau.finite_vertices_end(); it++ ){
		GeneralTools::m_imw->m_progress->setValue( cptProgressB++ );
		int currentMolecule = it->info();
		if( m_infos[currentMolecule].getData( MoleculeInfos::Area ) == 0. ){
			m_infos[currentMolecule].setData( MoleculeInfos::Area, maxArea );
			m_infos[currentMolecule].setDataLog( MoleculeInfos::Area, MiscFunction::log10Custom( maxArea ) );
		}
		if( m_infos[currentMolecule].getData( MoleculeInfos::MeanDistance ) == 0. ){
			m_infos[currentMolecule].setData( MoleculeInfos::MeanDistance, maxMeanD );
			m_infos[currentMolecule].setDataLog( MoleculeInfos::MeanDistance, MiscFunction::log10Custom( maxMeanD ) );
		}
		m_area += m_infos[currentMolecule].getData( MoleculeInfos::Area );
	}

	double delta = ( double )m_nbMolecules / ( m_originalWidth * m_originalHeight );

	// Version of local density: adding all area of the seed and its rank-1 neighbors
	for( Delaunay_triangulation_2::Finite_vertices_iterator it = m_delau.finite_vertices_begin(); it != m_delau.finite_vertices_end(); it++ ){
		GeneralTools::m_imw->m_progress->setValue( cptProgressB++ );
		MoleculeInfos * info = &m_infos[it->info()];
		Delaunay_triangulation_2::Edge_circulator first = m_delau.incident_edges( it ), current = first;
		double totalArea = info->getData( MoleculeInfos::Area ), nb = 1. + info->nbEdges();
		for( int n = 0; n < info->nbEdges(); n++ ){
			VertHandle other = info->getNeighbor( n );
			totalArea += m_infos[other->info()].getData( MoleculeInfos::Area );
		}
		double localD = nb / totalArea;
		info->setData( MoleculeInfos::LocalDensity, localD );
		info->setDataLog( MoleculeInfos::LocalDensity, MiscFunction::log10Custom( localD ) );
	}

	Vec2mf * verticesDelaunay;
	if( trueTotal == nbTotalVertexVoronoi )
		verticesDelaunay = verticesTmp;
	else{
		verticesDelaunay = new Vec2mf[trueTotal];
		memcpy( verticesDelaunay, verticesTmp, trueTotal * sizeof( Vec2mf ) );
		delete [] verticesTmp;
		nbTotalVertexVoronoi = trueTotal;
	}

	for( int n = 0; n < nbTotalVertexVoronoi; n++ )
		verticesDelaunay[n].set( verticesDelaunay[n].x() / m_originalWidth, verticesDelaunay[n].y() / m_originalHeight );

	m_nbVertForLines = nbTotalVertexVoronoi*2;
	m_linesCell = new Vec2mf[m_nbVertForLines];
	m_firstVerticesLine = new int[m_nbMolecules];
	m_sizeVerticesLine = new int[m_nbMolecules];
	m_nbVertForTriangles = nbTotalVertexVoronoi*3;
	m_trianglesCell = new Vec2mf[m_nbVertForTriangles];
	m_firstVerticesTriangle = new int[m_nbMolecules];
	m_sizeVerticesTriangle = new int[m_nbMolecules];
	int cptTriangle = 0;
	cpt = 0;
	Vec2mf * ptrLine = m_linesCell, * ptrTriangle = m_trianglesCell;
	for( int n = 0; n < m_nbMolecules; n++ ){
		GeneralTools::m_imw->m_progress->setValue( cptProgressB++ );
		m_firstVerticesLine[n] = cpt;
		m_firstVerticesTriangle[n] = cptTriangle;
		double x = m_infos[n].getMolecule()->point().x() / m_originalWidth, y = m_infos[n].getMolecule()->point().y() / m_originalHeight;
		for( int i = 0; i < sizeVerticesVoronoi[n]; i++ ){
			int indexC = firstVertexVoronoi[n] + i, indexN = firstVertexVoronoi[n] + ( ( i + 1 ) % sizeVerticesVoronoi[n] );
			( *ptrLine++ ).set( verticesDelaunay[indexC].x(), verticesDelaunay[indexC].y() );
			( *ptrLine++ ).set( verticesDelaunay[indexN].x(), verticesDelaunay[indexN].y() );
			( *ptrTriangle++ ).set( verticesDelaunay[indexC].x(), verticesDelaunay[indexC].y() );
			( *ptrTriangle++ ).set( verticesDelaunay[indexN].x(), verticesDelaunay[indexN].y() );
			( *ptrTriangle++ ).set( x, y );
		}
		cpt += sizeVerticesVoronoi[n] * 2;
		m_sizeVerticesLine[n] = sizeVerticesVoronoi[n] * 2;
		cptTriangle += sizeVerticesVoronoi[n] * 3;
		m_sizeVerticesTriangle[n] = sizeVerticesVoronoi[n] * 3;
	}
	m_colorsLine = new Color4D[m_nbVertForLines];
	m_colorsTriangle = new Color4D[m_nbVertForTriangles];

	delete [] verticesDelaunay;
	delete [] firstVertexVoronoi;
	delete [] sizeVerticesVoronoi;

	/******* Computation of the stats for the moleculeInfos ************/
	m_stats = new ArrayStatistics[MoleculeInfos::NB_DATATYPE];
	double ** datas = new double *[MoleculeInfos::NB_DATATYPE];
	for( int n = 0; n < MoleculeInfos::NB_DATATYPE; n++ )
		datas[n] = new double[m_nbMolecules];
	for( int n = 0; n < MoleculeInfos::NB_DATATYPE; n++ )
		for( int i = 0; i < m_nbMolecules; i++ )
			datas[n][i] = m_infos[i].getData( n );
	for( int n = 0; n < MoleculeInfos::NB_DATATYPE; n++ ){
		m_stats[n] = GeneralTools::generateArrayStatistics( datas[n], m_nbMolecules );
		delete [] datas[n];
	}
	delete [] datas;
	/******************************************************************/
}

void WrapperVoronoiDiagram::regenerateIntensityColorVector()
{
	Color4D * ptrCLine = m_colorsLine, * ptrCTriangle = m_colorsTriangle;
	long current = 0;
	double minI = m_histograms[m_typeHistogram]->getMinH(), inter = m_histograms[m_typeHistogram]->getMaxH() - minI;
	double sizeImage = 500.;
	bool logHist = m_histograms[m_typeHistogram]->isLog();
	for( int n = 0; n < m_nbMolecules; n++ ){
		MoleculeInfos * mi = &m_infos[n];
		double val = ( logHist ) ? mi->getDataLog( m_typeHistogram ) : mi->getData( m_typeHistogram );
		val = ( val - minI ) / inter;
		QColor color_tmp = m_palette->getColor( val );
		float alpha = ( m_selection[n] ) ? color_tmp.alphaF() : 0.f;
		for( int nc = 0; nc < m_sizeVerticesLine[n]; nc++ )
			( *ptrCLine++ ).set( color_tmp.redF(), color_tmp.greenF(), color_tmp.blueF(), alpha );
		for( int nc = 0; nc < m_sizeVerticesTriangle[n]; nc++ )
			( *ptrCTriangle++ ).set( color_tmp.redF(), color_tmp.greenF(), color_tmp.blueF(), alpha );
	}
}

void WrapperVoronoiDiagram::getHistogramParameters( double & _minH, double & _maxH, double & _stepX, double & _maxY, const int _typeHistogram, const bool _isLog )
{
	m_histograms[m_typeHistogram]->setParameters( _minH, _maxH, _stepX, _maxY );
}

double * WrapperVoronoiDiagram::getHistogram( const int _typeHistogram, const bool _isLog ) const
{
	return m_histograms[m_typeHistogram]->getHistogram();
}
void WrapperVoronoiDiagram::determineSelection( const bool resetSelectionByUser )
{
	m_nbSelection = 0;
	if( resetSelectionByUser )
		m_histograms[m_typeHistogram]->resetBounds();
	
	bool isLog = m_histograms[m_typeHistogram]->isLog();
	for( int n = 0; n < m_nbMolecules; n++ ){
		MoleculeInfos * m = &m_infos[n];
		double val1 = ( isLog ) ? m->getDataLog( m_typeHistogram ) : m->getData( m_typeHistogram );
		m_selection[n] = ( m_histograms[m_typeHistogram]->getMin() <= val1 && val1 <= m_histograms[m_typeHistogram]->getMax() );
		if( m_selection[n] )
			m_nbSelection++;
	}
}

void WrapperVoronoiDiagram::resetDataSelection()
{
	for( uint i = 0; i < m_nbMolecules; i++ )
		m_selection[i] = false;
}

void WrapperVoronoiDiagram::forceRegenerateSelection()
{
	determineSelection();
	regenerateIntensityColorVector();
}

void WrapperVoronoiDiagram::iterativeAddCells( FaceHandle _f, FaceHandle * _allFaces, int & _indexQueue, bool * _selectionFaces )
{
	FaceHandle currentFace = _f;
	int index = currentFace->info();
	if( index == -1 ) return;
	_allFaces[_indexQueue++] = currentFace;
	_selectionFaces[index] = false;
	for( int i = 0; i < _indexQueue; i++ ){
		currentFace = _allFaces[i];
		for( int j = 0; j < 3; j++ ){
			FaceHandle nextFace = currentFace->neighbor( j );
			index = nextFace->info();
			if( index != -1 ){
				if( _selectionFaces[index] ){
					_allFaces[_indexQueue++] = nextFace;
					_selectionFaces[index] = false;
				}
			}
		}
	}
}

NeuronObjectList WrapperVoronoiDiagram::createVoronoiObjects(const double _minArea, const unsigned int _minLocs, const double _maxArea, const unsigned int _maxLocs, const bool _applyCutD, const double _cutDSqr, const bool _pca, const bool _watershed, const double _radiusWatershed, const double _nbLocsWatershed)
{
	std::ofstream fs("d:/testV1.txt");

	NeuronObjectList neuronObjects;
	m_ptsLocalMax.clear();

	bool * selectionFaces = new bool[m_delau.number_of_faces()];
	memset( selectionFaces, 0, m_delau.number_of_faces() * sizeof( bool ) );
	int cpt = 0;
	for( Delaunay_triangulation_2::Finite_faces_iterator it = m_delau.finite_faces_begin(); it != m_delau.finite_faces_end(); it++, cpt++ ){
		it->info() = cpt;
		int i0 = it->vertex( 0 )->info(), i1 = it->vertex( 1 )->info(), i2 = it->vertex( 2 )->info();
		selectionFaces[cpt] = m_selection[i0] && m_selection[i1] && m_selection[i2];
		if( selectionFaces[cpt] && _applyCutD ){
			VertHandle v0 = m_infos[i0].getMolecule(), v1 = m_infos[i1].getMolecule(), v2 = m_infos[i2].getMolecule();
			double d0 = CGAL::squared_distance( v0->point(), v1->point() ), d1 = CGAL::squared_distance( v0->point(), v2->point() ), d2 = CGAL::squared_distance( v1->point(), v2->point() );
			selectionFaces[cpt] = !( d0 > _cutDSqr || d1 > _cutDSqr || d2 > _cutDSqr );
		}
	}
	bool * selectionFacesOriginal = new bool[m_delau.number_of_faces()], *selectionFacesForOutline = new bool[m_delau.number_of_faces()];
	memcpy( selectionFacesOriginal, selectionFaces, m_delau.number_of_faces() * sizeof( bool ) );
	memset(selectionFacesForOutline, 0, m_delau.number_of_faces() * sizeof(bool));

	FaceHandle * allFaces = new FaceHandle[m_delau.number_of_faces()], *facesWatershed = new FaceHandle[m_delau.number_of_faces()];
	bool * selectionMolecules = new bool[m_nbMolecules];
	memset( selectionMolecules, 0, m_nbMolecules * sizeof( bool ) );
	unsigned int * molecules = new unsigned int[m_nbMolecules], *moleculesWaterhshed = new unsigned int[m_nbMolecules], nbMolsWatershed, nbFacesWatershed;

	memset( m_selection, 0, m_nbMolecules * sizeof( bool ) );

	printf("Creation of 0 Voronoi objects.");
	for (Delaunay_triangulation_2::Finite_faces_iterator it = m_delau.finite_faces_begin(); it != m_delau.finite_faces_end(); it++){
		int index = it->info();
		if( !selectionFaces[index] ) continue;
		int indexQueue = 0;
		iterativeAddCells( it, allFaces, indexQueue, selectionFaces );

		int nbMol = 0, i0, i1, i2;
		//Now, we have a merging of triangles (not necessary all the selected triangles
		double area = 0;
		for( int i = 0; i < indexQueue; i++ ){
			FaceHandle f = allFaces[i];
			area += Geometry::getTriangleArea( f->vertex( 0 ), f->vertex( 1 ), f->vertex( 2 ) );
			i0 = f->vertex( 0 )->info();
			i1 = f->vertex( 1 )->info();
			i2 = f->vertex( 2 )->info();
			if( !selectionMolecules[i0] ){
				selectionMolecules[i0] = true;
				molecules[nbMol++] = i0;
			}
			if( !selectionMolecules[i1] ){
				selectionMolecules[i1] = true;
				molecules[nbMol++] = i1;
			}
			if( !selectionMolecules[i2] ){
				selectionMolecules[i2] = true;
				molecules[nbMol++] = i2;
			}
		}
		if( area > _minArea && nbMol > _minLocs && area <= _maxArea && nbMol <= _maxLocs){
			//Check if watershed is selected
			if (_watershed && (nbMol > (1.5 * _nbLocsWatershed))){
				KdPointCloud_D cloud;
				cloud.m_pts.resize(nbMol);
				double x = 0, y = 0, nbD = nbMol;
				for (unsigned int n2 = 0; n2 < nbMol; n2++){
					VertHandle v = m_infos[molecules[n2]].getMolecule();
					cloud.m_pts[n2].m_x = v->point().x();
					cloud.m_pts[n2].m_y = v->point().y();
					x += cloud.m_pts[n2].m_x / nbD;
					y += cloud.m_pts[n2].m_y / nbD;
				}
				fs << "For cluster [" << x << ", " << y << "] - " << nbMol << std::endl;
				KdTree_2D_double tree(2, cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */));
				tree.buildIndex();
				double dWatershedSqr = _radiusWatershed * _radiusWatershed;
				const double search_radius = static_cast<double>(dWatershedSqr);
				std::vector<std::pair<std::size_t, double> > ret_matches;
				nanoflann::SearchParams params;
				std::size_t nMatches;
				std::vector <std::pair< unsigned int, unsigned int >> indexesSup;
				indexesSup.resize(nbMol);
				for (unsigned int n2 = 0; n2 < nbMol; n2++){
					const double queryPt[2] = { cloud.m_pts[n2].m_x, cloud.m_pts[n2].m_y };
					indexesSup[n2].first = n2;
					indexesSup[n2].second = tree.radiusSearch(&queryPt[0], search_radius, ret_matches, params);
					//std::cout << "[" << cloud.m_pts[n2].m_x << ", " << cloud.m_pts[n2].m_y << "] ------> " << indexesSup[n2].second << std::endl;
				}
				std::sort(indexesSup.begin(), indexesSup.end(), boost::bind(&std::pair<unsigned int, unsigned int>::second, _1) > boost::bind(&std::pair<unsigned int, unsigned int>::second, _2));
				double limitNb = _nbLocsWatershed * 0.8;
				std::vector < unsigned int > localMaxima;
				localMaxima.push_back(indexesSup[0].first);
				unsigned int current = 1;
				//std::cout << __LINE__ << std::endl;
				while (indexesSup[current].second > limitNb){
					bool tooclose = false;
					unsigned int index1 = indexesSup[current].first, nKeep;
					double d;
					for (unsigned int n = 0; n < localMaxima.size() && !tooclose; n++){
						unsigned int index2 = localMaxima[n];
						d = Geometry::distanceSqr(cloud.m_pts[index1].m_x, cloud.m_pts[index1].m_y, cloud.m_pts[index2].m_x, cloud.m_pts[index2].m_y);
						tooclose = (d < dWatershedSqr);
						nKeep = n;
					}
					if (!tooclose)
						localMaxima.push_back(index1);
					current++;
				}
				for (unsigned int n = 0; n < localMaxima.size(); n++){
					unsigned int index = localMaxima[n];
					m_ptsLocalMax.push_back(Vec2md(cloud.m_pts[index].m_x, cloud.m_pts[index].m_y));
					fs << "Local max = [" << cloud.m_pts[index].m_x << ", " << cloud.m_pts[index].m_y << "]" << std::endl;;
				}

				//std::cout << __LINE__ << std::endl;
				bool * closers = new bool[nbMol];
				memset(closers, 0, nbMol * sizeof(bool));
				unsigned int index = localMaxima[0];
				for (unsigned int n = 0; n < nbMol; n++){
					bool closer = true;
					double d1 = Geometry::distanceSqr(cloud.m_pts[index].m_x, cloud.m_pts[index].m_y, cloud.m_pts[n].m_x, cloud.m_pts[n].m_y);
					for (unsigned int i = 1; i < localMaxima.size() && closer; i++){
						unsigned int index2 = localMaxima[i];
						double d2 = Geometry::distanceSqr(cloud.m_pts[n].m_x, cloud.m_pts[n].m_y, cloud.m_pts[index2].m_x, cloud.m_pts[index2].m_y);
						closer = d1 < d2;
					}
					closers[n] = closer;
				}

				//std::cout << __LINE__ << std::endl;
				for (int i = 0; i < indexQueue; i++){
					FaceHandle f = allFaces[i];
					i0 = f->vertex(0)->info();
					i1 = f->vertex(1)->info();
					i2 = f->vertex(2)->info();
					selectionMolecules[i0] = selectionMolecules[i1] = selectionMolecules[i2] = false;
				}

				//std::cout << __LINE__ << std::endl;
				nbMolsWatershed = 0;
				for (unsigned int n = 0; n < nbMol; n++){
					if (closers[n]){
						moleculesWaterhshed[nbMolsWatershed] = molecules[n];
						nbMolsWatershed++;
						selectionMolecules[molecules[n]] = true;
					}
				}

				//std::cout << __LINE__ << std::endl;
				nbFacesWatershed = 0;
				area = 0.;
				for (int i = 0; i < indexQueue; i++){
					FaceHandle f = allFaces[i];
					i0 = f->vertex(0)->info();
					i1 = f->vertex(1)->info();
					i2 = f->vertex(2)->info();
					if (selectionMolecules[i0] && selectionMolecules[i1] && selectionMolecules[i2]){
						area += Geometry::getTriangleArea(f->vertex(0), f->vertex(1), f->vertex(2));
						facesWatershed[nbFacesWatershed++] = f;
					}
					//else
						//selectionFaces[f->info()] = true;
				}
				//std::cout << __LINE__ << std::endl;
				for (int i = 0; i < indexQueue; i++){
					FaceHandle f = allFaces[i];
					selectionFaces[f->info()] = true;
				}
				//std::cout << __LINE__ << std::endl;
				for (int i = 0; i < nbMolsWatershed; i++){
					VertHandle v = m_infos[moleculesWaterhshed[i]].getMolecule();
					Delaunay_triangulation_2::Face_circulator firstFace = m_delau.incident_faces(v);
					Delaunay_triangulation_2::Face_circulator currentFace = firstFace;
					do{
						int index = currentFace->info();
						//Test if the triangle is infinite (index == -1)
						if (index >= 0)
							selectionFaces[index] = false;
						currentFace++;
					} while (currentFace != firstFace);

					/*FaceHandle f = facesWatershed[i];
					selectionFaces[f->info()] = false;
					for (int j = 0; j < 3; j++){
							FaceHandle neigh = f->neighbor(j);
							if (neigh->info() >= 0)
								selectionFaces[neigh->info()] = false;
					}*/
				}
				//std::cout << __LINE__ << std::endl;
				indexQueue = nbFacesWatershed;
				memcpy(allFaces, facesWatershed, indexQueue * sizeof(FaceHandle));
				nbMol = nbMolsWatershed;
				memcpy(molecules, moleculesWaterhshed, nbMolsWatershed * sizeof(unsigned int));
			}

			for (int i = 0; i < indexQueue; i++){
				FaceHandle f = allFaces[i];
				selectionFacesForOutline[f->info()] = true;
			}

			/*std::cout << "*********************\nMolecules of cluster:\n";
			for (unsigned int n = 0; n < nbMol; n++){
				VertHandle v = m_infos[molecules[n]].getMolecule();
				std::cout << "[" << v->point().x() << ", " << v->point().y() << "]" << std::endl;
			}*/

			//Determining the border edges
			std::vector < Vec2dm > borderEdges;
			for( int i = 0; i < indexQueue; i++ ){
				FaceHandle f = allFaces[i];
				for( int j = 0; j < 3; j++ ){
					FaceHandle neigh = f->neighbor( j );
					bool selected = (neigh->info() >= 0 );
					if (selected) selected = selected && selectionFacesForOutline[neigh->info()];// selectionFacesOriginal[neigh->info()];
					if( !selected ){
						int index1 = ( j + 1 ) % 3, index2 = ( j + 2 ) % 3;
						VertHandle v1 = f->vertex( index1 ), v2 = f->vertex( index2 );
						borderEdges.push_back( Vec2dm( v1->point().x(), v1->point().y() ) );
						borderEdges.push_back( Vec2dm( v2->point().x(), v2->point().y() ) );
					}
				}
			}
			
			VoronoiObject * obj = new VoronoiObject( this );
			obj->setTriangles( allFaces, indexQueue );
			obj->setMolecules( molecules, nbMol );
			obj->setOutline( borderEdges );
			if (_pca)
				obj->fitEllipsePCA();
			else
				obj->fitBoundingEllipse();
			obj->setArea( area );
			neuronObjects.push_back( new NeuronObject( obj ) );

			printf("\rCreation of %i Voronoi objects.", neuronObjects.size());

			for( int i = 0; i < indexQueue; i++ ){
				FaceHandle f = allFaces[i];
				m_selection[f->vertex( 0 )->info()] = true;
				m_selection[f->vertex( 1 )->info()] = true;
				m_selection[f->vertex( 2 )->info()] = true;
			}
		}
		for( int i = 0; i < indexQueue; i++ ){
			FaceHandle f = allFaces[i];
			i0 = f->vertex( 0 )->info();
			i1 = f->vertex( 1 )->info();
			i2 = f->vertex( 2 )->info();
			selectionMolecules[i0] = selectionMolecules[i1] = selectionMolecules[i2] = false;
			selectionFacesForOutline[f->info()] = true;
		}
	}

	regenerateIntensityColorVector();
	std::sort( neuronObjects.begin(), neuronObjects.end(), sortNeuronbjects );

	delete [] selectionFaces;
	delete [] allFaces;
	delete [] molecules;
	delete [] selectionMolecules;
	delete [] selectionFacesOriginal;

	delete[] facesWatershed;

	fs.close();

	return neuronObjects;
}

const double WrapperVoronoiDiagram::getMeanDensityFromSelectedLocalizations( unsigned int * _selectedMolecules, const unsigned int _nbMolecules ) const
{
	double totalArea = 0;
	for( unsigned int n = 0; n < _nbMolecules; n++ ){
		unsigned int index = _selectedMolecules[n];
		totalArea += this->getData( MoleculeInfos::Area, index );
	}
	return ( double )_nbMolecules / totalArea;
}

void WrapperVoronoiDiagram::applyDensityFactorROIs( const double _factor, const bool _deltaOnROIs, const bool _selectionOnROIs, const RoiList & _rois )
{
	unsigned int cpt = 0;
	GeneralTools::m_imw->m_progress->setMaximum( 2 * m_nbMolecules );
	GeneralTools::m_imw->m_progress->setValue( cpt++ );

	double thresh = _factor * m_avgDensity, nbInsideROIs = 0., areaInsideROIs = 0.;

	for( unsigned int n = 0; n < m_nbMolecules; n++ ){
		GeneralTools::m_imw->m_progress->setValue( cpt++ );
		VertHandle v = m_infos[n].getMolecule();
		m_selection[n] = true;
		if( !_rois.empty() && _selectionOnROIs ){
			m_selection[n] = false;
			for( RoiList::const_iterator it = _rois.begin(); it != _rois.end() && !m_selection[n]; it++ ){
				const Roi & roi = *it;
				m_selection[n] = roi.inside( v->point().x(), v->point().y() );
			}
		}
		if( m_selection[n] ){
			nbInsideROIs++;
			areaInsideROIs += m_infos[n].getData( MoleculeInfos::Area );
		}
	}

	if( _deltaOnROIs )
		thresh = _factor * ( nbInsideROIs / areaInsideROIs );

	for( unsigned n = 0; n < m_nbMolecules; n++ ){
		GeneralTools::m_imw->m_progress->setValue( cpt++ );
		if( !m_selection[n] ) continue;
		m_selection[n] = m_infos[n].getData( MoleculeInfos::LocalDensity ) > thresh;
	}

	regenerateIntensityColorVector();
}