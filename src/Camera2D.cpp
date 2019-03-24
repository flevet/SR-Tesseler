/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      Camera2D.cpp
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
#include <QtGui>
#include <GL/glu.h>
#include <GL/gl.h>
#include <float.h>
#include <fstream>

#include "Camera2D.hpp"
#include "SuperResObject.hpp"
#include "MainFilterDialog.hpp"
#include "VoronoiWidget.hpp"
#include "DetectionSet.hpp"
#include "WrapperVoronoiDiagram.hpp"
#include "DetectionCleanerWidget.hpp"
#include "RoiManagerWidget.hpp"
#include "MiscQuantificationWidget.hpp"
#include "DBScan.hpp"
#include "gl2ps.h"

Camera2D::Camera2D(QScrollBar * _hbar, QScrollBar * _vbar) :m_pointSmooth(false), m_sizePoint(1), m_displayGrid(false), m_displayObjectLabels(true), m_lineSmooth(false), m_lineWidth(1), m_displayClusterLabels(true), m_backColor(0, 0, 0, 255), m_displayDBSCANLabels(true)
{
	m_hbar = _hbar;
	m_vbar = _vbar;

	m_superResObj = NULL;
	m_dcw = NULL;

	m_doubleClick = false;
	m_zoomFactor = 1.f;
	m_modeInteraction = Camera2D::ModeNone;
	m_button = false;
	m_xv = m_yv = 0.f;
	m_widthProjection = m_heightProjection = 1.f;
	m_originalImageWidth = m_originalImageHeight = 0;

	setMouseTracking(true);
	QObject::connect( m_hbar, SIGNAL( sliderReleased () ), this, SLOT( updateGL() ) );
	QObject::connect( m_vbar, SIGNAL( sliderReleased () ), this, SLOT( updateGL() ) );
	QObject::connect( m_hbar, SIGNAL( sliderMoved ( int ) ), this, SLOT( moveXScrollbarWithUpdateDisplay( int ) ) );
	QObject::connect( m_vbar, SIGNAL( sliderMoved ( int ) ), this, SLOT( moveYScrollbarWithUpdateDisplay( int ) ) );
}

Camera2D::~Camera2D()
{
}

void Camera2D::initializeGL()
{
	GLenum err = glGetError();

	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	glDisable( GL_CULL_FACE );
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void Camera2D::paintGL()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glViewport(0, 0, this->width(), this->height());
	gluOrtho2D(0.f, m_widthProjection, m_heightProjection, 0.f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor( ( float )m_backColor[0] / 255.f, ( float )m_backColor[1] / 255.f, ( float )m_backColor[2] / 255.f, ( float )m_backColor[3] / 255.f );	
	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glEnable(GL_COLOR_MATERIAL);
	glDisable( GL_CULL_FACE );

	glTranslatef( -m_xv, -m_yv, 0.f );

	glPointSize( m_sizePoint );
	gl2psPointSize(m_sizePoint);
	if (m_pointSmooth)
		glEnable( GL_POINT_SMOOTH );
	else
		glDisable( GL_POINT_SMOOTH );
	glLineWidth( m_lineWidth );
	gl2psLineWidth(m_lineWidth);
	if (m_lineSmooth)
		glEnable( GL_LINE_SMOOTH );
	else
		glDisable( GL_LINE_SMOOTH );

	if( m_superResObj != NULL ){
		m_superResObj->draw( this );
		if( m_displayObjectLabels )
			displayObjectLabels();
		if( m_displayClusterLabels )
			displayClusterLabels();
		if (m_displayDBSCANLabels)
			displayDBSCANLabels();
	}

	if( m_displayGrid )
		displayGrid();

	if( m_modeInteraction == Camera2D::ModeZoom ){
		glPushMatrix();
		glColor3f(1.f, 0.f, 0.f);
		glBegin(GL_LINE_STRIP);
		glVertex2f(m_interactionStart.x(), m_interactionStart.y());
		glVertex2f(m_interactionStop.x(), m_interactionStart.y());
		glVertex2f(m_interactionStop.x(), m_interactionStop.y());
		glVertex2f(m_interactionStart.x(), m_interactionStop.y());
		glVertex2f(m_interactionStart.x(), m_interactionStart.y());
		glEnd();
		glPopMatrix();
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void Camera2D::displayObjectLabels()
{
	const NeuronObjectList & objs = m_superResObj->getNeuronObjects();
	QFont font( "Times", 10, QFont::Bold );
	QFontMetrics fm( font );

	glColor3f( 1., 0.86, 0.02 );
	int cpt = 1;
	for( NeuronObjectList::const_iterator it = objs.begin(); it != objs.end(); it++, cpt++ ){
		NeuronObject * nobj = *it;
		const Vec2mf & barycenter = nobj->getObject()->getBarycenter();
		QString text( "o" + QString::number( cpt ) );
		int textW1 = fm.width( text );
		Vec2mf pos = getScreenCoordinates( barycenter.x() / m_originalImageWidth, barycenter.y() / m_originalImageHeight );
		renderText( pos.x(), pos.y(), text, font );
	}
}

void Camera2D::displayClusterLabels()
{
	const NeuronObjectList & objs = m_superResObj->getNeuronObjects();
	QFont font( "Times", 10, QFont::Bold );
	QFontMetrics fm( font );

	glColor3f( 1., 0., 0.86 );
	int cpt = 1;
	for( NeuronObjectList::const_iterator it = objs.begin(); it != objs.end(); it++ ){
		NeuronObject * nobj = *it;
		const VoronoiClusterList & listClusters = nobj->getClusters();
		for( VoronoiClusterList::const_iterator it = listClusters.begin(); it != listClusters.end(); it++ ){
			VoronoiCluster * cluster = *it;
			const Vec2mf & barycenter = cluster->getBarycenter();
			QString text( "c" + QString::number( cpt ) );
			int textW1 = fm.width( text );
			Vec2mf pos = getScreenCoordinates( barycenter.x() / m_originalImageWidth, barycenter.y() / m_originalImageHeight );
			renderText( pos.x(), pos.y(), text, font );
			cpt++;
		}
	}
}

void Camera2D::displayDBSCANLabels()
{
	QFont font("Times", 10, QFont::Bold);
	QFontMetrics fm(font);

	glColor3f(1., 0.86, 0.02);
	DBScan * dbscan = m_superResObj->getDBSCAN();
	Vec2mf * centroids = dbscan->getCentroids();
	if (centroids == NULL) return;
	unsigned int nbClusters = dbscan->getNbClusters();
	for (unsigned int n = 0; n < nbClusters; n++){
		QString text("o" + QString::number(n + 1));
		int textW1 = fm.width(text);
		Vec2mf pos = getScreenCoordinates(centroids[n].x() / m_originalImageWidth, centroids[n].y() / m_originalImageHeight);
		renderText(pos.x(), pos.y(), text, font);
	}
}

void Camera2D::setInteraction( const QString & mode )
{
	if(mode == tr("Zoom")){
		QIcon icon("./images/zoom.png");
		this->setCursor(icon.pixmap(25, 25));
		m_modeInteraction = Camera2D::ModeZoom;
	}
	else if(mode == tr("Move image")){
		this->setCursor(Qt::OpenHandCursor);
		m_modeInteraction = Camera2D::ModeMove;
	}
	else if( mode == tr( "Define roi" ) ){
		this->setCursor( Qt::ArrowCursor );
		m_modeInteraction = Camera2D::RoiDefinition;
	}
	else{
		this->setCursor(Qt::ArrowCursor);
	}
}

void Camera2D::mousePressEvent( QMouseEvent * e )
{
	m_interactionStart = getTrueCoordinates(e->pos().x(), e->pos().y());
	determineQStringForStatusBar( m_interactionStart.x(), m_interactionStart.y() );
	if(m_modeInteraction == Camera2D::ModeMove){
		this->setCursor(Qt::ClosedHandCursor);
		m_interactionStart.set( e->pos().x(), e->pos().y() );
	}
	m_button = true;
}

void Camera2D::mouseMoveEvent( QMouseEvent * e )
{
	if(!m_button){
		Vec2mf tmp = getTrueCoordinates(e->pos().x(), e->pos().y());
		determineQStringForStatusBar( tmp.x(), tmp.y() );
		return;
	}
	if(m_modeInteraction == Camera2D::ModeMove){
		m_interactionStop.set( e->pos().x(), e->pos().y() );
		moveImage();
		m_interactionStart = m_interactionStop;
		Vec2mf tmp = getTrueCoordinates( m_interactionStart.x(), m_interactionStart.y() );
		determineQStringForStatusBar( tmp.x(), tmp.y() );
	}
	else{
		m_interactionStop = getTrueCoordinates(e->pos().x(), e->pos().y());
		determineQStringForStatusBar(m_interactionStart.x(), m_interactionStart.y(), m_interactionStop.x(), m_interactionStop.y());
	}
	updateGL();
}

void Camera2D::mouseReleaseEvent( QMouseEvent * e )
{
	m_interactionStop = getTrueCoordinates( e->pos().x(), e->pos().y() );

	if( e->modifiers() == Qt::ControlModifier ){
	}
	else if( e->modifiers() == Qt::ShiftModifier ){
	}
	else if( m_modeInteraction == Camera2D::ModeZoom ){
		float xmin = ( m_interactionStart.x() < m_interactionStop.x() ) ? m_interactionStart.x() : m_interactionStop.x();
		float xmax = ( m_interactionStart.x() > m_interactionStop.x() ) ? m_interactionStart.x() : m_interactionStop.x();
		float ymin = ( m_interactionStart.y() < m_interactionStop.y() ) ? m_interactionStart.y() : m_interactionStop.y();
		float ymax = ( m_interactionStart.y() > m_interactionStop.y() ) ? m_interactionStart.y() : m_interactionStop.y();
		m_interactionStart.set( xmin, ymin );
		m_interactionStop.set( xmax, ymax );

		float minZoom = ( this->width() > this->height())?(1.f / ( float )this->height()) : (1.f / ( float )this->width() );
		minZoom /= 10.f;
		float dist = pow( ( float )( m_interactionStop.x()-m_interactionStart.x() ), 2 ) + pow( ( float )( m_interactionStop.y()-m_interactionStart.y() ), 2 );
		if( dist > minZoom )
			zoomIn();
		else if( e->button() == Qt::LeftButton ){
			Vec2mf currentPoint = getTrueCoordinates( e->pos().x(), e->pos().y() );;
			float zoomMult = 2.f;
			zoom( m_zoomFactor * zoomMult, currentPoint.x(), currentPoint.y() );
		}
		else if( e->button() == Qt::RightButton ){
			Vec2mf currentPoint = getTrueCoordinates( e->pos().x(), e->pos().y() );;
			zoom( m_zoomFactor * .5f, currentPoint.x(), currentPoint.y() );
		}

	}
	else if( m_modeInteraction == Camera2D::ModeMove ){
		this->setCursor( Qt::OpenHandCursor );
	}
	else if( m_modeInteraction == Camera2D::RoiDefinition ){
		if( !m_doubleClick ) {
			m_superResObj->addPointToRoi( m_interactionStop.x() * m_originalImageWidth, m_interactionStop.y() * m_originalImageHeight );
			updateGL();
		}
	}
	m_doubleClick = false;
	determineQStringForStatusBar( m_interactionStart.x(), m_interactionStart.y() );
	m_interactionStart = m_interactionStop = Vec2mf( -1.f, -1.f );
	updateGL();
	m_button = false;
}

void Camera2D::mouseDoubleClickEvent( QMouseEvent * _event )
{
	Vec2mf tmp = getTrueCoordinates( _event->pos().x(), _event->pos().y() );
	if( m_modeInteraction == Camera2D::RoiDefinition ){
		if( m_superResObj->addRoiToList() )
			m_RoiManagerW->addRoiToRoiManager();
	}
	m_doubleClick = true;
}

void Camera2D::wheelEvent( QWheelEvent * _event )
{
	if( _event->delta() > 0 ){
		Vec2mf currentPoint = getTrueCoordinates( _event->pos().x(), _event->pos().y() );;
		float zoomMult = 2.f;
		zoom( m_zoomFactor * zoomMult, currentPoint.x(), currentPoint.y() );
	}
	else{
		Vec2mf currentPoint = getTrueCoordinates( _event->pos().x(), _event->pos().y() );;
		zoom( m_zoomFactor * .5f, currentPoint.x(), currentPoint.y() );
	}
	updateGL();
}

Vec2mf Camera2D::getTrueCoordinates( const int x, const int y )
{
	float origX = (float)x / ( (float)m_originalImageWidth * m_zoomFactor );
	origX += m_xv;
	float origY = (float)y / ( (float)m_originalImageHeight * m_zoomFactor );
	origY += m_yv;
	return Vec2mf(origX, origY);
}


Vec2mf Camera2D::getScreenCoordinates( const double _x, const double _y )
{
	float x = _x - m_xv;
	x *= ( float )m_originalImageWidth * m_zoomFactor;
	float y = _y - m_yv;
	y *= ( float )m_originalImageHeight * m_zoomFactor;
	return Vec2mf( x, y );
}

void Camera2D::determineQStringForStatusBar( const float x1, const float y1, const float x2 /*= -1*/, const float y2 /*= -1*/ )
{
	QString stringX = QString::number( /*( int )*/( x1 * m_originalImageWidth ) );
	QString stringY = QString::number( /*( int )*/( y1 * m_originalImageHeight ) );

	QString mess("[x=" + stringX + ", y=" + stringY + "]");

	if( x2 != -1.f && y2 != -1.f ){
		stringX = QString::number( /*( int )*/( x2 * m_originalImageWidth ) );
		stringY = QString::number( /*( int )*/( y2 * m_originalImageHeight ) );
		mess += (" - > [x=" + stringX + ", y=" + stringY  + "]");
	}
	emit(setStatusBarPosition(mess));
}

void Camera2D::zoomIn()
{
	float scaleW = fabs( m_interactionStop.x() - m_interactionStart.x() ) ;// 2.f;
	float scaleH = fabs( m_interactionStop.y() - m_interactionStart.y() ) ;// 2.f;
	float scale = ( scaleW < scaleH ) ? scaleW : scaleH;
	float centerX = ( m_interactionStart.x() + m_interactionStop.x() ) / 2.f;
	float centerY = ( m_interactionStart.y() + m_interactionStop.y() ) / 2.f;

	if(scaleW < scaleH){
		float rapport = m_widthProjection / m_heightProjection;
		float rapportScale = scaleH / m_heightProjection;
		m_yv = centerY - scaleH / 2.f;
		m_heightProjection = scaleH;
		m_widthProjection = m_heightProjection * rapport;
		m_xv = centerX - m_widthProjection / 2.f;
		m_zoomFactor /= rapportScale;
	}
	else{
		float rapport = m_heightProjection / m_widthProjection;
		float rapportScale = scaleW / m_widthProjection;
		m_xv = centerX - scaleW / 2.f;
		m_widthProjection = scaleW;
		m_heightProjection = m_widthProjection * rapport;
		m_yv = centerY - m_heightProjection / 2.f;
		m_zoomFactor /= rapportScale;
	}

	zoom( m_zoomFactor, centerX, centerY );
	replaceImageInScrollArea();

	m_interactionStart = m_interactionStop = Vec2mf(-1.f, -1.f);
	QString mess( "Zoom = " + QString::number( m_zoomFactor * 100.f ) + "%" );
	
	emit( setStatusBarZoom( mess ) );
}

void Camera2D::replaceImageInScrollArea()
{
	if( m_hbar->isVisible() ){
		m_hbar->setMinimum( 0 );
		m_hbar->setMaximum( this->getZoomedWidth() - this->width() );
		m_hbar->setValue( m_xv * this->getZoomedWidth() );
	}
	if( m_vbar->isVisible() ){
		m_vbar->setMinimum( 0 );
		m_vbar->setMaximum( this->getZoomedHeight() - this->height() );
		m_vbar->setValue( m_yv * this->getZoomedHeight() );
	}
}

void Camera2D::moveImage( const int scrollX, const int scrollY )
{
	m_xv = scrollX / ( float )this->getZoomedWidth();
	m_yv = scrollY / ( float )this->getZoomedHeight();
	updateGL();
}

void Camera2D::zoom( const float val, const float x, const float y )
{
	m_zoomFactor = val;
	{
		m_widthProjection = ( float )this->width() / ( float )this->getZoomedWidth();
		m_xv = x - m_widthProjection / 2.f;
		m_xv = ( m_xv < 0.f ) ? 0.f : m_xv;
		float maxTranslation = 1.f - m_widthProjection;
		m_xv = ( m_xv < maxTranslation ) ? m_xv : maxTranslation;
		if( m_widthProjection < 1.f ){
			float transTotalX = 1.f - m_widthProjection;
			float oneTick = transTotalX / ( float )m_hbar->maximum();
			int pos = m_xv / oneTick;
			m_hbar->setValue( pos );
		}
		else{
			m_hbar->hide();
			m_xv = 0.f;
		}
	}
	{
		m_heightProjection = ( float )this->height() / ( float )this->getZoomedHeight();
		m_yv = y - m_heightProjection / 2.f;
		m_yv = ( m_yv < 0.f ) ? 0.f : m_yv;
		float maxTranslation = 1.f - m_heightProjection;
		m_yv = ( m_yv < maxTranslation ) ? m_yv : maxTranslation;
		if( m_heightProjection < 1.f ){
			float transTotalY = 1.f - m_heightProjection;
			float oneTick = transTotalY / ( float )m_vbar->maximum();
			int pos = m_yv / oneTick;
			m_vbar->setValue( pos );
		}
		else{
			m_vbar->hide();
			m_yv = 0.f;
		}
	}
	if( m_widthProjection < 1.f )
		m_hbar->show();
	if( m_heightProjection < 1.f )
		m_vbar->show();
	QString mess( "Zoom = " + QString::number( m_zoomFactor * 100.f ) + "%" );
	emit( setStatusBarZoom( mess ) );
}

void Camera2D::moveImage()
{
	float diffW = m_interactionStop.x() - m_interactionStart.x();
	float diffH = m_interactionStop.y() - m_interactionStart.y();

	float sizePixX = 1.f / m_originalImageWidth;
	sizePixX /= m_zoomFactor;
	float sizePixY = 1.f / m_originalImageHeight;
	sizePixY /= m_zoomFactor;
	diffW *= sizePixX;
	diffH *= sizePixY;

	float newPosX = m_xv - diffW;
	float newPosY = m_yv - diffH;

	newPosX = ( newPosX < 0.f ) ? 0.f : newPosX;
	float maxTranslation = 1.f - m_widthProjection;
	newPosX = ( newPosX < maxTranslation ) ? newPosX : maxTranslation;

	newPosY = ( newPosY < 0.f ) ? 0.f : newPosY;
	maxTranslation = 1.f - m_heightProjection;
	newPosY = ( newPosY < maxTranslation ) ? newPosY : maxTranslation;

	int finalX = ( newPosX * ( float )this->getZoomedWidth() );
	int finalY = ( newPosY * ( float )this->getZoomedHeight() );

	moveImage( finalX, finalY );
	m_hbar->setValue( finalX );
	m_vbar->setValue( finalY );
}

void Camera2D::resizeEvent( QResizeEvent * _event )
{
	QSize oldSize = _event->oldSize(), size = _event->size();
	if( oldSize.width() == -1 && oldSize.height() == -1 )
		oldSize = size;
	float pixX = m_widthProjection / ( float )oldSize.width(), pixY = m_heightProjection / ( float )oldSize.height();
	float diffX = size.width() - oldSize.width(), diffY = size.height() - oldSize.height();
	m_widthProjection += diffX * pixX;
	m_heightProjection += diffY * pixY;
	updateGL();
}


void Camera2D::moveXScrollbarWithUpdateDisplay( int _val )
{
	moveXScrollbar( _val );
	updateGL();
}

void Camera2D::moveXScrollbar( int _val )
{
	float transTotalX = 1.f - m_widthProjection;
	float oneTick = transTotalX / ( float )m_hbar->maximum();
	m_xv = oneTick * ( float )_val;
}

void Camera2D::moveYScrollbarWithUpdateDisplay( int _val )
{
	moveYScrollbar( _val );
	updateGL();
}

void Camera2D::moveYScrollbar( int _val )
{
	float transTotalY = 1.f - m_heightProjection;
	float oneTick = transTotalY / ( float )m_vbar->maximum();
	m_yv = oneTick * ( float )_val;
}

void Camera2D::setDimension( const double _w, const double _h )
{
	m_originalImageWidth = _w;
	m_originalImageHeight = _h;
}

void Camera2D::setSuperResObject( SuperResObject * _obj )
{
	m_superResObj = _obj;
	m_tabWidget = new QTabWidget;
	m_tabWidget->setWindowIcon( QIcon("./images/voronIcon1.PNG") );
	MainFilterDialog * mainF = new MainFilterDialog( this );
	m_tabWidget->addTab( mainF, QObject::tr( "Filters" ) );
	m_RoiManagerW = new RoiManagerWidget( this );
	m_tabWidget->addTab( m_RoiManagerW, QObject::tr( "ROI Manager" ) );
	m_dcw = new DetectionCleanerWidget( this );
	m_tabWidget->addTab( m_dcw, QObject::tr( "Detection cleaner" ) );
	QObject::connect( m_dcw->getProcessButton(), SIGNAL( pressed() ), this, SLOT( cleanDetections() ) );
	m_voroWidget = new VoronoiWidget( this );
	m_tabWidget->addTab( m_voroWidget, QObject::tr( "Voronoi diagram" ) );
	m_miscQW = new MiscQuantificationWidget(this);
	m_tabWidget->addTab(m_miscQW, QObject::tr("Misc quantification"));
	m_tabWidget->resize(800, 950);
	m_tabWidget->setWindowTitle( tr( "SR-Tesseler: Controls" ) );
	m_tabWidget->show();
}

void Camera2D::createVoronoiDiagram( const bool _cleanerChosen )
{
	DetectionPoint * dset = NULL;
	int nbPoints = 0;

	if( _cleanerChosen ){
		dset = this->getDetectionSetCleaned()->getPoints();
		nbPoints = this->getDetectionSetCleaned()->getNbPoints();
	}
	else{
		dset = this->getDetectionSet()->getPoints();
		nbPoints = this->getDetectionSet()->getNbPoints();
	}

	if( dset == NULL ) return;

	double w = m_superResObj->getWidth(), h = m_superResObj->getHeight();
	WrapperVoronoiDiagram * wrapper = new WrapperVoronoiDiagram( dset, nbPoints, w, h );
	m_superResObj->setVoronoiDiagram( wrapper );
}

void Camera2D::cleanDetections()
{
	DetectionSet * dset = this->getDetectionSet();
	if (!dset->isCleanable()){
		QMessageBox msgBox;
		msgBox.setText("Dataset is not cleanable because the number of frames is less than 3.");
		msgBox.exec();
		return;
	}

	DetectionCleaner * dcleaner = new DetectionCleaner(dset, m_dcw->getSizeFixedNeighborhood(), m_dcw->getPixelSize(), m_dcw->getBackgroundValue(), m_dcw->getInt2PhotonRatio(), m_dcw->getMaxDarkTime(), m_dcw->getOptions(), m_superResObj->getDir());
	m_superResObj->setDetectionCleaner( dcleaner );
	m_dcw->setDetectionCleaner( this->getDetectionCleaner() );
	m_voroWidget->setEnableFordsetCleaner( true );
	updateGL();
}

void Camera2D::displayGrid() const
{
	glColor3f(1.f, 1.f, 1.f);
	float stepx = 1. / ( float )m_originalImageWidth;
	glBegin(GL_LINES);
	for( float x = 0.f; x <= 1.f; x+=stepx ){
		glVertex2f( x, 0.f );
		glVertex2f( x, 1.f );
	}
	float stepy = 1. / ( float )m_originalImageHeight;
	for( float y = 0.f; y <= 1.f; y+=stepy ){
		glVertex2f( 0.f, y );
		glVertex2f( 1.f, y );
	}
	glEnd();
}

void Camera2D::toggleGridDisplay()
{
	m_displayGrid = !m_displayGrid;
	updateGL();
}

void Camera2D::toggleDisplayObjectLabels( bool _val )
{
	m_displayObjectLabels = _val;
	updateGL();
}

void Camera2D::closeAll()
{
	m_tabWidget->close();
	delete m_superResObj;
}

void Camera2D::toggleDisplayClusterLabels( bool _val )
{
	m_displayClusterLabels = _val;
	updateGL();
}

void Camera2D::toggleDisplayLabelRoi( bool _val )
{
	m_superResObj->toggleDisplayLabelRoi( _val );
	updateGL();
}

void Camera2D::toggleDisplayDBSCANClusterLabels(bool _val)
{
	m_displayDBSCANLabels = _val;
	updateGL();
}
void Camera2D::changeBackgroundColor()
{
	QColor color = QColorDialog::getColor( QColor( ( int )m_backColor[0], ( int )m_backColor[1], ( int )m_backColor[2] ) );
	if( color.isValid() )
		m_backColor.set( color.red(), color.green(), color.blue(), 255 );
}

void Camera2D::loadRois()
{
	SuperResObject * sobj = getSuperResObject();
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Rois"), sobj->getDir().c_str(), tr("Rois Files (*.txt)"), 0, QFileDialog::DontUseNativeDialog);

	std::ifstream fs( filename.toAscii().data() );
	int nbRois, nbPointRoi;
	double x, y;

	std::string s;
	std::getline( fs, s );
	std::istringstream is( s );

	is >> nbRois;
	for( int n = 0; n < nbRois; n++ ){
		std::getline( fs, s );
		std::istringstream is2( s );
		Roi roi;
		is2 >> nbPointRoi;
		for( int n2 = 0; n2 < nbPointRoi; n2++ ){
			std::getline( fs, s );
			std::istringstream is3( s );
			is3 >> x >> y;
			roi.push_back( Vec2mf( x, y ) );
		}
		sobj->addRoiToList( roi );
		m_RoiManagerW->addRoiToRoiManager();
	}
	fs.close();
	updateGL();
}

void Camera2D::setPositionZoomParameters(double _values[5])
{
	m_widthProjection = _values[0];
	m_heightProjection = _values[1];
	m_xv = _values[2];
	m_yv = _values[3];
	m_zoomFactor = _values[4];

	repaint();
}

void Camera2D::getPositionZoomParameters(double _values[5])
{
	_values[0] = m_widthProjection;
	_values[1] = m_heightProjection;
	_values[2] = m_xv;
	_values[3] = m_yv;
	_values[4] = m_zoomFactor;
}

void Camera2D::snap(const QString & _filename)
{
	repaint();
	QImage image = this->grabFrameBuffer();
	QImageWriter writer(_filename);
	if (!writer.write(image))
		qDebug() << writer.errorString();
	else
		std::cout << "Snapshot " << _filename.toAscii().data() << " was saved" << std::endl;
	/*bool isSaved = image.save(_filename, 0, 100);
	if (!isSaved) std::cout << "Failed to save snapshot " << _filename.toAscii().data() << std::endl;
	else std::cout << "Snapshot " << _filename.toAscii().data() << " was saved" << std::endl;*/
}


void Camera2D::exportInVectorialFile(const QString & _filename)
{
	repaint();
	
	unsigned int nbFormat = 1;
	int formats[1] = { GL2PS_SVG }, opt;
	char ext[32];
	opt = GL2PS_DRAW_BACKGROUND;

	FILE *fp;
	char file[256];
	int state = GL2PS_OVERFLOW, buffsize = 0;
	GLint viewport[4];

	strcpy(file, _filename.toAscii().data());

	viewport[0] = 0;
	viewport[1] = 0;
	viewport[2] = this->width();
	viewport[3] = this->height();

	fp = fopen(file, "wb");

	if (!fp){
		printf("Unable to open file %s for writing\n", file);
		exit(1);
	}

	printf("Saving image to file %s... ", file);
	fflush(stdout);

	while (state == GL2PS_OVERFLOW){
		buffsize += 1024 * 1024;
		gl2psBeginPage(file, _filename.toAscii().data(), viewport, GL2PS_SVG, GL2PS_SIMPLE_SORT, opt,
			GL_RGBA, 0, NULL, 0, 0, 0, buffsize, fp, file);
		paintGL();
		state = gl2psEndPage();
	}

	fclose(fp);

	printf("Done!\n");
	fflush(stdout);

	/*printf("GL2PS %d.%d.%d%s done with all images\n", GL2PS_MAJOR_VERSION,
		GL2PS_MINOR_VERSION, GL2PS_PATCH_VERSION, GL2PS_EXTRA_VERSION);*/
}
