/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      RoiManagerWidget.hpp
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

#ifndef RoiManagerWidget_h__
#define RoiManagerWidget_h__

#include <QWidget>
#include <QButtonGroup>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QGroupBox>

#include "Camera2D.hpp"

class RoiManagerWidget: public QWidget{
	Q_OBJECT

public:
	RoiManagerWidget( Camera2D *, QWidget * = 0, Qt::WFlags = 0 );
	~RoiManagerWidget();

	inline const int getCurrentROIIndex() const {return m_roisCombo->currentIndex();}

public slots:
	void addRoiToRoiManager();

protected slots:
	void saveRois();
	void discardCurrentRoi();
	void discardAllRois();

protected:
	QGroupBox * m_groupRois;
	QComboBox * m_roisCombo;
	QPushButton * m_discardCurRoiBtn, * m_discardAllRoisBtn, * m_loadRoisBtn, * m_saveRoisBtn;
	QCheckBox * m_cboxDisplayLabelRoi;

	Camera2D * m_camera;
};

#endif // RoiManagerWidget_h__