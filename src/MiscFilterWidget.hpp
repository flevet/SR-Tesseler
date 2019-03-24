/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      MiscFilterWidget.hpp
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

#ifndef MiscFilterWidget_h__
#define MiscFilterWidget_h__

#include <QDockWidget>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>

class Camera2D;

class MiscFilterWidget: public QDockWidget{
	Q_OBJECT
public:
	MiscFilterWidget( Camera2D *, QWidget* = 0, Qt::WFlags = 0 );
	~MiscFilterWidget();

	inline void setCurrentCamera( Camera2D * _cam ){m_currentCamera = _cam;}

protected slots:
	void setSizePoint( int );
	void setPointSmooth( bool );
	void setLineWidth( int );
	void setLineSmooth( bool );
	void changeBackgroundColor();

	void savePosition();
	void loadPosition();
	void snapViewer();
	void exportInVectorialFile();

protected:
	QCheckBox * m_smoothPointCB, * m_smoothLineCB;
	QLabel * m_sizePointLbl, * m_widthLineLbl, * m_strokeWidthLbl;
	QLineEdit * m_strokeWidthLEdit;
	QSpinBox * m_sizePointSpn, * m_widthLineSpn;

	QPushButton * m_colorBackBtn;

	Camera2D * m_currentCamera;
};

#endif // MiscFilterWidget_h__