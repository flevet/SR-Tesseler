/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      VoronoiWidget.hpp
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

#ifndef VoronoiWidget_h__
#define VoronoiWidget_h__

#include <QTabWidget>
#include <QRadioButton>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QLineEdit>
#include <QComboBox>
#include <QButtonGroup>
#include <QTableWidget>

#include "FilterObjectWidget.hpp"

class Camera2D;
class WrapperVoronoiDiagram;
class NeuronObject;
class QCustomPlot;

class VoronoiWidget: public QTabWidget{
	Q_OBJECT
public:
	VoronoiWidget( Camera2D *, QWidget* = 0 );
	~VoronoiWidget();

	void setHistogramData();
	void setWrapperVoronoi( WrapperVoronoiDiagram * );
	void setCurrentCamera( Camera2D * );

	void changeMinHistogram( const double );
	void changeColor(QPushButton *, const int);

	inline QPushButton * getButtonCreation() const {return m_buttonCreation;}
	inline QCheckBox * getDisplayLabelObjectsCheckBox() const {return m_cboxDisplayObjLabels;}
	inline void setEnableFordsetCleaner( const bool _val ){m_cboxdsetCleaner->setEnabled( true ); m_cboxdsetCleaner->setChecked( true );}
	inline const bool isdsetCleanerChosen() const {return m_cboxdsetCleaner->isChecked();}

protected:
	void updateObjectsList();

protected slots:
	void applyDensityFactor();
	void segmentVoronoi();
	void createClusters();
	void exportStatsClustersObjects();
	void exportStatsObjects();
	void exportObjectsToClipboard();
	void exportClustersToClipboard();
	void exportLocalizationsIDObjects();
	void exportLocalizationsIDClusters();
	void createVoronoi();
	void toggleDisplayShapeObjs( bool );
	void toggleDisplayOutlineObjs( bool );
	void toggleDisplayShapeClusts( bool );
	void toggleDisplayOutlineClusts( bool );
	void toggleDisplayObjectEllipses(bool);
	void changeColorSlot();
	void transferColorToLocs();

protected:
	Camera2D * m_currentCamera;

	//Construction
	QGroupBox * m_groupVoronoi;
	QCheckBox * m_cboxdsetCleaner, * m_cboxdset;
	QPushButton * m_buttonCreation;
	FilterObjectWidget * m_filterVoronoiWidget;
	
	//Object
	QGroupBox * m_groupSegmentation, * m_groupVoronoiObjects;
	QCheckBox * m_cboxObjectOnDiagram, *m_cboxObjectOnROIs, *m_cboxDeltaObjectDiagram, *m_cboxDeltaObjectROIs, *m_cboxDisplayObjLabels, *m_cboxMinAreaObjs, *m_cboxMinLocsObjs, *m_cboxCutDistObjs, *m_cboxPCAEllipse, *m_cboxBoundingEllipse, *m_cboxWatershed, *m_cboxMaxAreaObjs, *m_cboxMaxLocsObjs;
	QLineEdit * m_factorDensityObjectLEdit, *m_minAreaObjectsLEdit, *m_minLocsObjectsLEdit, *m_cutDistObjectsLEdit, *m_radiusWatershedLEdit, *m_nbLocsWatershedLEdit, *m_maxAreaObjectsLEdit, *m_maxLocsObjectsLEdit;
	QButtonGroup * m_buttonGroupObjectsOnWhat, * m_buttonGroupEllipse;
	QWidget * m_emptyForObjects;
	QTableWidget * m_tableObjs;
	QPushButton * m_colorObjShapeBtn, *m_colorObjOutlineBtn, *m_colorObjEllipseBtn, * m_transferColorObjBtn;

	//Clusters
	QGroupBox * m_groupVoronoiClusters;
	QCheckBox * m_cboxClustersOnObject, *m_cboxClustersOnROIs, *m_cboxDeltaClusters, *m_cboxDeltaClustersROIs, *m_cboxMinAreaClusters, *m_cboxMinLocsClusters, *m_cboxMaxAreaClusters, *m_cboxMaxLocsClusters;
	QLineEdit * m_factorDensityClustersLEdit, *m_minLocsClustersLEdit, *m_minAreaClustersLEdit, *m_maxLocsClustersLEdit, *m_maxAreaClustersLEdit;
	QButtonGroup * m_buttonGroupClustersOnWhat;
	QWidget * m_emptyForClusters, * m_clustersWidget;
	QTableWidget * m_tableClusters;
	QPushButton * m_colorClusterShapeBtn, *m_colorClusterOutlineBtn, *m_colorClusterEllipseBtn, *m_transferColorClusterBtn;
};

#endif // VoronoiWidget_h__