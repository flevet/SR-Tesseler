/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      HistogramCamera.hpp
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

#ifndef HistogramCamera_h__
#define HistogramCamera_h__

#include <QGLWidget>
#include "Palette.hpp"

class ObjectInterface;
class Camera2D;

class HistogramCamera: public QGLWidget{
	Q_OBJECT
public:
	HistogramCamera( ObjectInterface *, Camera2D *, QWidget * = 0, const QGLWidget * = 0, Qt::WindowFlags = 0 );
	~HistogramCamera();

	void changeDataSelected( ObjectInterface *, Camera2D * );
	void regenerateDataSelected();

	double setMinHistogram( double );
	double setMaxHistogram( double );
	void saveDataHistogram(const QString &, const int);

public slots:
	void setLog( bool );
	void changeDataSelected( bool );
	void changeDataSelected( bool, Camera2D * );
	void changeTypeHisto( int );
	void setPolygonFilled( bool );
	void setOutlineDisplay( bool );

	void changeLut( const QString & );

protected:
	void initializeGL();
	void paintGL();
	void mousePressEvent( QMouseEvent * );
	void mouseMoveEvent ( QMouseEvent * );
	void mouseReleaseEvent( QMouseEvent * );

	void determineCoordInWorld( const int, const int, double &, double & );
	double determineCoordInWorldX( const int );
	double determineCoordInWorldY( const int );
	void setCurrentMin( ObjectInterface * );
	void setCurrentMax( ObjectInterface * );

protected:
	ObjectInterface * m_data;
	Camera2D * m_camera;
	double m_cursorX, m_cursorY, m_paletteY, m_minX, m_maxX, m_minY, m_maxY, m_zoomx1, m_zoomx2, m_zoomy1, m_zoomy2, m_currentMin, m_currentMax;
	bool m_insidePalette, m_buttonLeft, m_buttonRight, m_zoomEnabled;
	int m_indexHistograms;
};
#endif // HistogramCamera_h__