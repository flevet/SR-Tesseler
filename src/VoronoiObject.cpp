/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      VoronoiObject.cpp
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
#include <fstream>
#include <CGAL/Polygon_2_algorithms.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/ch_graham_andrew.h>

#include <qmath.h>

#include "VoronoiObject.hpp"
#include "WrapperVoronoiDiagram.hpp"
#include "Geometry.hpp"

unsigned short VoronoiCluster::NB_DATATYPE = 7;

VoronoiCluster::VoronoiCluster()
{
	m_parent = NULL;
	m_triangles = NULL;
	m_molecules = NULL;
	m_data = NULL;
	memset(m_ellipse, 0, 5 * sizeof(double));
}

VoronoiCluster::VoronoiCluster( WrapperVoronoiDiagram * _parent )
{
	m_parent = _parent;
	m_triangles = NULL;
	m_molecules = NULL;
	m_data = NULL;
	memset(m_ellipse, 0, 5 * sizeof(double));
}

VoronoiCluster::VoronoiCluster( const VoronoiCluster & _o ):m_nbTriangles(_o.m_nbTriangles), m_nbMolecules(_o.m_nbMolecules), m_outlines( _o.m_outlines.begin(), _o.m_outlines.end() )
{
	m_parent = _o.m_parent;
	m_triangles = new FaceHandle[m_nbTriangles];
	memcpy( m_triangles, _o.m_triangles, m_nbTriangles * sizeof( FaceHandle ) );
	m_molecules = new unsigned int[m_nbMolecules];
	memcpy( m_molecules, _o.m_molecules, m_nbMolecules * sizeof( unsigned int ) );
	m_data = new double[VoronoiCluster::NB_DATATYPE];
	memcpy( m_data, _o.m_data, VoronoiCluster::NB_DATATYPE * sizeof( double ) );
	memcpy(m_ellipse, _o.m_ellipse, 5 * sizeof(double));
}

VoronoiCluster::~VoronoiCluster()
{
	if( m_triangles != NULL )
		delete [] m_triangles;
	m_triangles = NULL;
	if( m_molecules != NULL )
		delete [] m_molecules;
	m_molecules = NULL;
	if( m_data != NULL )
		delete [] m_data;
	m_data = NULL;
}

void VoronoiCluster::setTriangles( FaceHandle * _triangles, const int _nb )
{
	if( m_triangles != NULL )
		delete [] m_triangles;
	m_nbTriangles = _nb;
	m_triangles = new FaceHandle[m_nbTriangles];
	memcpy( m_triangles, _triangles, m_nbTriangles * sizeof( FaceHandle ) );
}

void VoronoiCluster::setMolecules( unsigned int * _molecules, const int _nb )
{
	if( m_molecules != NULL )
		delete [] m_molecules;
	m_nbMolecules = _nb;
	m_molecules = new unsigned int[m_nbMolecules];
	memcpy( m_molecules, _molecules, m_nbMolecules * sizeof( unsigned int ) );

	m_data = new double [VoronoiCluster::NB_DATATYPE];
	for( int n = 0; n < VoronoiCluster::NB_DATATYPE; n++ )
		m_data[n] = 0.;

	double nbTmp = m_nbMolecules, x = 0., y = 0.;
	for( int n = 0; n < m_nbMolecules; n++ ){
		int index = m_molecules[n];
		VertHandle v = m_parent->m_infos[index].getMolecule();
		x += ( v->point().x() / nbTmp );
		y += ( v->point().y() / nbTmp );
		m_data[VoronoiCluster::Area] += m_parent->m_infos[index].getData( VoronoiCluster::Area );
		m_data[VoronoiCluster::MeanDistance] += ( m_parent->m_infos[index].getData( VoronoiCluster::MeanDistance ) / nbTmp );
	}
	m_data[VoronoiCluster::LocalDensity] = nbTmp / m_data[VoronoiCluster::Area];
	m_barycenter.set( x, y );
}

void VoronoiCluster::setOutline( const std::vector < Vec2dm > & _outline )
{
	m_outlines = _outline;
}

void VoronoiCluster::fitEllipsePCA()
{
	double xc, yc, angle, axisX, axisY, diameter = 0., circ = 0., slope;
	Vec2md p0, p1;
	std::vector < KernelPoint > points;
	for( int n = 0; n < m_nbMolecules; n++ ){
		unsigned int index = m_molecules[n];
		VertHandle v = m_parent->m_infos[index].getMolecule();
		points.push_back( KernelPoint( v->point().x(), v->point().y() ) );
	}
	KernelPoint centroid = CGAL::centroid( points.begin(), points.end(), CGAL::Dimension_tag < 0 >() );
	KernelLine majorAxis;
	CGAL::linear_least_squares_fitting_2( points.begin(), points.end(), majorAxis, CGAL::Dimension_tag < 0 >() );
	slope = - ( majorAxis.a() / majorAxis.b() );
	KernelLine minorAxis = majorAxis.perpendicular( centroid );
	double * distanceToMajorAxis = new double[points.size()];
	double * distanceToMinorAxis = new double[points.size()];
	double meanDMajor = 0., meanDMinor = 0., nb = points.size();
	p0.set( centroid.x(), centroid.y() );
	p1.set( centroid.x(), centroid.y() );
	double d0 = 0., d1 = 0., dpositive = 0., dnegative = 0.;
	for( int n = 0; n < points.size(); n++ ){
		KernelPoint proj = majorAxis.projection( points[n] );
		distanceToMajorAxis[n] = sqrt( CGAL::squared_distance( proj, points[n] ) );
		if( majorAxis.has_on_positive_side( points[n] ) ){
			if( distanceToMajorAxis[n] > dpositive )
				dpositive = distanceToMajorAxis[n];
		}
		else{
			if( distanceToMajorAxis[n] > dnegative )
				dnegative = distanceToMajorAxis[n];
		}
		meanDMajor += ( distanceToMajorAxis[n] / nb );
		if( minorAxis.has_on_positive_side( proj ) ){
			double d = sqrt( CGAL::squared_distance( proj, centroid ) );
			if( d > d0 ){
				p0.set( proj.x(), proj.y() );
				d0 = d;
			}
		}
		else{
			double d = sqrt( CGAL::squared_distance( proj, centroid ) );
			if( d > d1 ){
				p1.set( proj.x(), proj.y() );
				d1 = d;
			}
		}
		proj = minorAxis.projection( points[n] );
		distanceToMinorAxis[n] = sqrt( CGAL::squared_distance( proj, points[n] ) );
		meanDMinor += ( distanceToMinorAxis[n] / nb );
	}
	ArrayStatistics statsMajor = GeneralTools::generateArrayStatistics( distanceToMajorAxis, points.size() );
	ArrayStatistics statsMinor = GeneralTools::generateArrayStatistics( distanceToMinorAxis, points.size() );
	Vec2mf v1 = p1 - p0, v2( 1.f, 0.f );
	v1.normalize();
	float dot = v1.dot( v2 );
	angle = acos( dot );
	angle = ( angle * ( 180 / M_PI ) );
	KernelLine axis( KernelPoint( 0., 0. ), KernelPoint( 1., 0. ) );
	if( axis.has_on_negative_side( KernelPoint( v1.x(), v1.y() ) ) )
		angle = 180. + 180 - angle;

	xc = centroid.x();
	yc = centroid.y();

	double dForRapport = ( dpositive > dnegative ) ? dnegative : dpositive;
	double rapport = 2.35;
	axisY = rapport * statsMajor.m_stdDev;
	axisX = rapport * statsMinor.m_stdDev;
	double major, minor;
	if( axisX > 1000 || axisY > 1000 )
		diameter = circ = 0.;
	else{
		if( axisX > axisY ){
			major = axisX;
			minor = axisY;
		}
		else{
			major = axisY;
			minor = axisX;
		}
		circ = minor / major;
		diameter = ( ( minor + major ) / 2. ) * 2.;
	}

	m_data[MajorAxis] = major * 2.;
	m_data[MinorAxis] = minor * 2.;
	m_data[Circularity] = circ;
	m_data[Diameter] = diameter;

	m_ellipse[0] = (float)xc;
	m_ellipse[1] = (float)yc;
	m_ellipse[2] = (float)angle;
	m_ellipse[3] = (float)axisX;
	m_ellipse[4] = (float)axisY;

	//std::cout << "[" << xc << ", " << yc << "], " << axisX << ", " << axisY << std::endl;
}

void VoronoiCluster::fitBoundingEllipse()
{
	CartesianPoint * pts = new CartesianPoint[m_nbMolecules];
	for (int n = 0; n < m_nbMolecules; n++){
		int index = m_molecules[n];
		VertHandle v = m_parent->m_infos[index].getMolecule();
		pts[n] = CartesianPoint(v->point().x(), v->point().y());
	}
	Min_ellipse me2(pts, pts + m_nbMolecules, true);
	double A, B, C, D, E, F;
	me2.ellipse().double_coefficients(A, C, B, D, E, F);
	double a = A, b = B / 2., c = C, d = D / 2., f = E / 2., g = F;
	double xc = (c*d - b*f) / (b*b - a*c);
	double yc = (a*f - b*d) / (b*b - a*c);
	double phi = 0.;
	if (b == 0 && a < c)
		phi = 0.;
	else if (b == 0 && a > c)
		phi = 0.5 * M_PI;
	else if (b != 0 && a < c){
		double tmp = (a - c) / (2 * b);
		phi = 0.5 * atan(1. / tmp);
	}
	else if (b != 0 && a > c){
		double tmp = (a - c) / (2 * b);
		phi = (M_PI / 2.) + .5 * atan(1. / tmp);
	}
	double angle = (phi * (180 / M_PI));
	double nom = 2. * (a * f * f + c * d * d + g * b * b - 2 * b * d * f - a * c * g);
	double denom = (b * b - a * c) * (sqrt((a - c)*(a - c) + 4 * b*b) - (a + c));
	double axisX = sqrt(nom / denom);
	denom = (b * b - a * c) * (-sqrt((a - c)*(a - c) + 4 * b*b) - (a + c));
	double axisY = sqrt(nom / denom);
	double major, minor, circ, diameter;
	if (axisX > 1000 || axisY > 1000)
		diameter = circ = 0.;
	else{
		if (axisX > axisY){
			major = axisX;
			minor = axisY;
		}
		else{
			major = axisY;
			minor = axisX;
		}
		circ = minor / major;
		diameter = ((minor + major) / 2.) * 2.;
	}

	m_data[MajorAxis] = major * 2.;
	m_data[MinorAxis] = minor * 2.;
	m_data[Circularity] = circ;
	m_data[Diameter] = diameter;

	m_ellipse[0] = (float)xc;
	m_ellipse[1] = (float)yc;
	m_ellipse[2] = (float)angle;
	m_ellipse[3] = (float)axisX;
	m_ellipse[4] = (float)axisY;
	//std::cout << axisX << ", " << axisY << ", " << m_data[Diameter] << std::endl;
}

void VoronoiCluster::draw() const
{
	glBegin( GL_LINES );
	for( unsigned int n = 0; n < m_outlines.size(); n += 2 ){
		glVertex2d( m_outlines.at( n ).x() / m_parent->m_originalWidth, m_outlines.at( n ).y() / m_parent->m_originalHeight );
		glVertex2d( m_outlines.at( n + 1 ).x() / m_parent->m_originalWidth, m_outlines.at( n + 1 ).y() / m_parent->m_originalHeight );
	}
	glEnd();
}

void VoronoiCluster::drawEllipse() const
{
	double factor = 1.;// (double)m_factorEllipse / 100.;
	glPushMatrix();
	//glLoadIdentity();
	glTranslated(m_ellipse[0] / m_parent->m_originalWidth, m_ellipse[1] / m_parent->m_originalHeight, 0.);
	glRotated(m_ellipse[2], 0., 0., 1.);
	glScaled(m_ellipse[3] / m_parent->m_originalWidth * factor, m_ellipse[4] / m_parent->m_originalHeight * factor, 0.);
	Roi::drawUnitCircle();
	glPopMatrix();
}

void VoronoiCluster::setArea( const double _area )
{
	m_data[MoleculeInfos::Area] = _area;
	m_data[MoleculeInfos::LocalDensity] = ( double )m_nbMolecules / m_data[MoleculeInfos::Area];
}

VoronoiClusterList::VoronoiClusterList():m_displayShape( true ), m_displayOutline( true )
{
	m_trianglesCell = NULL;
	m_firstVerticesTriangle = m_sizeVerticesTriangle = NULL;
}

VoronoiClusterList::~VoronoiClusterList()
{
	erase();
}

void VoronoiClusterList::draw(const Color4D & _colorShape, const Color4D & _colorOutline, const Color4D & _colorEllipse) const
{
	glPushMatrix();
	if( m_displayShape && m_trianglesCell != NULL){
		glDisable( GL_BLEND );
		glDisable( GL_CULL_FACE );
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		glColor3fv(_colorShape.getArray());
		glEnableClientState( GL_VERTEX_ARRAY );
		glVertexPointer( 2, GL_FLOAT, 0, m_trianglesCell );
		glDrawArrays( GL_TRIANGLES, 0, m_nbVertForTriangles );
		glDisableClientState( GL_VERTEX_ARRAY );
	}
	if( m_displayOutline ){
		glColor3fv(_colorOutline.getArray());
		for (VoronoiClusterList::const_iterator it = this->begin(); it != this->end(); it++){
			VoronoiCluster * cluster = *it;
			cluster->draw();
		}
	}
	glPopMatrix();
}

void VoronoiClusterList::generateDisplay()
{
	if( m_trianglesCell != NULL )
		delete [] m_trianglesCell;
	if( m_firstVerticesTriangle != NULL )
		delete [] m_firstVerticesTriangle;
	if( m_sizeVerticesTriangle != NULL )
		delete [] m_sizeVerticesTriangle;

	//Creation of the display of the molecules part of the object
	m_nbVertForTriangles = 0;
	m_nbMolClusters = 0;
	int nbVertices = 0;
	for( VoronoiClusterList::const_iterator it = this->begin(); it != this->end(); it++ ){
		VoronoiCluster * cluster = *it;
		m_nbMolClusters += cluster->m_nbMolecules;
		for( int n = 0; n < cluster->m_nbMolecules; n++ ){
			int index = cluster->m_molecules[n];
			VertHandle v = cluster->m_parent->m_infos[index].getMolecule();
			m_nbVertForTriangles += cluster->m_parent->m_sizeVerticesTriangle[index];
		}
	}

	//creation of the display for the voronoi cells part of the object
	m_trianglesCell = new Vec2mf[m_nbVertForTriangles];
	m_firstVerticesTriangle = new int[m_nbMolClusters];
	m_sizeVerticesTriangle = new int[m_nbMolClusters];
	Vec2mf * ptrT = m_trianglesCell;
	int cptT = 0, cptCell = 0;
	for( VoronoiClusterList::const_iterator it = this->begin(); it != this->end(); it++ ){
		VoronoiCluster * cluster = *it;
		for( int n = 0; n < cluster->m_nbMolecules; n++, cptCell++ ){
			int index = cluster->m_molecules[n];
			m_firstVerticesTriangle[cptCell] = cptT;
			m_sizeVerticesTriangle[cptCell] = cluster->m_parent->m_sizeVerticesTriangle[index];
			for( int i = cluster->m_parent->m_firstVerticesTriangle[index]; i < cluster->m_parent->m_firstVerticesTriangle[index] + cluster->m_parent->m_sizeVerticesTriangle[index]; i++ ){
				( *ptrT++ ).set( cluster->m_parent->m_trianglesCell[i].x(), cluster->m_parent->m_trianglesCell[i].y() );
			}
			cptT += m_sizeVerticesTriangle[cptCell];
		}
	}
}

void VoronoiClusterList::determineClusters( VoronoiCluster * _src, unsigned int * _molecules, bool * _selectionMolecules, FaceHandle * _triangles, bool * _selectionTriangles, const double _threshold, const int _minNbMolPerCluster, VoronoiClusterList & _clusters )
{
	WrapperVoronoiDiagram * voronoi = _src->m_parent;

	int nbMol = 0, nbTriangles = 0;
	bool * moleculesAboveThreshold = new bool[voronoi->nbMolecules()];
	memset( moleculesAboveThreshold, 0, voronoi->nbMolecules() * sizeof( bool ) );
	bool * selectionTrianglesForMerging = new bool[voronoi->getNbFiniteTriangles()];
	memset( selectionTrianglesForMerging, 0, voronoi->getNbFiniteTriangles() * sizeof( bool ) );
	bool * selectionTrianglesNotChanged = new bool[voronoi->getNbFiniteTriangles()];
	memset( selectionTrianglesNotChanged, 0, voronoi->getNbFiniteTriangles() * sizeof( bool ) );
	FaceHandle * allFaces = new FaceHandle[voronoi->getNbFiniteTriangles()];

	//Determination of the molecules selected by the threshold
	for( int n = 0; n < _src->nbMolecules(); n++ ){
		int index = _src->m_molecules[n];
		double val = voronoi->m_infos[index].getData( MoleculeInfos::LocalDensity );
		moleculesAboveThreshold[index] = val > _threshold;
		if( moleculesAboveThreshold[index] )
			_molecules[nbMol++] = index;
	}

	memset( _selectionTriangles, 0, voronoi->getNbFiniteTriangles() * sizeof( bool ) );
	//Determination of the triangles selected with respect to the molecules
	for( int n = 0; n < nbMol; n++ ){
		MoleculeInfos * mi = &voronoi->m_infos[_molecules[n]];
		VertHandle molecule = mi->getMolecule();
		Delaunay_triangulation_2::Face_circulator firstFace = voronoi->m_delau.incident_faces( molecule );
		Delaunay_triangulation_2::Face_circulator currentFace = firstFace;
		do{
			int index = currentFace->info();
			//Test if the triangle is infinite (index == -1) or already selected (_selectionTriangles == true )
			if( index >= 0 && !_selectionTriangles[index] ){
				int i0 = currentFace->vertex( 0 )->info(), i1 = currentFace->vertex( 1 )->info(), i2 = currentFace->vertex( 2 )->info();
				if( moleculesAboveThreshold[i0] && moleculesAboveThreshold[i1] && moleculesAboveThreshold[i2] ){
					selectionTrianglesNotChanged[index] = selectionTrianglesForMerging[index] = true;
				}
			}
			currentFace++;
		}while( currentFace != firstFace );
	}

	//Merging of the selected triangles
	for( Delaunay_triangulation_2::Finite_faces_iterator it = voronoi->m_delau.finite_faces_begin(); it != voronoi->m_delau.finite_faces_end(); it++ ){
		int index = it->info();
		if( !selectionTrianglesForMerging[index] ) continue;
		int indexQueue = 0;
		voronoi->iterativeAddCells( it, allFaces, indexQueue, selectionTrianglesForMerging );

		nbMol = 0;
		memset( _selectionMolecules, 0, voronoi->nbMolecules() * sizeof( bool ) );

		std::vector < Vec2dm > borderEdges;
		double area = 0.;
		//Now, we have a merging of triangles (not necessary all the selected triangles
		for( int i = 0; i < indexQueue; i++ ){
			FaceHandle f = allFaces[i];
			area += Geometry::getTriangleArea( f->vertex( 0 ), f->vertex( 1 ), f->vertex( 2 ) );
			int i0 = f->vertex( 0 )->info(), i1 = f->vertex( 1 )->info(), i2 = f->vertex( 2 )->info();
			unsigned int total = 0;
			if( !_selectionMolecules[i0] ){
				_selectionMolecules[i0] = true;
				_molecules[nbMol++] = i0;
			}
			if( !_selectionMolecules[i1] ){
				_selectionMolecules[i1] = true;
				_molecules[nbMol++] = i1;
			}
			if( !_selectionMolecules[i2] ){
				_selectionMolecules[i2] = true;
				_molecules[nbMol++] = i2;
			}
		
			//Determining the border edges
			for( int j = 0; j < 3; j++ ){
				FaceHandle neigh = f->neighbor( j );
				if( !selectionTrianglesNotChanged[neigh->info()] ){
					int index1 = ( j + 1 ) % 3, index2 = ( j + 2 ) % 3;
					VertHandle v1 = f->vertex( index1 ), v2 = f->vertex( index2 );
					borderEdges.push_back( Vec2dm( v1->point().x(), v1->point().y() ) );
					borderEdges.push_back( Vec2dm( v2->point().x(), v2->point().y() ) );
				}
			}	
		}

		if( nbMol >= _minNbMolPerCluster ){
			VoronoiCluster * cluster = new VoronoiCluster( voronoi );
			cluster->setTriangles( allFaces, indexQueue );
			cluster->setMolecules( _molecules, nbMol );
			cluster->setOutline( borderEdges );
			cluster->fitEllipsePCA();
			cluster->setArea( area );
			_clusters.push_back( cluster );
		}
	}

	delete [] moleculesAboveThreshold;
	delete [] selectionTrianglesForMerging;
	delete [] selectionTrianglesNotChanged;
	delete [] allFaces;
}

void VoronoiClusterList::determineClusters(WrapperVoronoiDiagram * _voronoi, bool * _polygonsSelected, const unsigned int _minLocs, const double _minArea, const unsigned int _maxLocs, const double _maxArea, VoronoiClusterList & _clusters)
{
	unsigned int nbMolVoro = _voronoi->nbMolecules();

	bool * selectionFaces = new bool[_voronoi->m_delau.number_of_faces()];
	memset( selectionFaces, 0, _voronoi->m_delau.number_of_faces() * sizeof( bool ) );
	int cpt = 0;
	for( Delaunay_triangulation_2::Finite_faces_iterator it = _voronoi->m_delau.finite_faces_begin(); it != _voronoi->m_delau.finite_faces_end(); it++, cpt++ ){
		it->info() = cpt;
		int i0 = it->vertex( 0 )->info(), i1 = it->vertex( 1 )->info(), i2 = it->vertex( 2 )->info();
		selectionFaces[cpt] = _polygonsSelected[i0] && _polygonsSelected[i1] && _polygonsSelected[i2];
	}
	bool * selectionFacesOriginal = new bool[_voronoi->m_delau.number_of_faces()];
	memcpy( selectionFacesOriginal, selectionFaces, _voronoi->m_delau.number_of_faces() * sizeof( bool ) );

	FaceHandle * allFaces = new FaceHandle[_voronoi->m_delau.number_of_faces()];
	bool * selectionMolecules = new bool[nbMolVoro];
	memset( selectionMolecules, 0, nbMolVoro * sizeof( bool ) );
	unsigned int * molecules = new unsigned int[nbMolVoro];

	for( Delaunay_triangulation_2::Finite_faces_iterator it = _voronoi->m_delau.finite_faces_begin(); it != _voronoi->m_delau.finite_faces_end(); it++ ){
		int index = it->info();
		if( !selectionFaces[index] ) continue;
		int indexQueue = 0;
		_voronoi->iterativeAddCells( it, allFaces, indexQueue, selectionFaces );

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
			//Determining the border edges
			std::vector < Vec2dm > borderEdges;
			for( int i = 0; i < indexQueue; i++ ){
				FaceHandle f = allFaces[i];
				for( int j = 0; j < 3; j++ ){
					FaceHandle neigh = f->neighbor( j );
					bool selected = (neigh->info() >= 0 );
					if( selected ) selected = selected && selectionFacesOriginal[neigh->info()];
					if( !selected ){
						int index1 = ( j + 1 ) % 3, index2 = ( j + 2 ) % 3;
						VertHandle v1 = f->vertex( index1 ), v2 = f->vertex( index2 );
						borderEdges.push_back( Vec2dm( v1->point().x(), v1->point().y() ) );
						borderEdges.push_back( Vec2dm( v2->point().x(), v2->point().y() ) );
					}
				}
			}

			VoronoiCluster * cluster = new VoronoiCluster( _voronoi );
			cluster->setTriangles( allFaces, indexQueue );
			cluster->setMolecules( molecules, nbMol );
			cluster->setOutline( borderEdges );
			cluster->fitEllipsePCA();
			cluster->setArea( area );
			_clusters.push_back( cluster );
		}
		for( int i = 0; i < indexQueue; i++ ){
			FaceHandle f = allFaces[i];
			i0 = f->vertex( 0 )->info();
			i1 = f->vertex( 1 )->info();
			i2 = f->vertex( 2 )->info();
			selectionMolecules[i0] = selectionMolecules[i1] = selectionMolecules[i2] = false;
		}
	}
	delete [] selectionFaces;
	delete [] allFaces;
	delete [] molecules;
	delete [] selectionMolecules;
	delete [] selectionFacesOriginal;
}

void VoronoiClusterList::erase()
{
	for( VoronoiClusterList::iterator it = this->begin(); it != this->end(); it++ )
		delete *it;
	this->clear();
	if( m_trianglesCell != NULL )
		delete [] m_trianglesCell;
	m_trianglesCell = NULL;
	if( m_firstVerticesTriangle != NULL )
		delete [] m_firstVerticesTriangle;
	m_firstVerticesTriangle = NULL;
	if( m_sizeVerticesTriangle != NULL )
		delete [] m_sizeVerticesTriangle;
	m_sizeVerticesTriangle = NULL;
}

VoronoiObject::VoronoiObject() :ObjectInterface(), m_outlineDisplay(true), m_filled(true), m_ellipseDisplay(true)
{
	m_parent = NULL;
	m_triangles = NULL;
	m_molecules = NULL;
	m_positionMolecules = NULL;
	m_trianglesCell = NULL;
	m_firstVerticesTriangle = m_sizeVerticesTriangle = NULL;

	m_stats = NULL;
}

VoronoiObject::VoronoiObject(WrapperVoronoiDiagram * _parent) :ObjectInterface(), VoronoiCluster(_parent), m_outlineDisplay(true), m_filled(true), m_ellipseDisplay(true)
{
	m_parent = _parent;
	m_triangles = NULL;
	m_molecules = NULL;
	m_positionMolecules = NULL;
	m_trianglesCell = NULL;
	m_firstVerticesTriangle = m_sizeVerticesTriangle = NULL;
	m_stats = NULL;
}

VoronoiObject::VoronoiObject(const VoronoiCluster & _cluster) :ObjectInterface(), VoronoiCluster(_cluster), m_outlineDisplay(true), m_filled(true), m_ellipseDisplay(true)
{
	generateStats();
	generateDisplay();
}

VoronoiObject::VoronoiObject(const VoronoiObject & _o) :ObjectInterface(_o), VoronoiCluster(_o), m_nbVertForTriangles(_o.m_nbVertForTriangles), m_outlineDisplay(_o.m_outlineDisplay), m_filled(_o.m_filled), m_ellipseDisplay(_o.m_ellipseDisplay)
{
	m_positionMolecules = new Vec2mf[m_nbMolecules];
	memcpy( m_positionMolecules, _o.m_positionMolecules, m_nbMolecules * sizeof( Vec2mf ) );
	m_trianglesCell = new Vec2mf[m_nbVertForTriangles];
	memcpy( m_trianglesCell, _o.m_trianglesCell, m_nbVertForTriangles * sizeof( Vec2mf ) );
	m_firstVerticesTriangle = new int[m_nbMolecules];
	memcpy( m_firstVerticesTriangle, _o.m_firstVerticesTriangle, m_nbMolecules * sizeof( int ) );
	m_sizeVerticesTriangle = new int[m_nbMolecules];
	memcpy( m_sizeVerticesTriangle, _o.m_sizeVerticesTriangle, m_nbMolecules * sizeof( int ) );
	m_selection = new bool[m_nbMolecules];
	m_stats = new ArrayStatistics[MoleculeInfos::NB_DATATYPE];
	for( int i = 0; i < MoleculeInfos::NB_DATATYPE; i++ )
		m_stats[i] = _o.m_stats[i];

	m_palette = Palette::getMonochromePalette( 80, 120, 249 );
	m_palette->setAutoscale( true );
	regenerateIntensityColorVector();
}

VoronoiObject::~VoronoiObject()
{
	if( m_positionMolecules != NULL )
		delete [] m_positionMolecules;
	m_positionMolecules = NULL;
	if( m_trianglesCell != NULL )
		delete [] m_trianglesCell;
	if( m_firstVerticesTriangle != NULL )
		delete [] m_firstVerticesTriangle;
	if( m_sizeVerticesTriangle != NULL )
		delete [] m_sizeVerticesTriangle;
	if( m_stats != NULL )
		delete [] m_stats;
	m_stats = NULL;
}

void VoronoiObject::setVoronoiObjectFromCluster( VoronoiCluster * _cluster )
{
	m_parent = _cluster->m_parent;
	m_nbTriangles = _cluster->m_nbTriangles;
	m_nbMolecules = _cluster->m_nbMolecules;
	m_outlines.resize( _cluster->m_outlines.size() );
	std::copy( _cluster->m_outlines.begin(), _cluster->m_outlines.end(), m_outlines.begin() );
	m_triangles = _cluster->m_triangles;
	m_molecules = _cluster->m_molecules;
	m_data = _cluster->m_data;
	generateStats();
	generateDisplay();
}

void VoronoiObject::setMolecules( unsigned int * _molecules, const int _nb )
{
	VoronoiCluster::setMolecules( _molecules, _nb );
	generateStats();
	generateDisplay();
}

void VoronoiObject::draw(const Color4D & _colorShape, const Color4D & _colorOutline, const Color4D & _colorEllipse) const
{
	glPushMatrix();
	if( m_selected ){
		glDisable( GL_BLEND );
		glDisable( GL_CULL_FACE );
		if( m_filled ){
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			glColor3fv(_colorShape.getArray());
			glEnableClientState( GL_VERTEX_ARRAY );
			glVertexPointer( 2, GL_FLOAT, 0, m_trianglesCell );
			glDrawArrays( GL_TRIANGLES, 0, m_nbVertForTriangles );
			glDisableClientState( GL_VERTEX_ARRAY );
		}
	}
	if( m_outlineDisplay ){
		glColor3fv(_colorOutline.getArray());
		glBegin(GL_LINES);
		for( unsigned int n = 0; n < m_outlines.size(); n += 2 ){
			glVertex2d( m_outlines.at( n ).x() / m_parent->m_originalWidth, m_outlines.at( n ).y() / m_parent->m_originalHeight );
			glVertex2d( m_outlines.at( n + 1 ).x() / m_parent->m_originalWidth, m_outlines.at( n + 1 ).y() / m_parent->m_originalHeight );
		}
		glEnd();
	}
	if (m_ellipseDisplay){
		glColor3fv(_colorEllipse.getArray());
		drawEllipse();
	}
	glPopMatrix();
}

void VoronoiObject::getHistogramParameters( double & _minH, double & _maxH, double & _stepX, double & _maxY, const int _typeHistogram, const bool _isLog )
{
	m_histograms[m_typeHistogram]->setParameters( _minH, _maxH, _stepX, _maxY );
}

double * VoronoiObject::getHistogram( const int _typeHistogram, const bool _isLog ) const
{
	return m_histograms[m_typeHistogram]->getHistogram();
}

void VoronoiObject::forceRegenerateSelection()
{
	determineSelection();
	regenerateIntensityColorVector();
}

void VoronoiObject::determineSelection( const bool _resetSelectionByUser /*= false */ )
{
	m_nbSelection = 0;
	if( _resetSelectionByUser )
		m_histograms[m_typeHistogram]->resetBounds();
	bool isLog = m_histograms[m_typeHistogram]->isLog();
	for( int n = 0; n < m_nbMolecules; n++ ){
		MoleculeInfos * m = &m_parent->m_infos[m_molecules[n]];
		double val1 = ( isLog ) ? m->getDataLog( m_typeHistogram ) : m->getData( m_typeHistogram );
		m_selection[n] = ( m_histograms[m_typeHistogram]->getMin() <= val1 && val1 <= m_histograms[m_typeHistogram]->getMax() );
		if( m_selection[n] )
			m_nbSelection++;
	}
}

void VoronoiObject::resetDataSelection()
{
	for( uint i = 0; i < m_nbMolecules; i++ )
		m_selection[i] = false;
}

void VoronoiObject::regenerateIntensityColorVector()
{
// 	Color4D /** ptrC = m_colors,*/ /** ptrCLine = m_colorsLine,*/ * ptrCTriangle = m_colorsTriangle;
// 	long current = 0;
// 	double minI = m_histograms[m_typeHistogram]->getMinH(), inter = m_histograms[m_typeHistogram]->getMaxH() - minI;
// 	double sizeImage = 500.;
// 	bool logHist = m_histograms[m_typeHistogram]->isLog();
// 	QImage image = m_palette->generateIndependantImage( (int)sizeImage + 1, 1, m_palette->isAutoScale() );
// 	for( int n = 0; n < m_nbMolecules; n++ ){
// 		MoleculeInfos * mi = &m_parent->m_infos[m_molecules[n]];
// 		double val = ( logHist ) ? mi->getDataLog( m_typeHistogram ) : mi->getData( m_typeHistogram );
// 		val = sizeImage * ( ( val - minI ) / inter );
// 		if( val > sizeImage )
// 			val = sizeImage;
// 		if( val < 0 )
// 			val = 0;
// 		QRgb rgb = image.pixel( val, 0 );
// 		QColor color_tmp( rgb );
// 		float alpha = ( m_selection[n] ) ? color_tmp.alphaF() : 0.f;
// 		/*for( int nc = 0; nc < m_sizeVerticesMolecules[n]; nc++ )
// 			( *ptrC++ ).set( color_tmp.redF(), color_tmp.greenF(), color_tmp.blueF(), alpha );*/
// 		/*for( int nc = 0; nc <  m_sizeVerticesLine[n]; nc++ )
// 			( *ptrCLine++ ).set( color_tmp.redF(), color_tmp.greenF(), color_tmp.blueF(), alpha );*/
// 		for( int nc = 0; nc < /*3 **/ m_sizeVerticesTriangle[n]; nc++ )
// 			( *ptrCTriangle++ ).set( color_tmp.redF(), color_tmp.greenF(), color_tmp.blueF(), alpha );
//	}
}

double VoronoiObject::getInfosData( const int _typeHisto, const int _idx ) const
{
	return m_parent->m_infos[m_molecules[_idx]].getData( _typeHisto );
}

double VoronoiObject::getInfosDataLog( const int _typeHisto, const int _idx ) const
{
	return m_parent->m_infos[m_molecules[_idx]].getDataLog( _typeHisto );

}

void VoronoiObject::generateDisplay()
{
	if( m_positionMolecules != NULL )
		delete [] m_positionMolecules;
	if( m_trianglesCell != NULL )
		delete [] m_trianglesCell;
	if( m_firstVerticesTriangle != NULL )
		delete [] m_firstVerticesTriangle;
	if( m_sizeVerticesTriangle != NULL )
		delete [] m_sizeVerticesTriangle;

	//Creation of the display of the molecules part of the object
	m_nbVertForTriangles = 0;
	m_positionMolecules = new Vec2mf[m_nbMolecules];
	for( int n = 0; n < m_nbMolecules; n++ ){
		int index = m_molecules[n];
		VertHandle v = m_parent->m_infos[index].getMolecule();
		m_positionMolecules[n].set( v->point().x(), v->point().y() );
		m_nbVertForTriangles += m_parent->m_sizeVerticesTriangle[index];
	}

	//creation of the display for the voronoi cells part of the object
	m_trianglesCell = new Vec2mf[m_nbVertForTriangles];
	m_firstVerticesTriangle = new int[m_nbMolecules];
	m_sizeVerticesTriangle = new int[m_nbMolecules];
	Vec2mf * ptrT = m_trianglesCell;
	int cptL = 0, cptT = 0;
	for( int n = 0; n < m_nbMolecules; n++ ){
		int index = m_molecules[n];
		m_firstVerticesTriangle[n] = cptT;
		m_sizeVerticesTriangle[n] = m_parent->m_sizeVerticesTriangle[index];
		for( int i = m_parent->m_firstVerticesTriangle[index]; i < m_parent->m_firstVerticesTriangle[index] + m_parent->m_sizeVerticesTriangle[index]; i++ ){
			( *ptrT++ ).set( m_parent->m_trianglesCell[i].x(), m_parent->m_trianglesCell[i].y() );
		}
		cptT += m_sizeVerticesTriangle[n];
	}

	m_selection = new bool[m_nbMolecules];
	m_nbHisto = 3;
	m_histograms = new Histogram *[m_nbHisto];
	m_histograms[0] = m_histograms[1] = m_histograms[2] = NULL;
	computeHistograms();
	if( m_palette == NULL ){
		m_palette = Palette::getMonochromePalette( 80, 120, 249 );//Palette::getMonochromePalette( (int)( ( ( float )rand() )/( ( float )RAND_MAX ) * 255. ), (int)( ( ( float )rand() )/( ( float )RAND_MAX ) * 255. ), (int)( ( ( float )rand() )/( ( float )RAND_MAX ) * 255. ) );
		m_palette->setAutoscale( true );
	}
	forceRegenerateSelection();
}

void VoronoiObject::generateStats()
{
	double ** datas = new double *[MoleculeInfos::NB_DATATYPE];
	for( int n = 0; n < MoleculeInfos::NB_DATATYPE; n++ )
		datas[n] = new double[m_nbMolecules];

	//Creation of the display of the molecules part of the object
	for( int n = 0; n < m_nbMolecules; n++ ){
		int index = m_molecules[n];
		for( int i = 0; i < MoleculeInfos::NB_DATATYPE; i++ )
			datas[i][n] = m_parent->m_infos[n].getData( i );
	}

	m_stats = new ArrayStatistics[MoleculeInfos::NB_DATATYPE];
	for( int i = 0; i < MoleculeInfos::NB_DATATYPE; i++ ){
		m_stats[i] = GeneralTools::generateArrayStatistics( datas[i], m_nbMolecules );
		delete [] datas[i];
	}
	delete [] datas;
}