/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      DetectionCleanerGLViewer.cpp
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
#include <math.h>
#include <QTextDocument>
#include <gl/GLU.h>

#include "DetectionCleanerGLViewer.hpp"

DetectionCleanerGLViewer::DetectionCleanerGLViewer( const QString & _name, QWidget * _parent /*= 0*/, const QGLWidget * _shareWidget /*= 0*/, Qt::WindowFlags _f /*= 0 */ ):QGLWidget( _parent, _shareWidget, _f ), m_name( _name )
{
	m_minX = m_minY = 0.;
	m_maxX = m_maxY = 1.;
	m_equation = NULL;
	setAutoFillBackground( true );
}

DetectionCleanerGLViewer::~DetectionCleanerGLViewer()
{
}

void DetectionCleanerGLViewer::setEquation( EquationFit * _eqn )
{
	m_equation = _eqn;
	double * counts = m_equation->getValues();
	int maxDarkT = m_equation->getNbTs();

	double diffX = ( double )maxDarkT / 10.;
	double diffY = ( counts[0] - counts[maxDarkT-1] ) / 10.;
	m_minX = -diffX;
	m_maxX = maxDarkT + diffX;
	m_minY = counts[maxDarkT-1] - diffY;
	m_maxY = counts[0] + diffY;
	m_axisX = -diffX / 2.;
	m_axisY = counts[maxDarkT-1] - ( diffY / 2. );
	updateGL();
}

void DetectionCleanerGLViewer::initializeGL()
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

void DetectionCleanerGLViewer::paintGL()
{
	double marginX = 0.2, marginY = 0.2;

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glViewport( 0, 0, width(), height() );
	gluOrtho2D( 0. - marginX, 1. + marginX, 0. - marginY, 1. + marginY  );

	glClearColor(.7f, .7f, .7f, 1.f);
	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	if( m_equation == NULL ){
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		return;
	}

	double * values = m_equation->getValues();
	double * ts = m_equation->getTs();
	int nbT = m_equation->getNbTs();
	double diffY = values[0] - values[nbT-1];

	//glPushMatrix();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glLoadIdentity();

	glEnable( GL_POINT_SMOOTH );
	glPointSize( 4.f );
	glLineWidth( 1.f );
	glColor3f( 0.f, 0.f, 0.f );

	double diffX = 1. / ( double )nbT;
	glBegin( GL_QUADS );
	for( int n = 0; n < nbT; n++ ){
		double diff = values[n] - values[nbT-1];
		double y = ( diff / diffY ) * 1.;
		glVertex2d( n * diffX, 0. );
		glVertex2d( n * diffX, y );
		glVertex2d( ( n + 1 ) * diffX, y );
		glVertex2d( ( n + 1 ) * diffX, 0. );
	}
	glEnd();
	glBegin( GL_LINES );
	glVertex2d( 0., 0. );
	glVertex2d( 0., 1. );
	glVertex2d( 0., 0. );
	glVertex2d( 1., 0. );
	glEnd();

	glColor3f( 1.f, 0.f, 0.f );
	glBegin( GL_LINE_STRIP );
	int nbFitValues = 100;
	double diffValuesX = ts[nbT-1] - ts[0];
	double stepX = diffValuesX / ( double )nbFitValues;
	for( int n = 0; n < nbFitValues; n++ ){
		double x = ts[0] + ( ( double )n * stepX );
		double y = m_equation->getFitValues( x );
		double px = ( diffX / 2. ) + ( ( ( x - ts[0] ) * diffX ) / ( 1. - diffX ) );
		double diff = y - values[nbT-1];
		double py = ( diff / diffY ) * 1.;
		glVertex2d( px, py );
	}
	glEnd();

	glColor3f( 0.f, 0.f, 0.f );
	double pixX = ( double )width() / 1.4; //1.4 is the size of the modelview in X, 1. + 2 margin of .2
	int sizeMarginXPix = 0.2 * pixX + 0.5;
	double pixY = ( double )height() / 1.4; //1.4 is the size of the modelview in X, 1. + 2 margin of .2
	int sizeMarginYPix = 0.2 * pixY + 0.5;
	QFont font( "Times", 8, QFont::Bold );
	QFontMetrics fm( font );

	double y1 = values[0] - diffY / 3., y2 = values[nbT-1] + diffY / 3.;
	QString ny0( QString::number( values[0], 'g', 3 ) );
	renderText( sizeMarginXPix - fm.width( ny0 ) - 5, sizeMarginYPix + ( fm.height() / 2 ), ny0, font );
	QString ny1( QString::number( y1, 'g', 3 ) );
	renderText( sizeMarginXPix - fm.width( ny1 ) - 5, sizeMarginYPix + pixY * 0.3 + ( fm.height() / 2 ), ny1, font );
	QString ny2( QString::number( y2, 'g', 3 ) );
	renderText( sizeMarginXPix - fm.width( ny2 ) - 5, height() - sizeMarginYPix - pixY * 0.3 - ( fm.height() / 2 ), ny2, font );
	QString ny3( QString::number( values[nbT-1], 'g', 3 ) );
	renderText( sizeMarginXPix - fm.width( ny3 ) - 5, height() - sizeMarginYPix, ny3, font );

	int xi1 = 0, xi2 = nbT / 4 + 0.5, xi3 = nbT / 2 + 0.5, xi4 = nbT * ( 3. / 4. ) + 0.5, xi5 = nbT - 1;
	QString nx1( QString::number( ts[xi1] ) ), nx2( QString::number( ts[xi2] ) ), nx3( QString::number( ts[xi3] ) ), nx4( QString::number( ts[xi4] ) ), nx5( QString::number( ts[xi5] ) );
	int halfSizeColumn = diffX / 2. * pixX + 0.5;
	renderText( sizeMarginXPix + xi1 * diffX * pixX + halfSizeColumn - ( fm.width( nx1 ) / 2 ), height() - sizeMarginYPix + fm.height() + 5, nx1, font );
	renderText( sizeMarginXPix + xi2 * diffX * pixX + halfSizeColumn - ( fm.width( nx2 ) / 2 ), height() - sizeMarginYPix + fm.height() + 5, nx2, font );
	renderText( sizeMarginXPix + xi3 * diffX * pixX + halfSizeColumn - ( fm.width( nx3 ) / 2 ), height() - sizeMarginYPix + fm.height() + 5, nx3, font );
	renderText( sizeMarginXPix + xi4 * diffX * pixX + halfSizeColumn - ( fm.width( nx4 ) / 2 ), height() - sizeMarginYPix + fm.height() + 5, nx4, font );
	renderText( sizeMarginXPix + xi5 * diffX * pixX + halfSizeColumn - ( fm.width( nx5 ) / 2 ), height() - sizeMarginYPix + fm.height() + 5, nx5, font );

	font.setPointSize( 10 );
	renderText( width() / 2 - fm.width( m_name ), 5 + fm.height(), m_name, font );

	QFont font2( "Times", 12, QFont::Bold );
	QFontMetrics fm2( font2 );
	int textEqW = fm2.width( m_equation->getEquation() );
	renderText( width() - textEqW - 25, 50, m_equation->getEquation(), font2 );

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	//drawEquation();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void DetectionCleanerGLViewer::drawEquation()
{
	QPainter painter(this);
	const int Margin = 11;
	const int Padding = 6;

	QTextDocument textDocument;
	textDocument.setDefaultStyleSheet("* { color: #FFEFEF }");
	textDocument.setHtml("<h2 align=\"center\">" + m_equation->getEquation() +"</h2>");
	textDocument.setTextWidth(textDocument.size().width());

	QRect rect(QPoint(0, 0), textDocument.size().toSize() + QSize(2 * Padding, 2 * Padding));
	painter.translate(width() - rect.width() - Margin, 3*Margin);
	painter.setPen(QColor(255, 239, 239));
	painter.setBrush(QColor(255, 0, 0, 31));
	painter.drawRect(rect);

	painter.translate(Padding, Padding);
	textDocument.drawContents( &painter );
}

QSize DetectionCleanerGLViewer::sizeHint() const
{
	return QSize( 400, 200 );
}

