/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      DetectionCleanerWidget.hpp
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

#ifndef DetectionCleanerWidget_h__
#define DetectionCleanerWidget_h__

#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QButtonGroup>
#include <QPlainTextEdit>

#include "DetectionCleanerGLViewer.hpp"
#include "FilterObjectWidget.hpp"

class Camera2D;
class DetectionCleaner;

class DetectionCleanerWidget: public QTabWidget{
	Q_OBJECT

public:
	DetectionCleanerWidget( Camera2D *, QWidget* = 0 );
	~DetectionCleanerWidget();

	double getSizeFixedNeighborhood() const;
	double getPixelSize() const;
	double getBackgroundValue() const;
	double getInt2PhotonRatio() const;
	int getMaxDarkTime() const;

	inline QPushButton * getProcessButton(){return m_buttonProcess;}
	inline bool isFixedN() const {return m_rbuttonFixedN->isChecked();}
	inline bool isPhotonN() const {return m_rbuttonPhotonN->isChecked();}
	inline bool isPhotonBackGroundN() const {return m_rbuttonPhotonBackGN->isChecked();}
	inline bool isMaxDarkTimeDefined() const { return m_cboxFixedMaxDarkTime->isChecked(); }
	inline void setEnableExport(const bool _val) { m_buttonExport->setEnabled(_val); }

	void setCurrentCamera( Camera2D * );
	void setDetectionCleaner( DetectionCleaner * );

	unsigned char getOptions() const;

protected slots:
	void createDetectionCleaner();
	void changeButton( QAbstractButton * );
	void exportStats();

protected:
	QCheckBox * m_displayPolygons, * m_cboxFixedMaxDarkTime;
	QRadioButton * m_rbuttonFixedN, * m_rbuttonPhotonN, * m_rbuttonPhotonBackGN;
	QLabel * m_lblFixed, * m_lblPixel, * m_lblBack, * m_lblInt2Photon;
	QLineEdit * m_leditFixedNeigh, *m_leditPixelSize, *m_leditBackground, *m_lEditInt2Photon, * m_leditMaxDarkTime;
	QPushButton * m_buttonProcess, * m_buttonExport;
	QLabel * m_equationFitLbl;
	QButtonGroup * m_bgroup;

	Camera2D * m_currentCamera;
	DetectionCleanerGLViewer * m_dcBlinksViewer, * m_dcTOnsViewer, * m_dcToffsViewer;
	FilterObjectWidget * m_filterDetectionsWidget;

	QPlainTextEdit * m_statsTEdit;
};

#endif // DetectionCleanerWidget_h__