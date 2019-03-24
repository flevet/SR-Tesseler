/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      ImageViewer.hpp
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

#ifndef ImageViewer_h__
#define ImageViewer_h__

#include <QScrollBar>
#include <QToolBar>
#include <QAction>
#include <QStatusBar>
#include <QGridLayout>
#include <QLabel>
#include <QMenu>
#include <QProgressBar>
#include <QKeyEvent>
#include "Camera2D.hpp"

class DetectionSet;

class ImageViewer: public QWidget{
	Q_OBJECT

public:
	ImageViewer();
	~ImageViewer();

protected:
	void createActions();
	void createConnections();
	QSize sizeHint() const;
	void closeEvent( QCloseEvent * );

protected slots:
	void openPalmTracerDataset();
	void openLocalizationDataset();
	void setInteractionCamera();
	void aboutDialog();

public slots:
	void adjustSizeViewer();
	void setStatusBarPosition(const QString &);
	void setStatusBarZoom(const QString &);
	void displayOrHideMenu();

protected:
	Camera2D * m_camera;

	QAction * m_openDirAct/*, *m_openPalmTracer2Act*/, *m_openLocFileAct;
	QAction * m_openAct, *zoomAct, *moveAct, *m_roisAct, *m_aboutAct, *m_gridAct;
	QToolBar * m_editToolBar;
	QWidget * m_statusBar;
	QLabel * m_labelStatusPosition, * m_labelStatusZoom;
	QGridLayout * m_centralLayout;
	QScrollBar * m_hbar, * m_vbar;

	QMenu *m_openSelectionMenu;

	bool m_initialized;
public:
	QProgressBar * m_progress;
};

#endif // ImageViewer_h__