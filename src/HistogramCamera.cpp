/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      HistogramCamera.cpp
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

#include <Windows.h>
#include <QCursorShape>
#include <QMouseEvent>
#include <QFileDialog>
#include <float.h>
#include <gl/GLU.h>
#include <fstream>
#include "HistogramCamera.hpp"
#include "ObjectInterface.hpp"
#include "WrapperVoronoiDiagram.hpp"
#include "Camera2D.hpp"
#include "DetectionSet.hpp"

HistogramCamera::HistogramCamera( ObjectInterface * _data, Camera2D * _cam, QWidget * _parent /*= 0*/, const QGLWidget * _shareWidget /*= 0*/, Qt::WindowFlags _f /*= 0 */ ):QGLWidget( _parent, _shareWidget, _f )
{	
	m_camera = _cam;
	this->setObjectName( "HistogramCamera" );
	m_cursorX = m_cursorY = 0.;
	m_minX = m_minY = m_currentMin = 0.;
	m_maxX = m_maxY = m_currentMax = 1.;
	m_zoomx1 = m_zoomx2 = m_zoomy1 = m_zoomy2 = 0.;
	m_insidePalette = m_buttonLeft = m_buttonRight = m_zoomEnabled = false;

	m_indexHistograms = 0;
	m_data = _data;

	if( _data->isHistogramDefined() ){
		double min = _data->getCurrentMin(), max = _data->getCurrentMax();
		double minH, maxH, maxY, stepX;
		_data->getHistogramParameters( minH, maxH, stepX, maxY, _data->whatTypeHistogram(), _data->isLogHistogram() );
		double inter = maxH - minH;
		min -= minH;
		max -= minH;
		m_currentMin = min / inter;
		m_currentMax = max / inter;
	}

	setCurrentMin( _data );
	setCurrentMax( _data );

	setMouseTracking( true );
	setAutoFillBackground( false );
}	

HistogramCamera::~HistogramCamera()
{
}

void HistogramCamera::initializeGL()
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

void HistogramCamera::paintGL()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glViewport(0, 0, width(), height());
	gluOrtho2D(m_minX, m_maxX, m_minY, m_maxY);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(.7f, .7f, .7f, 1.f);
	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	if( !m_data->isHistogramDefined() ){
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		return;
	}

	int cpt = 0;
	double minHistos = DBL_MAX, maxHistos = DBL_MIN, maxYHistos = DBL_MIN;
	cpt++;
	double minH, maxH, maxY, stepX;
	m_data->getHistogramParameters( minH, maxH, stepX, maxY, m_data->whatTypeHistogram(), m_data->isLogHistogram() );
	if( minH < minHistos ) minHistos = minH;
	if( maxH > maxHistos ) maxHistos = maxH;
	if( maxY > maxYHistos ) maxYHistos = maxY;

	if( cpt == 0 )
	{
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		return;
	}
	/*** Computation of the height of the color bar, 25 pix is the minimum height ***/
	m_paletteY = 25. / ( double )height();
	double diff = 1. - m_paletteY;

	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth( .5f );
	double w = ( double )Histogram::BINS;
	double * hist = m_data->getHistogram( m_data->whatTypeHistogram(), m_data->isLogHistogram() );
	m_data->getHistogramParameters( minH, maxH, stepX, maxY, m_data->whatTypeHistogram(), m_data->isLogHistogram() );
	for( int i = 0; i < Histogram::BINS; i++ )
	{
		glLineWidth( 1.f );
		QColor color = m_data->getPalette()->getColor( ( qreal )i / ( qreal )Histogram::BINS );
		double x1b = minH + ( double )i * stepX, x1;
		double x2b = minH + ( double )( i + 1 ) * stepX, x2;
		x1 = ( ( x1b - minHistos ) / ( maxHistos - minHistos ) );//determineCoordInWindow
		x2 = ( ( x2b - minHistos ) / ( maxHistos - minHistos ) );//determineCoordInWindow
		double y = m_paletteY + ( hist[i] / maxYHistos * diff );
		glColor4f( color.redF(), color.greenF(), color.blueF(), /*0.5f*/1.f );
		glBegin( GL_QUADS );
		glVertex2d( x1, m_paletteY );
		glVertex2d( x1, y );
		glVertex2d( x2, y );
		glVertex2d( x2, m_paletteY );
		glEnd();
		glColor4f( color.redF(), color.greenF(), color.blueF(), 1.f );
		glBegin( GL_QUADS );
		glVertex2d( x1, 0. );
		glVertex2d( x1, m_paletteY );
		glVertex2d( x2, m_paletteY );
		glVertex2d( x2, 0. );
		glEnd();
	}
	double currentMinb = m_data->getCurrentMin(), currentMaxb = m_data->getCurrentMax(), currentMin, currentMax;
	
	currentMin = ( ( currentMinb - minHistos ) / ( maxHistos - minHistos ) );//determineCoordInWindow
	currentMax = ( ( currentMaxb - minHistos ) / ( maxHistos - minHistos ) );//determineCoordInWindow
	glColor4f( 0.f, 0.f, 0.f, 1.f );
	glLineWidth( 4.f );
	glBegin( GL_LINES );
	glVertex2d( currentMin, m_paletteY );
	glVertex2d( currentMin, 1. );
	glVertex2d( currentMax, m_paletteY );
	glVertex2d( currentMax, 1. );
	glEnd();	

		glLineWidth( 1.f );
	glColor4f( 0.f, 0.f, 0.f, 1.f );
	glBegin( GL_LINES );
	glVertex2d( 0., m_paletteY );
	glVertex2d( 1., m_paletteY );
	glEnd();
	QColor c( Qt::darkGreen );
	glColor4f( c.redF(), c.greenF(), c.blueF(), c.alphaF() );
	glBegin( GL_LINE_STRIP );
	glVertex2d( m_zoomx1, m_zoomy1 );
	glVertex2d( m_zoomx2, m_zoomy1 );
	glVertex2d( m_zoomx2, m_zoomy2 );
	glVertex2d( m_zoomx1, m_zoomy2 );
	glVertex2d( m_zoomx1, m_zoomy1 );
	glEnd();
	
	hist = m_data->getHistogram( m_data->whatTypeHistogram(), m_data->isLogHistogram() );
	m_data->getHistogramParameters( minH, maxH, stepX, maxY, m_data->whatTypeHistogram(), m_data->isLogHistogram() );
	glColor4f( 1.f, 1.f, 1.f, 1.f );
	double intensity = ( maxH - minH ) * m_cursorX + minH;
	double count = hist[(int)(m_cursorX * Histogram::BINS)];
	int realCurrentMin , realCurrentMax, valueMin, valueMax;
	if( m_data->isLogHistogram() )
	{
		realCurrentMin = MiscFunction::invLog10Custom( ( ( maxH - minH ) * m_currentMin + minH ) );
		realCurrentMax = MiscFunction::invLog10Custom( ( ( maxH - minH ) * m_currentMax + minH ) );
		valueMin = MiscFunction::invLog10Custom( minH );
		valueMax = MiscFunction::invLog10Custom( maxH );
		intensity = MiscFunction::invLog10Custom( intensity );
	}
	else
	{
		realCurrentMin = ( int )( ( maxH - minH ) * m_currentMin + minH );
		realCurrentMax = ( int )( ( maxH - minH ) * m_currentMax + minH );
		valueMin = minH;
		valueMax = maxH;
	}
	const ArrayStatistics & stats = m_data->getStats( m_data->whatTypeHistogram() );
	QString text2( "Count [0, " + QString::number( maxY ) + "]" );
	QString text3( "Bounds [" + QString::number( realCurrentMin ) + ", " + QString::number( realCurrentMax ) + "]" );
	QString text( "Mean [" + QString::number( stats.m_mean ) + "]" );
	QString text5( "Median [" + QString::number( stats.m_median ) + "]" );
	QString text6( "Std dev [" + QString::number( stats.m_stdDev ) + "]" );
	QString text4( "[" + QString::number( intensity ) + ", " + QString::number( count ) + "]" );
	QFont font( "Times", 10, QFont::Bold );
	QFontMetrics fm( font );
	int textW1 = fm.width( text ), textW2 = fm.width( text2 ), textW3 = fm.width( text3 ), textW4 = fm.width( text4 ),textH = fm.height();
	textW1 = ( textW1 > textW2 ) ? textW1 : textW2;
	textW1 = ( textW1 > textW3 ) ? textW1 : textW3;
	textW1 = ( textW1 > textW4 ) ? textW1 : textW4;
	renderText( width() - textW1 - 10, 15, text2, font );
	renderText( width() - textW1 - 10, 15 + textH, text3, font );
	renderText( width() - textW1 - 10, 15 + 2 * textH, text, font );
	renderText( width() - textW1 - 10, 15 + 3 * textH, text5, font );
	renderText( width() - textW1 - 10, 15 + 4 * textH, text6, font );

	glDisable( GL_BLEND );
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void HistogramCamera::mousePressEvent( QMouseEvent * _event )
{
	switch( _event->button() )
	{
	case Qt::LeftButton:
		{
			if( ! m_insidePalette )
			{
				m_zoomEnabled = _event->modifiers() == Qt::ControlModifier || _event->modifiers() == Qt::ShiftModifier;
				if( m_zoomEnabled )
				{
					determineCoordInWorld( _event->x(), _event->y(), m_zoomx1, m_zoomy1 );
					m_zoomx2 = m_zoomx1;
					m_zoomy2 = m_zoomy1;
				}
				else
				{
					m_buttonLeft = true;
					m_currentMin = determineCoordInWorldX( _event->x() );
					if( m_currentMin > m_currentMax ) m_currentMin = m_currentMax;
					setCurrentMin( m_data );
					m_camera->updateGL();
				}
			}
			break;
		}
	case Qt::RightButton:
		{
			if( ! m_insidePalette )
			{
				bool resetZoom = _event->modifiers() == Qt::ControlModifier || _event->modifiers() == Qt::ShiftModifier;
				if( resetZoom )
				{
					m_minX = 0.;
					m_maxX = 1.;
				}
				else
				{
					m_buttonRight = true;
					m_currentMax = determineCoordInWorldX( _event->x() );
					if( m_currentMax < m_currentMin ) m_currentMax = m_currentMin;
					setCurrentMax( m_data );
					m_camera->updateGL();
				}
			}
			break;
		}
	default:
		break;
	}
	repaint();
}

void HistogramCamera::mouseMoveEvent ( QMouseEvent * _event )
{
	double h = ( double )height();
	int x  = _event->x(), y = _event->y();
	if( x < 0 ) 
		x = 0;
	if( x >= width() ) 
		x = width() - 1;
	if( y < 0 ) 
		y = 0;
	if( y >= height() ) 
		y = height() - 1;
	determineCoordInWorld( x, y, m_cursorX, m_cursorY );
	bool inside = ( 0. <= m_cursorY && m_cursorY <= m_paletteY );
	if( inside != m_insidePalette )
	{
		if( inside )
			setCursor( Qt::PointingHandCursor );
		else
			setCursor( Qt::ArrowCursor );
		m_insidePalette = inside;
	}
	if( !m_insidePalette && m_zoomEnabled )
		determineCoordInWorld( x, y, m_zoomx2, m_zoomy2 );
	if( !m_insidePalette && m_buttonLeft )
	{
		m_currentMin = determineCoordInWorldX( x );
		if( m_currentMin > m_currentMax ) m_currentMin = m_currentMax;
		setCurrentMin( m_data );
		m_camera->updateGL();
	}
	if( !m_insidePalette && m_buttonRight )
	{
		m_currentMax = determineCoordInWorldX( x );
		if( m_currentMax < m_currentMin ) m_currentMax = m_currentMin;
		setCurrentMax( m_data );
		m_camera->updateGL();
	}
	repaint();
}

void HistogramCamera::mouseReleaseEvent( QMouseEvent * _event )
{
	switch( _event->button() )
	{
	case Qt::LeftButton:
		{
			if( m_zoomEnabled )
			{
				if( m_zoomx1 < m_zoomx2 )
				{
					m_minX = m_zoomx1;
					m_maxX = m_zoomx2;
				}
				else
				{
					m_minX = m_zoomx2;
					m_maxX = m_zoomx1;
				}
			}
			m_zoomx1 = m_zoomx2 = m_zoomy1 = m_zoomy2 = 0.;
			m_buttonLeft = m_zoomEnabled = false;
			break;
		}
	case Qt::RightButton:
		{
			m_buttonRight = false;
			break;
		}
	default:
		break;
	}
	repaint();
}

void HistogramCamera::determineCoordInWorld( const int _x, const int _y, double & dx, double & dy )
{
	dx = determineCoordInWorldX( _x );
	dy = determineCoordInWorldY( _y );
}

double HistogramCamera::determineCoordInWorldX( const int _x )
{
	double realX = _x;
	realX /= ( double )width();
	double diffX = m_maxX - m_minX;
	return ( m_minX + diffX * realX );
}

double HistogramCamera::determineCoordInWorldY( const int _y )
{
	double realY = height() - _y;
	realY /= ( double )height();
	double diffY = m_maxY - m_minY;
	return ( m_minY + diffY * realY );
}

void HistogramCamera::setLog( bool _val )
{
	if( m_data->isHistogramDefined() ){
		m_data->setLogHistogram( _val );
		if( m_data->getPalette()->isAutoScale() )
			m_data->getPalette()->setPaletteAutoscale( m_currentMin, m_currentMax );
		m_data->forceRegenerateSelection();
	}
	if( m_data->isHistogramDefined() ){
		double minH, maxH, maxY, stepX;
		m_data->getHistogramParameters( minH, maxH, stepX, maxY, m_data->whatTypeHistogram(), m_data->isLogHistogram() );
		m_data->setCurrentMin( minH + m_currentMin * ( maxH - minH ) );
		m_data->setCurrentMax( minH + m_currentMax * ( maxH - minH ) );
	}
	repaint();
	m_camera->updateGL();
}

double HistogramCamera::setMinHistogram( double _min )
{
	double minH, maxH, maxY, stepX;
	m_data->getHistogramParameters( minH, maxH, stepX, maxY, m_data->whatTypeHistogram(), m_data->isLogHistogram() );
	if( minH == _min ) return _min;
	if( _min < minH )
		_min = minH;
	m_currentMin = ( _min - minH ) / ( maxH - minH );
	if( m_currentMin > m_currentMax )
		m_currentMin = m_currentMax;
	setCurrentMin( m_data );
	repaint();
	m_camera->updateGL();
	return _min;
}

void HistogramCamera::setCurrentMin( ObjectInterface * _data )
{
	if( !_data->isHistogramDefined() ) return;
	if( _data->getPalette()->isAutoScale() )
		_data->getPalette()->setPaletteAutoscale( m_currentMin, m_currentMax );
	double minH, maxH, maxY, stepX;
	_data->getHistogramParameters( minH, maxH, stepX, maxY, _data->whatTypeHistogram(), _data->isLogHistogram() );
	_data->setCurrentMin( minH + m_currentMin * ( maxH - minH ) );
	_data->forceRegenerateSelection();
}

double HistogramCamera::setMaxHistogram( double _max )
{
	double minH, maxH, maxY, stepX;
	m_data->getHistogramParameters( minH, maxH, stepX, maxY, m_data->whatTypeHistogram(), m_data->isLogHistogram() );
	if( maxH == _max ) return _max;
	if( _max > maxH )
		_max = maxH;
	m_currentMax = ( _max - minH ) / ( maxH - minH );
	setCurrentMax( m_data );
	repaint();
	m_camera->updateGL();
	return _max;
}

void HistogramCamera::setCurrentMax( ObjectInterface * _data )
{
	if( !_data->isHistogramDefined() ) return;
	if( _data->getPalette()->isAutoScale() )
		_data->getPalette()->setPaletteAutoscale( m_currentMin, m_currentMax );
	double minH, maxH, maxY, stepX;
	_data->getHistogramParameters( minH, maxH, stepX, maxY, _data->whatTypeHistogram(), _data->isLogHistogram() );
	_data->setCurrentMax( minH + m_currentMax * ( maxH - minH ) );
	_data->forceRegenerateSelection();
}

void HistogramCamera::changeDataSelected( bool _val )
{
	m_data->setSelected( _val );
	repaint();
	m_camera->updateGL();
}

void HistogramCamera::changeDataSelected( bool _val, Camera2D * _cam )
{
	m_camera = _cam;
	m_data->setSelected( _val );
	repaint();
	m_camera->updateGL();
}

void HistogramCamera::changeDataSelected( ObjectInterface * _obj, Camera2D * _cam )
{
	m_data = _obj;
	m_camera = _cam;
	if( m_data->isHistogramDefined() ){
		double minH, maxH, maxY, stepX;
		m_data->getHistogramParameters( minH, maxH, stepX, maxY, m_data->whatTypeHistogram(), m_data->isLogHistogram() );
	}
	repaint();
}


void HistogramCamera::regenerateDataSelected()
{
	if( m_data->isHistogramDefined() ){
		double minH, maxH, maxY, stepX;
		m_data->getHistogramParameters( minH, maxH, stepX, maxY, m_data->whatTypeHistogram(), m_data->isLogHistogram() );
	}

	repaint();
}

void HistogramCamera::changeTypeHisto( int _val )
{
	if( m_data->isHistogramDefined() ){
		m_data->setTypeHistogram( _val );
		if( m_data->getPalette()->isAutoScale() )
			m_data->getPalette()->setPaletteAutoscale( m_currentMin, m_currentMax );
		m_data->forceRegenerateSelection();
	}
	if( m_data->isHistogramDefined() ){
		double minH, maxH, maxY, stepX;
		m_data->getHistogramParameters( minH, maxH, stepX, maxY, m_data->whatTypeHistogram(), m_data->isLogHistogram() );
		m_data->setCurrentMin( minH + m_currentMin * ( maxH - minH ) );
		m_data->setCurrentMax( minH + m_currentMax * ( maxH - minH ) );
	}
	repaint();
	m_camera->updateGL();
}

void HistogramCamera::setPolygonFilled( bool _val )
{
	WrapperVoronoiDiagram * delaunay = dynamic_cast < WrapperVoronoiDiagram * > ( m_data );
	VoronoiObject * object = dynamic_cast < VoronoiObject * >( m_data );
	if( delaunay )
		delaunay->setPolygonFilled( _val );
	else if( object )
		object->setPolygonFilled( _val );
	m_camera->updateGL();
}

void HistogramCamera::setOutlineDisplay( bool _val )
{
	VoronoiObject * object = dynamic_cast < VoronoiObject * >( m_data );
	if( object )
		object->setOutlineDisplay( _val );
	m_camera->updateGL();
}

void HistogramCamera::changeLut( const QString & _nameLut )
{
	Palette * dataPalette = m_data->getPalette(), * palette = Palette::getStaticLut( _nameLut );

	palette->setAutoscale( dataPalette->isAutoScale() );
	QLinearGradient gradient;
	const QGradientStops& stops = palette->linearGradient().stops();
	for( int i = 0 ; i < stops.size(); ++i )
		gradient.setColorAt( stops[i].first, stops[i].second );
	QLinearGradient gradientAutoS;
	const QGradientStops& stopsAutoS = palette->linearGradientAutoscale().stops();
	for( int i = 0 ; i < stopsAutoS.size(); ++i )
		gradientAutoS.setColorAt( stopsAutoS[i].first, stopsAutoS[i].second );
	dataPalette->setGradient( gradient );
	dataPalette->setGradientAutoscale( gradientAutoS );

	repaint();
	m_data->forceRegenerateSelection();
	m_camera->updateGL();
}

void HistogramCamera::saveDataHistogram(const QString & _typeHisto, const int _typeI)
{
	QString nameXls(m_camera->getSuperResObject()->getDir().c_str());
	nameXls.append(_typeHisto).append(".txt");
	std::cout << "Saving values of " << _typeHisto.toAscii().data() << " histogram in " << nameXls.toAscii().data() << std::endl;

	QString filename = QFileDialog::getSaveFileName(this, QObject::tr("Save feature values..."), nameXls, QObject::tr("Txt (*.txt)"), 0, QFileDialog::DontUseNativeDialog);
	if (filename.isEmpty()) return;

	std::ofstream fs(nameXls.toAscii().data());

	if (_typeHisto == "Intensity"){

	}
	if (_typeHisto == "LocalDensity" || _typeHisto == "MeanDistance" || _typeHisto == "Area"){
		WrapperVoronoiDiagram * wv = dynamic_cast <WrapperVoronoiDiagram *>(m_data);
		if (wv){
			double nbs = wv->nbMolecules();
			unsigned int nbForUpdate = nbs / 100., cpt = 0;
			if (nbForUpdate == 0) nbForUpdate = 1;
			printf("Saving in progress: %.2f %%", (0. / nbs * 100.));

			for (unsigned int i = 0; i < wv->nbMolecules(); i++){
				//if (wv->isDataSelected(i))
				{
					if (cpt++ % nbForUpdate == 0) printf("\rSaving in progress: %.2f %%", ((double)cpt / nbs * 100.));
					double val = wv->getInfosData(_typeI, i);
					fs << val << std::endl;
				}
			}
		}
	}

	printf("\rSaving done.               \n");
	fs.close();
}