/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      MiscQuantificationWidget.hpp
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

#ifndef MiscQuantificationWidget_h__
#define MiscQuantificationWidget_h__

#include <QTabWidget>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QGroupBox>
#include <QTableWidget>

#include "qcustomplot\QCPHistogram.h"
#include "Vec4.hpp"

class Camera2D;
class DBScan;
class KRipley;

class MiscQuantificationWidget : public QTabWidget{

	Q_OBJECT

public:
	MiscQuantificationWidget(Camera2D *, QWidget* = 0);
	~MiscQuantificationWidget();

	inline void setCurrentCamera(Camera2D * _cam){ m_currentCamera = _cam; }

protected:
	void setKripleyCurveDisplay();

protected slots:
	void computeDBSCAN();
	void exportDBSCANResults();
	void computeKRipley();
	void toggleRipleyFunctionDisplay(bool);
	void changeBackgroundColor();
	void changeObjectColor();
	void exportKRipleyResults();

protected:
	/************************************************************************/
	/* For DBScan                                                           */
	/************************************************************************/
	QGroupBox * m_groupDBScan;
	QLabel * m_distanceDBScanLbl;
	QLineEdit * m_leditDistanceDBScan, *m_leditMinDDBScan, *m_leditMinPtsPerCluster;
	QPushButton * m_buttonDBScan, *m_buttonExportDBSCANRes, *m_colorBackBtn, *m_colorObjsBtn;
	QCheckBox * m_cboxOneColorDBSCAN, *m_cboxColorPerObjDBSCAN, *m_cboxDisplayDBSCANLabels, *m_cboxPCAEllipse, *m_cboxBoundingEllipse;
	QCPHistogram * m_customPlotDBSCAN;
	QTableWidget * m_tableObjs;
	QButtonGroup * m_buttonGroupEllipse;
	Color4D m_colorBack, m_colorObj;
	//DBScan * m_dbscan;

	/************************************************************************/
	/* For KRipley                                                          */
	/************************************************************************/
	QGroupBox * m_groupKRipley;
	QLabel * m_minKRipleyLbl, *m_maxKRipleyLbl, *m_stepKRipleyLbl, * m_resKRipleyLbl;
	QLineEdit * m_minKRipleyEdit, *m_maxKRipleyEdit, *m_stepKRipleyEdit;
	QCheckBox * m_cboxLsDisplayKRipley, * m_cboxRipleyOnROIs;
	QPushButton * m_buttonKRipley, *m_buttonExportKRipleyRes;
	QCustomPlot * m_customPlotKRipley;
	//KRipley * m_kripley;
	bool m_lsSelected;

	Camera2D * m_currentCamera;
};

#endif // MiscQuantificationWidget_h__