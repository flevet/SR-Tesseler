/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      DetectionCleanerGLViewer.hpp
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

#ifndef DetectionCleanerGLViewer_h__
#define DetectionCleanerGLViewer_h__

#include <QGLWidget>
#include <QPainter>

#include "DetectionCleaner.hpp"
#include "GeneralTools.hpp"

class DetectionCleanerGLViewer: public QGLWidget{
public:
	DetectionCleanerGLViewer( const QString &, QWidget * = 0, const QGLWidget * = 0, Qt::WindowFlags = 0 );
	~DetectionCleanerGLViewer();

	QSize sizeHint() const;

	void setEquation( EquationFit * );

protected:
	void initializeGL();
	void paintGL();
	void drawEquation();

protected:
	QString m_name;
	EquationFit * m_equation;

	double m_minX, m_maxX, m_minY, m_maxY, m_axisX, m_axisY;
};

#endif // DetectionCleanerGLViewer_h__