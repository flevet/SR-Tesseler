/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      VoronoiWidget.cpp
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

#include <QStandardItemModel>
#include <QModelIndex>
#include <QTreeView>
#include <QHeaderView>
#include <QFileDialog>
#include <QTime>
#include <fstream>

#include "VoronoiWidget.hpp"
#include "Camera2D.hpp"
#include "WrapperVoronoiDiagram.hpp"
#include "DetectionSet.hpp"
#include "ImageViewer.hpp"
#include "qcustomplot\qcustomplot.h"

#define WATERSHED_DEFINED 0

VoronoiWidget::VoronoiWidget( Camera2D * _camera, QWidget* _parent /*= 0*/ ):QTabWidget( _parent )
{
	m_currentCamera = _camera;

	QWidget * interactionWidget = new QWidget;
	QWidget * voroObjectsWidget = new QWidget;
	m_clustersWidget = new QWidget;

	QGroupBox * groupConstruction = new QGroupBox( QObject::tr( "Construction" ) );
	groupConstruction->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxdsetCleaner = new QCheckBox;
	m_cboxdsetCleaner->setText( "On detection cleaner" );
	m_cboxdsetCleaner->setEnabled( false );
	m_cboxdset = new QCheckBox;
	m_cboxdset->setText( "On all detections" );
	m_cboxdset->setChecked( true );
	QButtonGroup * groupBox = new QButtonGroup;
	groupBox->addButton( m_cboxdsetCleaner );
	groupBox->addButton( m_cboxdset );
	m_buttonCreation = new QPushButton( "Create polygons" );
	m_buttonCreation->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	QGridLayout * layoutConstruction= new QGridLayout;
	int columnCount = 0;
	layoutConstruction->addWidget( m_cboxdsetCleaner, 0, columnCount, 1, 1 );
	layoutConstruction->addWidget( m_cboxdset, 1, columnCount++, 1, 1 );
	layoutConstruction->addWidget( m_buttonCreation, 0, columnCount++, 2, 1 );
	groupConstruction->setLayout( layoutConstruction );

	m_groupSegmentation = new QGroupBox( QObject::tr( "Object segmentation" ) );
	m_groupSegmentation->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	QLabel * factorDensityLbl = new QLabel( "Density factor: " );
	factorDensityLbl->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_factorDensityObjectLEdit = new QLineEdit( "2." );
	m_factorDensityObjectLEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	QPushButton * applyDensityFacor = new QPushButton( "Set density factor" );
	applyDensityFacor->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	QPushButton * applySegmentationBtn = new QPushButton( "Create objects" );
	applySegmentationBtn->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	m_cboxMinAreaObjs = new QCheckBox( "Min area: " );
	m_cboxMinAreaObjs->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxMinAreaObjs->setChecked( true );
	m_minAreaObjectsLEdit = new QLineEdit( "2" );
	m_minAreaObjectsLEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxMinLocsObjs = new QCheckBox( "Min # locs: " );
	m_cboxMinLocsObjs->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxMinLocsObjs->setChecked( true );
	m_minLocsObjectsLEdit = new QLineEdit( "5" );
	m_minLocsObjectsLEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxCutDistObjs = new QCheckBox( "Cut distance: " );
	m_cboxCutDistObjs->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxCutDistObjs->setChecked( false );
	m_cutDistObjectsLEdit = new QLineEdit( "0.3" );
	m_cutDistObjectsLEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	QGroupBox * groupComputationObj = new QGroupBox( QObject::tr( "" ) );
	groupComputationObj->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	QString delta( 0x03B4 );
	QLabel * choiceForSigmaObjectLbl = new QLabel( "Computation of " + delta + ":" );
	choiceForSigmaObjectLbl->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxDeltaObjectDiagram = new QCheckBox( "On diagram" );
	m_cboxDeltaObjectDiagram->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxDeltaObjectROIs = new QCheckBox( "On ROIs" );
	m_cboxDeltaObjectROIs->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	QButtonGroup * buttonGroupDeltaObjects = new QButtonGroup;
	buttonGroupDeltaObjects->addButton( m_cboxDeltaObjectDiagram );
	buttonGroupDeltaObjects->addButton( m_cboxDeltaObjectROIs );
	m_cboxDeltaObjectDiagram->setChecked( true );
	QHBoxLayout * layoutComputationObj = new QHBoxLayout;
	layoutComputationObj->addWidget( choiceForSigmaObjectLbl );
	layoutComputationObj->addWidget( m_cboxDeltaObjectDiagram );
	layoutComputationObj->addWidget( m_cboxDeltaObjectROIs );
	groupComputationObj->setLayout( layoutComputationObj );
	QGroupBox * groupOnWhatObj = new QGroupBox( QObject::tr( "" ) );
	groupOnWhatObj->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	QLabel * objectsOnWhatLbl = new QLabel( "Object identification:" );
	objectsOnWhatLbl->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxObjectOnDiagram = new QCheckBox( "On diagram" );
	m_cboxObjectOnDiagram->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxObjectOnROIs = new QCheckBox( "On ROIs" );
	m_cboxObjectOnROIs->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_buttonGroupObjectsOnWhat = new QButtonGroup;
	m_buttonGroupObjectsOnWhat->addButton( m_cboxObjectOnDiagram );
	m_buttonGroupObjectsOnWhat->addButton( m_cboxObjectOnROIs );
	m_buttonGroupObjectsOnWhat->setId( m_cboxObjectOnDiagram, 0 );
	m_buttonGroupObjectsOnWhat->setId( m_cboxObjectOnROIs, 2 );
	m_cboxObjectOnDiagram->setChecked( true );
	m_cboxMaxAreaObjs = new QCheckBox("Max area: ");
	m_cboxMaxAreaObjs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_cboxMaxAreaObjs->setChecked(false);
	m_maxAreaObjectsLEdit = new QLineEdit("100000");
	m_maxAreaObjectsLEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_cboxMaxLocsObjs = new QCheckBox("Max # locs: ");
	m_cboxMaxLocsObjs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_cboxMaxLocsObjs->setChecked(false);
	m_maxLocsObjectsLEdit = new QLineEdit("1000000");
	m_maxLocsObjectsLEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_cboxPCAEllipse = new QCheckBox("PCA ellipse");
	m_cboxPCAEllipse->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_cboxBoundingEllipse = new QCheckBox("Bounding ellipse");
	m_cboxBoundingEllipse->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_buttonGroupEllipse = new QButtonGroup;
	m_buttonGroupEllipse->addButton(m_cboxPCAEllipse);
	m_buttonGroupEllipse->addButton(m_cboxBoundingEllipse);
	m_cboxPCAEllipse->setChecked(true);
	QLabel * watershedRadiusLbl = NULL, *watershedLbl = NULL;
	if (WATERSHED_DEFINED){
		watershedRadiusLbl = new QLabel("Radius elm std: ");
		watershedRadiusLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		m_radiusWatershedLEdit = new QLineEdit("0.5");
		m_radiusWatershedLEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		watershedLbl = new QLabel("# locs elm std: ");
		watershedLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		m_nbLocsWatershedLEdit = new QLineEdit("60");
		m_nbLocsWatershedLEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		m_cboxWatershed = new QCheckBox("Watershed");
		m_cboxWatershed->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		m_cboxWatershed->setChecked(false);
	}
	else{
		m_radiusWatershedLEdit = m_nbLocsWatershedLEdit = NULL;
		m_cboxWatershed = NULL;
	}

	QHBoxLayout * layoutOnWhatObj = new QHBoxLayout;
	layoutOnWhatObj->addWidget( objectsOnWhatLbl );
	layoutOnWhatObj->addWidget( m_cboxObjectOnDiagram );
	layoutOnWhatObj->addWidget( m_cboxObjectOnROIs );
	groupOnWhatObj->setLayout( layoutOnWhatObj );
	QGridLayout * layoutSegmentation = new QGridLayout;
	layoutSegmentation->addWidget( factorDensityLbl, 0, 0, 1, 1 );
	layoutSegmentation->addWidget( m_factorDensityObjectLEdit, 0, 1, 1, 1 );
	layoutSegmentation->addWidget( m_cboxCutDistObjs, 0, 2, 1, 1 );
	layoutSegmentation->addWidget( m_cutDistObjectsLEdit, 0, 3, 1, 1 );
	layoutSegmentation->addWidget( applyDensityFacor, 0, 4, 1, 1 );
	layoutSegmentation->addWidget( choiceForSigmaObjectLbl, 0, 5, 1, 1 );
	layoutSegmentation->addWidget( m_cboxDeltaObjectDiagram, 0, 6, 1, 1 );
	layoutSegmentation->addWidget( m_cboxDeltaObjectROIs, 0, 7, 1, 1 );
	layoutSegmentation->addWidget( m_cboxMinAreaObjs, 1, 0, 1, 1 );
	layoutSegmentation->addWidget( m_minAreaObjectsLEdit, 1, 1, 1, 1 );
	layoutSegmentation->addWidget( m_cboxMinLocsObjs, 1, 2, 1, 1 );
	layoutSegmentation->addWidget( m_minLocsObjectsLEdit, 1, 3, 1, 1 );
	layoutSegmentation->addWidget( applySegmentationBtn, 1, 4, 1, 1 );
	layoutSegmentation->addWidget( objectsOnWhatLbl, 1, 5, 1, 1 );
	layoutSegmentation->addWidget( m_cboxObjectOnDiagram, 1, 6, 1, 1 );
	layoutSegmentation->addWidget( m_cboxObjectOnROIs, 1, 7, 1, 1 );
	layoutSegmentation->addWidget(m_cboxMaxAreaObjs, 2, 0, 1, 1);
	layoutSegmentation->addWidget(m_maxAreaObjectsLEdit, 2, 1, 1, 1);
	layoutSegmentation->addWidget(m_cboxMaxLocsObjs, 2, 2, 1, 1);
	layoutSegmentation->addWidget(m_maxLocsObjectsLEdit, 2, 3, 1, 1);
	layoutSegmentation->addWidget(m_cboxPCAEllipse, 2, 6, 1, 1);
	layoutSegmentation->addWidget(m_cboxBoundingEllipse, 2, 7, 1, 1);
	m_groupSegmentation->setLayout(layoutSegmentation);
	m_groupSegmentation->setVisible( false );
	if (WATERSHED_DEFINED){
		layoutSegmentation->addWidget(watershedRadiusLbl, 3, 0, 1, 1);
		layoutSegmentation->addWidget(m_radiusWatershedLEdit, 3, 1, 1, 1);
		layoutSegmentation->addWidget(watershedLbl, 3, 2, 1, 1);
		layoutSegmentation->addWidget(m_nbLocsWatershedLEdit, 3, 3, 1, 1);
		layoutSegmentation->addWidget(m_cboxWatershed, 3, 4, 1, 1);
	}

	m_groupVoronoi = new QGroupBox( QObject::tr( "Voronoi" ) );
	m_groupVoronoi->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_filterVoronoiWidget = new FilterVoronoiDiagramWidget( NULL, _camera );
	m_filterVoronoiWidget->setWindowTitle( "Voronoi" );
	m_filterVoronoiWidget->setFeatures( QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetVerticalTitleBar );
	m_filterVoronoiWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	QVBoxLayout * layoutVoronoi = new QVBoxLayout;
	layoutVoronoi->addWidget( m_filterVoronoiWidget );
	m_groupVoronoi->setLayout( layoutVoronoi );
	m_groupVoronoi->setVisible( false );

	QWidget * empty = new QWidget;
	empty->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

	QVBoxLayout * layoutContruction = new QVBoxLayout;
	layoutContruction->addWidget( groupConstruction );
	layoutContruction->addWidget( m_groupVoronoi );
	layoutContruction->addWidget(empty);
	interactionWidget->setLayout( layoutContruction );

	m_groupVoronoiObjects = new QGroupBox( QObject::tr( "Voronoi Objects" ) );
	m_groupVoronoiObjects->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	m_cboxDisplayObjLabels = new QCheckBox( "Display label" );
	m_cboxDisplayObjLabels->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxDisplayObjLabels->setChecked( true );
	QCheckBox * cboxDisplayShapeObjs = new QCheckBox( "Display shape" );
	cboxDisplayShapeObjs->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	cboxDisplayShapeObjs->setChecked( true );
	QCheckBox * cboxDisplayOutlineObjs = new QCheckBox( "Display outline" );
	cboxDisplayOutlineObjs->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	cboxDisplayOutlineObjs->setChecked( true );
	QCheckBox * cboxDisplayEllipseObjs = new QCheckBox("Display ellipse");
	cboxDisplayEllipseObjs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	cboxDisplayEllipseObjs->setChecked(true);
	m_tableObjs = new QTableWidget;
	m_tableObjs->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	QStringList tableHeader;
	tableHeader << "Obj index" << "Area" << "# detections" << "Circularity" << "Diameter";
	m_tableObjs->setColumnCount( tableHeader.size() );
	m_tableObjs->setHorizontalHeaderLabels( tableHeader );
	QPushButton * exportStatsObjectsBtn = new QPushButton( "Export stats" );
	exportStatsObjectsBtn->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	QPushButton * clipboardObjectsBtn = new QPushButton("Copy clipboard");
	clipboardObjectsBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	QPushButton * locsIDObjectsBtn = new QPushButton("Export locs ID");
	locsIDObjectsBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

	QLabel * objShapeColorLbl = new QLabel("Shape color:");
	objShapeColorLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_colorObjShapeBtn = new QPushButton();
	m_colorObjShapeBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_colorObjShapeBtn->setStyleSheet("background-color: rgb(80, 120, 255);"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 5px;"
		"border-color: black;"
		"font: 12px;"
		"min-width: 5em;"
		"padding: 3px;"
		);
	QLabel * objOutlineColor = new QLabel("Outline color:");
	objOutlineColor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_colorObjOutlineBtn = new QPushButton();
	m_colorObjOutlineBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_colorObjOutlineBtn->setStyleSheet("background-color: rgb(255, 0, 0);"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 5px;"
		"border-color: black;"
		"font: 12px;"
		"min-width: 5em;"
		"padding: 3px;"
		);
	QLabel * objEllipseColor = new QLabel("Ellipse color:");
	objEllipseColor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_colorObjEllipseBtn = new QPushButton();
	m_colorObjEllipseBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_colorObjEllipseBtn->setStyleSheet("background-color: rgb(255, 255, 0);"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 5px;"
		"border-color: black;"
		"font: 12px;"
		"min-width: 5em;"
		"padding: 3px;"
		);
	m_transferColorObjBtn = new QPushButton("Transfer color to locs");
	m_transferColorObjBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

	QGridLayout * layoutObjectSelected = new QGridLayout;
	layoutObjectSelected->addWidget( m_cboxDisplayObjLabels, 0, 0, 1, 1 );
	layoutObjectSelected->addWidget( cboxDisplayShapeObjs, 0, 1, 1, 1 );
	layoutObjectSelected->addWidget( cboxDisplayOutlineObjs, 0, 2, 1, 1 );
	layoutObjectSelected->addWidget(cboxDisplayEllipseObjs, 0, 3, 1, 1);
	layoutObjectSelected->addWidget(exportStatsObjectsBtn, 0, 4, 1, 1);
	layoutObjectSelected->addWidget(clipboardObjectsBtn, 0, 5, 1, 1);
	layoutObjectSelected->addWidget(locsIDObjectsBtn, 0, 6, 1, 1);	
	layoutObjectSelected->addWidget(objShapeColorLbl, 1, 0, 1, 1);
	layoutObjectSelected->addWidget(m_colorObjShapeBtn, 1, 1, 1, 1);
	layoutObjectSelected->addWidget(objOutlineColor, 1, 2, 1, 1);
	layoutObjectSelected->addWidget(m_colorObjOutlineBtn, 1, 3, 1, 1);
	layoutObjectSelected->addWidget(objEllipseColor, 1, 4, 1, 1);
	layoutObjectSelected->addWidget(m_colorObjEllipseBtn, 1, 5, 1, 1);
	layoutObjectSelected->addWidget(m_transferColorObjBtn, 1, 6, 1, 1);
	layoutObjectSelected->addWidget(m_tableObjs, 2, 0, 1, 7);
	m_groupVoronoiObjects->setLayout( layoutObjectSelected );
	m_groupVoronoiObjects->setVisible( false );

	m_emptyForObjects = new QWidget;
	m_emptyForObjects->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	QVBoxLayout * layoutVObjects = new QVBoxLayout;
	layoutVObjects->addWidget( m_groupSegmentation );
	layoutVObjects->addWidget( m_groupVoronoiObjects );
	layoutVObjects->addWidget( m_emptyForObjects );
	voroObjectsWidget->setLayout( layoutVObjects );

	QGroupBox * clustersMiscGB = new QGroupBox( QObject::tr( "Clusters definition" ) );
	clustersMiscGB->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	QLabel * factorDensityClustLbl = new QLabel( "Density factor:" );
	factorDensityClustLbl->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_factorDensityClustersLEdit = new QLineEdit( "2" );
	m_factorDensityClustersLEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxMinLocsClusters = new QCheckBox( "Min # locs:" );
	m_cboxMinLocsClusters->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxMinLocsClusters->setChecked( true );
	m_minLocsClustersLEdit = new QLineEdit( "5" );
	m_minLocsClustersLEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxMinAreaClusters = new QCheckBox( "Min area:" );
	m_cboxMinAreaClusters->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxMinAreaClusters->setChecked( true );
	m_minAreaClustersLEdit = new QLineEdit( "2" );
	m_minAreaClustersLEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxClustersOnObject = new QCheckBox( "On objects" );
	m_cboxClustersOnObject->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxClustersOnROIs = new QCheckBox( "On ROIs" );
	m_cboxClustersOnROIs->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_buttonGroupClustersOnWhat = new QButtonGroup;
	m_buttonGroupClustersOnWhat->addButton( m_cboxClustersOnObject );
	m_buttonGroupClustersOnWhat->addButton( m_cboxClustersOnROIs );
	m_buttonGroupClustersOnWhat->setId( m_cboxClustersOnObject, 0 );
	m_buttonGroupClustersOnWhat->setId( m_cboxClustersOnROIs, 2 );
	m_cboxClustersOnObject->setChecked( true );
	QPushButton * createClustersBtn = new QPushButton( "Create clusters" );
	createClustersBtn->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	QLabel * clustersOnWhatLbl = new QLabel( "Cluster identification:" );
	clustersOnWhatLbl->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	QLabel * choiceForSigmaClustersLbl = new QLabel( "Computation of " + delta + ":" );
	choiceForSigmaClustersLbl->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxDeltaClusters = new QCheckBox( "On objects" );
	m_cboxDeltaClusters->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxDeltaClustersROIs = new QCheckBox( "On ROIs" );
	m_cboxDeltaClustersROIs->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	QButtonGroup * buttonGroupDeltaClusters = new QButtonGroup;
	buttonGroupDeltaClusters->addButton( m_cboxDeltaClusters );
	buttonGroupDeltaClusters->addButton( m_cboxDeltaClustersROIs );
	m_cboxDeltaClusters->setChecked( true );
	m_cboxMaxLocsClusters = new QCheckBox("Max # locs:");
	m_cboxMaxLocsClusters->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_cboxMaxLocsClusters->setChecked(true);
	m_maxLocsClustersLEdit = new QLineEdit("1000000");
	m_maxLocsClustersLEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_cboxMaxAreaClusters = new QCheckBox("Max area:");
	m_cboxMaxAreaClusters->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_cboxMaxAreaClusters->setChecked(true);
	m_maxAreaClustersLEdit = new QLineEdit("100000");
	m_maxAreaClustersLEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	QGridLayout * layoutClustersMisc = new QGridLayout;
	layoutClustersMisc->addWidget( factorDensityClustLbl, 0, 0, 1, 1 );
	layoutClustersMisc->addWidget( m_factorDensityClustersLEdit, 0, 1, 1, 1 );
	layoutClustersMisc->addWidget( choiceForSigmaClustersLbl, 0, 5, 1, 1 );
	layoutClustersMisc->addWidget( m_cboxDeltaClusters, 0, 6, 1, 1 );
	layoutClustersMisc->addWidget( m_cboxDeltaClustersROIs, 0, 7, 1, 1 );
	layoutClustersMisc->addWidget( m_cboxMinAreaClusters, 1, 0, 1, 1 );
	layoutClustersMisc->addWidget( m_minAreaClustersLEdit, 1, 1, 1, 1 );
	layoutClustersMisc->addWidget( m_cboxMinLocsClusters, 1, 2, 1, 1 );
	layoutClustersMisc->addWidget( m_minLocsClustersLEdit, 1, 3, 1, 1 );
	layoutClustersMisc->addWidget( createClustersBtn, 1, 4, 1, 1 );
	layoutClustersMisc->addWidget( clustersOnWhatLbl, 1, 5, 1, 1 );
	layoutClustersMisc->addWidget( m_cboxClustersOnObject, 1, 6, 1, 1 );
	layoutClustersMisc->addWidget( m_cboxClustersOnROIs, 1, 7, 1, 1 );
	layoutClustersMisc->addWidget(m_cboxMaxAreaClusters, 2, 0, 1, 1);
	layoutClustersMisc->addWidget(m_maxAreaClustersLEdit, 2, 1, 1, 1);
	layoutClustersMisc->addWidget(m_cboxMaxLocsClusters, 2, 2, 1, 1);
	layoutClustersMisc->addWidget(m_maxLocsClustersLEdit, 2, 3, 1, 1);
	clustersMiscGB->setLayout(layoutClustersMisc);

	m_groupVoronoiClusters = new QGroupBox( QObject::tr( "Voronoï clusters" ) );
	m_groupVoronoiClusters->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	QCheckBox * displayLabelClustCBox = new QCheckBox( "Display label" );
	displayLabelClustCBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	displayLabelClustCBox->setChecked( true );
	QCheckBox * displayShapeClustCBox = new QCheckBox( "Display shape" );
	displayShapeClustCBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	displayShapeClustCBox->setChecked( true );
	QCheckBox * displayOutlineClustCBox = new QCheckBox( "Display outline" );
	displayOutlineClustCBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	displayOutlineClustCBox->setChecked( true );
	QPushButton * exportStatsBtn = new QPushButton( "Export stats" );
	exportStatsBtn->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	QPushButton * clipboardClustersBtn = new QPushButton("Copy clipboard");
	clipboardClustersBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	QPushButton * locsIDClustersBtn = new QPushButton("Export locs ID");
	locsIDClustersBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_tableClusters = new QTableWidget;
	m_tableClusters->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	QStringList tableHeader2;
	tableHeader2 << "Obj index" << "# roi" << "Area" << "# detections" << "Barycenter" << "Circularity" << "Diameter";
	m_tableClusters->setColumnCount( tableHeader2.size() );
	m_tableClusters->setHorizontalHeaderLabels( tableHeader2 );
	
	QLabel * clusterShapeColorLbl = new QLabel("Shape color:");
	clusterShapeColorLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_colorClusterShapeBtn = new QPushButton();
	m_colorClusterShapeBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_colorClusterShapeBtn->setStyleSheet("background-color: rgb(100, 205, 40);"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 5px;"
		"border-color: black;"
		"font: 12px;"
		"min-width: 5em;"
		"padding: 3px;"
		);
	QLabel * clusterOutlineColor = new QLabel("Outline color:");
	clusterOutlineColor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_colorClusterOutlineBtn = new QPushButton();
	m_colorClusterOutlineBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_colorClusterOutlineBtn->setStyleSheet("background-color: rgb(255, 0, 0);"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 5px;"
		"border-color: black;"
		"font: 12px;"
		"min-width: 5em;"
		"padding: 3px;"
		);
	m_transferColorClusterBtn = new QPushButton("Transfer color to locs");
	m_transferColorClusterBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	/*QLabel * clusterEllipseColor = new QLabel("Ellipse color:");
	clusterEllipseColor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_colorClusterEllipseBtn = new QPushButton();
	m_colorClusterEllipseBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_colorClusterEllipseBtn->setStyleSheet("background-color: rgb(255, 255, 0);"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 5px;"
		"border-color: black;"
		"font: 12px;"
		"min-width: 5em;"
		"padding: 3px;"
		);*/
	QGridLayout * layoutInfosClusters = new QGridLayout;
	layoutInfosClusters->addWidget( displayLabelClustCBox, 0, 0, 1, 1 );
	layoutInfosClusters->addWidget( displayShapeClustCBox, 0, 1, 1, 1 );
	layoutInfosClusters->addWidget( displayOutlineClustCBox, 0, 2, 1, 1 );
	layoutInfosClusters->addWidget( exportStatsBtn, 0, 3, 1, 1 );
	layoutInfosClusters->addWidget(clipboardClustersBtn, 0, 4, 1, 1);
	layoutInfosClusters->addWidget(locsIDClustersBtn, 0, 5, 1, 1);
	layoutInfosClusters->addWidget(clusterShapeColorLbl, 1, 0, 1, 1);
	layoutInfosClusters->addWidget(m_colorClusterShapeBtn, 1, 1, 1, 1);
	layoutInfosClusters->addWidget(clusterOutlineColor, 1, 2, 1, 1);
	layoutInfosClusters->addWidget(m_colorClusterOutlineBtn, 1, 3, 1, 1);
	layoutInfosClusters->addWidget(m_transferColorClusterBtn, 1, 4, 1, 1);
	//layoutInfosClusters->addWidget(clusterEllipseColor, 1, 4, 1, 1);
	//layoutInfosClusters->addWidget(m_colorClusterEllipseBtn, 1, 5, 1, 1);
	layoutInfosClusters->addWidget(m_tableClusters, 2, 0, 1, 5);
	m_groupVoronoiClusters->setLayout( layoutInfosClusters );
	m_groupVoronoiClusters->setVisible( false );

	m_emptyForClusters = new QWidget;
	m_emptyForClusters->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

	QVBoxLayout * layoutClusters = new QVBoxLayout;
	layoutClusters->addWidget( clustersMiscGB );
	layoutClusters->addWidget( m_groupVoronoiClusters );
	layoutClusters->addWidget( m_emptyForClusters );
	m_clustersWidget->setLayout( layoutClusters );

	this->addTab( interactionWidget, tr( "Voronoi construction" ) );
	this->addTab( voroObjectsWidget, tr( "Objects" ) );

	QObject::connect( applyDensityFacor, SIGNAL( pressed() ), this, SLOT( applyDensityFactor() ) );
	QObject::connect( applySegmentationBtn, SIGNAL( pressed() ), this, SLOT( segmentVoronoi() ) );
	QObject::connect( exportStatsBtn, SIGNAL( pressed() ), this, SLOT( exportStatsClustersObjects() ) );
	QObject::connect( exportStatsObjectsBtn, SIGNAL( pressed() ), this, SLOT( exportStatsObjects() ) );
	QObject::connect(clipboardObjectsBtn, SIGNAL(pressed()), this, SLOT(exportObjectsToClipboard()));
	QObject::connect(clipboardClustersBtn, SIGNAL(pressed()), this, SLOT(exportClustersToClipboard()));
	QObject::connect(locsIDObjectsBtn, SIGNAL(pressed()), this, SLOT(exportLocalizationsIDObjects()));
	QObject::connect(locsIDClustersBtn, SIGNAL(pressed()), this, SLOT(exportLocalizationsIDClusters()));
	QObject::connect(m_buttonCreation, SIGNAL(pressed()), this, SLOT(createVoronoi()));

	QObject::connect( cboxDisplayShapeObjs, SIGNAL( toggled( bool ) ), this, SLOT( toggleDisplayShapeObjs( bool ) ) );
	QObject::connect( cboxDisplayOutlineObjs, SIGNAL( toggled( bool ) ), this, SLOT( toggleDisplayOutlineObjs( bool ) ) );
	QObject::connect( m_cboxDisplayObjLabels, SIGNAL( toggled( bool ) ), _camera, SLOT( toggleDisplayObjectLabels( bool ) ) );
	QObject::connect(cboxDisplayEllipseObjs, SIGNAL(toggled(bool)), this, SLOT(toggleDisplayObjectEllipses(bool)));
	QObject::connect(displayLabelClustCBox, SIGNAL(toggled(bool)), _camera, SLOT(toggleDisplayClusterLabels(bool)));

	QObject::connect( createClustersBtn, SIGNAL( pressed() ), this, SLOT( createClusters() ) );
	QObject::connect( displayShapeClustCBox, SIGNAL( toggled( bool ) ), this, SLOT( toggleDisplayShapeClusts( bool ) ) );
	QObject::connect(displayOutlineClustCBox, SIGNAL(toggled(bool)), this, SLOT(toggleDisplayOutlineClusts(bool))); 

	QObject::connect(m_colorObjShapeBtn, SIGNAL(pressed()), this, SLOT(changeColorSlot()));
	QObject::connect(m_colorObjOutlineBtn, SIGNAL(pressed()), this, SLOT(changeColorSlot()));
	QObject::connect(m_colorObjEllipseBtn, SIGNAL(pressed()), this, SLOT(changeColorSlot()));
	QObject::connect(m_colorClusterShapeBtn, SIGNAL(pressed()), this, SLOT(changeColorSlot()));
	QObject::connect(m_colorClusterOutlineBtn, SIGNAL(pressed()), this, SLOT(changeColorSlot()));
	//QObject::connect(m_colorClusterEllipseBtn, SIGNAL(pressed()), this, SLOT(changeColorSlot()));
	QObject::connect(m_transferColorObjBtn, SIGNAL(pressed()), this, SLOT(transferColorToLocs()));
	QObject::connect(m_transferColorClusterBtn, SIGNAL(pressed()), this, SLOT(transferColorToLocs()));

}

VoronoiWidget::~VoronoiWidget()
{

}

void VoronoiWidget::setHistogramData()
{
	m_filterVoronoiWidget->setHistogramData( m_currentCamera->getVoronoiDiagram(), m_currentCamera );
	m_filterVoronoiWidget->setVisible( true );
}

void VoronoiWidget::setWrapperVoronoi( WrapperVoronoiDiagram * _voronoi )
{
	if( _voronoi == NULL ){
		m_groupSegmentation->setVisible( false );
		m_filterVoronoiWidget->setVisible( false );
		m_groupVoronoi->setVisible( false );
	}
	else{
		m_factorDensityObjectLEdit->setText( QString::number( _voronoi->getFactorDensity() ) );
		m_filterVoronoiWidget->setHistogramData( _voronoi, m_currentCamera );
		double minH, maxH, maxY, stepX;
		_voronoi->getHistogramParameters( minH, maxH, stepX, maxY, _voronoi->whatTypeHistogram(), _voronoi->isLogHistogram() );
		m_groupSegmentation->setVisible( true );
		m_filterVoronoiWidget->setVisible( true );
		m_groupVoronoi->setVisible( true );
	}
	updateObjectsList();
}

void VoronoiWidget::updateObjectsList()
{
	m_tableObjs->clear();
	QStringList tableHeader;
	tableHeader << "Obj index" << "Area" << "# detections" << "Circularity" << "Diameter";
	m_tableObjs->setHorizontalHeaderLabels( tableHeader );
	const NeuronObjectList & objects = m_currentCamera->getNeuronObjects();
	m_tableObjs->setRowCount( objects.size() );
	if( !objects.empty() ){
		for( int i = 0; i < objects.size(); i++ ){
			int y = 0;
			VoronoiObject * vobj = objects.at( i )->getObject();
			double area = vobj->getArea();
			m_tableObjs->setItem( i, y++, new QTableWidgetItem( QString::number( i + 1 ) ) );
			m_tableObjs->setItem( i, y++, new QTableWidgetItem( QString::number( area ) ) );
			m_tableObjs->setItem( i, y++, new QTableWidgetItem( QString::number( vobj->nbMolecules() ) ) );
			m_tableObjs->setItem( i, y++, new QTableWidgetItem( QString::number( vobj->getData( VoronoiCluster::Circularity ) ) ) );
			m_tableObjs->setItem( i, y++, new QTableWidgetItem( QString::number( vobj->getData( VoronoiCluster::Diameter ) ) ) );
		}
		m_tableObjs->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
		m_groupVoronoiObjects->setVisible(true);
		m_emptyForObjects->setVisible( false );
		this->addTab( m_clustersWidget, tr( "Clusters" ) );
	}
	else{
		m_groupVoronoiObjects->setVisible( false );
		m_emptyForObjects->setVisible( true );
		this->removeTab( this->indexOf( m_clustersWidget ) );
	}
}

void VoronoiWidget::setCurrentCamera( Camera2D * _cam )
{
	m_currentCamera = _cam;
	setWrapperVoronoi( m_currentCamera->getVoronoiDiagram() );
}

void VoronoiWidget::changeMinHistogram( const double _min )
{
	double val = ( m_filterVoronoiWidget->isLogChecked() ) ? MiscFunction::log10Custom( _min ) : _min;
	double res = m_filterVoronoiWidget->getHistogramCamera()->setMinHistogram( val );
}

void VoronoiWidget::applyDensityFactor()
{
	WrapperVoronoiDiagram * voronoi = m_currentCamera->getVoronoiDiagram();
	if( voronoi == NULL )return;
	bool ok;
	double factor = m_factorDensityObjectLEdit->text().toDouble( &ok );
	factor = ( ok ) ? factor : 1.;
	voronoi->setFactorDensity( factor );
	if( voronoi->whatTypeHistogram() == MoleculeInfos::LocalDensity ){
		if( m_cboxDeltaObjectDiagram->isChecked() && m_cboxObjectOnDiagram->isChecked() ){
			double val = voronoi->getFactorDensity() * voronoi->getAverageDensity();
			changeMinHistogram( val );
		}
		else{
			bool ok;
			double factor = m_factorDensityObjectLEdit->text().toDouble( &ok );
			if( !ok ) factor = 1.;
			voronoi->applyDensityFactorROIs( factor, m_cboxDeltaObjectROIs->isChecked(), m_cboxObjectOnROIs->isChecked(), m_currentCamera->getSuperResObject()->getRois() );
		}
		m_currentCamera->updateGL();
	}
}

void VoronoiWidget::segmentVoronoi()
{
	m_groupVoronoiClusters->setVisible( false );
	m_emptyForClusters->setVisible( true );

	QTime time;
	time.start();
	double minArea = 0., cutD = DBL_MAX, radiusWatershed = 0., nbLocsWatershed = DBL_MAX, maxArea = DBL_MAX;
	bool watershed = false;
	bool ok;
	double tmp = m_minAreaObjectsLEdit->text().toDouble( &ok );
	if( ok && m_cboxMinAreaObjs->isChecked() ) minArea = tmp;
	tmp = m_maxAreaObjectsLEdit->text().toDouble(&ok);
	if (ok && m_cboxMaxAreaObjs->isChecked()) maxArea = tmp;
	tmp = m_cutDistObjectsLEdit->text().toDouble(&ok);
	if( ok && m_cboxCutDistObjs->isChecked() ) cutD = tmp * tmp;
	unsigned int minLocs = 1, maxLocs = UINT_MAX;
	unsigned int tmp2 = m_minLocsObjectsLEdit->text().toUInt( &ok );
	if( ok && m_cboxMinLocsObjs->isChecked() ) minLocs = tmp2;
	tmp2 = m_maxLocsObjectsLEdit->text().toUInt(&ok);
	if (ok && m_cboxMaxLocsObjs->isChecked()) maxLocs = tmp2;
	if (m_cboxWatershed != NULL){
		tmp = m_radiusWatershedLEdit->text().toDouble(&ok);
		if (ok) radiusWatershed = tmp;
		tmp = m_nbLocsWatershedLEdit->text().toDouble(&ok);
		if (ok) nbLocsWatershed = tmp;
		watershed = m_cboxWatershed->isChecked();
	}
	WrapperVoronoiDiagram * voronoi = m_currentCamera->getVoronoiDiagram();
	if( voronoi == NULL )return;
	SuperResObject * sobj = m_currentCamera->getSuperResObject();
	if( m_cboxObjectOnROIs->isChecked() )
		sobj->removeObjectsInsideROIs( sobj->getRois() );
	else
		sobj->removeObjectsInsideROIs( RoiList() );
	sobj->addNeuronObjects( voronoi->createVoronoiObjects( minArea, minLocs, maxArea, maxLocs, m_cboxCutDistObjs->isChecked(), cutD, m_cboxPCAEllipse->isChecked(), watershed, radiusWatershed, nbLocsWatershed ) );
	int elapsedTime = time.elapsed();
	time.restart();
	QTime test = QTime();
	test = test.addMSecs( elapsedTime );
	std::cout << "\nElapsed time for creation of the Voronoi objects [" << test.hour() << ":" << test.minute() << ":" << test.second() << ":" << test.msec() << "] (h:min:s:ms)" << std::endl;
	m_currentCamera->updateGL();
	updateObjectsList();
}


void VoronoiWidget::createClusters()
{
	std::cout << "Beginning identification of clusters" << std::endl;
	QTime time;
	time.start();

	const RoiList & rois = m_currentCamera->getSuperResObject()->getRois();
	if( ( m_cboxClustersOnROIs->isChecked() || m_cboxDeltaClustersROIs->isChecked() ) && rois.empty() ) return;

	m_tableClusters->clear();
	QStringList tableHeader2;
	tableHeader2 << "Obj index" << "# roi" << "Area" << "# detections" << "Barycenter" << "Circularity" << "Diameter";
	m_tableClusters->setHorizontalHeaderLabels( tableHeader2 );
	m_tableClusters->setRowCount( 0 );
	m_groupVoronoiClusters->setVisible( true );
	m_emptyForClusters->setVisible( false );

	const NeuronObjectList & neuronObjects = m_currentCamera->getNeuronObjects();
	if( neuronObjects.empty() ) return;

	double minArea = 0., factor = 1., maxArea = DBL_MAX;
	bool ok;
	double tmp = m_minAreaClustersLEdit->text().toDouble( &ok );
	if( ok && m_cboxMinAreaClusters->isChecked() ) minArea = tmp;
	tmp = m_maxAreaClustersLEdit->text().toDouble(&ok);
	if (ok && m_cboxMaxAreaClusters->isChecked()) maxArea = tmp;
	tmp = m_factorDensityClustersLEdit->text().toDouble( &ok );
	if( ok ) factor = tmp;
	unsigned int minLocs = 1, maxLocs = UINT_MAX;
	unsigned int tmp2 = m_minLocsClustersLEdit->text().toUInt( &ok );
	if( ok && m_cboxMinLocsClusters->isChecked() ) minLocs = tmp2;
	tmp2 = m_maxLocsClustersLEdit->text().toUInt(&ok);
	if (ok && m_cboxMaxLocsClusters->isChecked()) maxLocs = tmp2;

	WrapperVoronoiDiagram * voronoi = m_currentCamera->getVoronoiDiagram();
	MoleculeInfos * infos = voronoi->getMoleculeInfos();
	int nbMolVoro = voronoi->nbMolecules(), nbTrianglesDelau = voronoi->getNbFiniteTriangles();
	NeuronObjectList objects = m_currentCamera->getNeuronObjects();

	unsigned int * roisIndex = new unsigned int[nbMolVoro];
	memset( roisIndex, 0, nbMolVoro * sizeof( unsigned int ) );
	unsigned int * objsIndex = new unsigned int[nbMolVoro];
	memset( objsIndex, 0, nbMolVoro * sizeof( unsigned int ) );
	int curObject = 1;

	bool * polygonsSelected = new bool[nbMolVoro], * polygonsSelectedOnROIs = new bool[nbMolVoro];
	memset( polygonsSelected, 0, nbMolVoro * sizeof( bool ) );
	memset( polygonsSelectedOnROIs, 0, nbMolVoro * sizeof( bool ) );

	//computation of the number of polygons of the objects
	unsigned int cptProgress = 0, nbMolObjs = 0;
	for( NeuronObjectList::iterator it = objects.begin(); it != objects.end(); it++ ){
		VoronoiObject * obj = ( *it )->getObject();
		nbMolObjs += obj->nbMolecules();
	}
	GeneralTools::m_imw->m_progress->setMaximum( nbMolObjs );
	GeneralTools::m_imw->m_progress->setValue( cptProgress++ );

	unsigned int nbPolygons = 0, nbPolygonsROIs = 0, nbPolygonsROIsBefore = 0, currentRoi = 1;
	bool insideROIs = false;

	//Determination of the polygons selected for the complete objects and the polygons inside the ROIs
	for( NeuronObjectList::iterator it = objects.begin(); it != objects.end(); it++, curObject++ ){
		VoronoiObject * obj = ( *it )->getObject();
		unsigned int * clusterPolygons = obj->getMolecules(), nbPolCluster = obj->nbMolecules();
		for( unsigned int n = 0; n < nbPolCluster; n++ ){
			GeneralTools::m_imw->m_progress->setValue( cptProgress++ );
			unsigned int index = clusterPolygons[n];
			objsIndex[index] = curObject;
	
			polygonsSelected[index] = true;
			nbPolygons++;

			nbPolygonsROIsBefore = nbPolygonsROIs;
			if( m_cboxDeltaClustersROIs->isChecked() || m_cboxClustersOnROIs->isChecked() ){
				VertHandle v = infos[index].getMolecule();
				insideROIs = false;
				currentRoi = 1;
				for( RoiList::const_iterator it2 = rois.begin(); it2 != rois.end() && !insideROIs; it2++, currentRoi++ ){
					insideROIs = it2->inside( v->point().x(), v->point().y() );
					if( insideROIs ){
						roisIndex[index] = currentRoi;
						polygonsSelectedOnROIs[index] = true;
						nbPolygonsROIs++;
					}
				}
			}

			//The clusters of an object are discarded if clusters are identified on all objects or clusters are identified in objects inside the ROIs (identified if nbPolygonsROIs != nbPolygonsROIsBefore)
			if( m_cboxClustersOnObject->isChecked() || ( m_cboxClustersOnROIs->isChecked() && nbPolygonsROIs != nbPolygonsROIsBefore ) )
				( *it )->clearClusters();
		}
	}

	bool * correctPolygonsSelected;
	unsigned int nbCorrectPolygonsSelected = 0;
	if( m_cboxDeltaClusters->isChecked() ){
		correctPolygonsSelected = polygonsSelected;
		nbCorrectPolygonsSelected = nbPolygons;
	}
	else if( m_cboxDeltaClustersROIs->isChecked() ){
		correctPolygonsSelected = polygonsSelectedOnROIs;
		nbCorrectPolygonsSelected = nbPolygonsROIs;
	}

	//Determination of the mean density
	double delta = 0., areaInsideROIs = 0.;
	for( unsigned int n = 0; n < nbMolVoro; n++ )
		if( correctPolygonsSelected[n] )
			areaInsideROIs += infos[n].getData( MoleculeInfos::Area );
	delta = ( double )nbCorrectPolygonsSelected / areaInsideROIs;

	if( m_cboxClustersOnObject->isChecked() )
		correctPolygonsSelected = polygonsSelected;
	else if( m_cboxClustersOnROIs->isChecked() )
		correctPolygonsSelected = polygonsSelectedOnROIs;

	//Application of the threshold
	double thresh = factor * delta;
	for( unsigned int n = 0; n < nbMolVoro; n++ )
		if( correctPolygonsSelected[n] )
			correctPolygonsSelected[n] = voronoi->getInfosData( MoleculeInfos::LocalDensity, n ) > thresh;

	//creation of the clusters
	VoronoiClusterList clusters;
	VoronoiClusterList::determineClusters( voronoi, correctPolygonsSelected, minLocs, minArea, maxLocs, maxArea, clusters );
	for( VoronoiClusterList::iterator it = clusters.begin(); it != clusters.end(); it++ ){
		VoronoiCluster * cluster = *it;
		unsigned int indexPolygon = cluster->getMolecules()[0];
		unsigned int indexObj = objsIndex[indexPolygon];
		NeuronObject * nobj = *( objects.begin() + ( indexObj - 1 ) );
		nobj->addCluster( cluster );
	}

	curObject = 1;
	for( NeuronObjectList::iterator it = objects.begin(); it != objects.end(); it++, curObject++ ){
		NeuronObject * nobj = *it;

		for( unsigned int n = 0; n < nobj->nbClusters(); n++ ){
			int currentRow = m_tableClusters->rowCount(), y = 0;
			m_tableClusters->setRowCount( m_tableClusters->rowCount() + 1 );

			VoronoiCluster * cluster = nobj->getCluster( n );
			unsigned int indexPolygon = cluster->getMolecules()[0];
			unsigned int indexROI = roisIndex[indexPolygon];

			int y2 = 0;
			m_tableClusters->setItem( currentRow, y2++, new QTableWidgetItem( QString::number( curObject ) ) );
			m_tableClusters->setItem( currentRow, y2++, new QTableWidgetItem( ( indexROI == 0 ) ? QString( "-" ) : QString::number( indexROI ) ) );
			m_tableClusters->setItem( currentRow, y2++, new QTableWidgetItem( QString::number( cluster->getArea() ) ) );
			m_tableClusters->setItem( currentRow, y2++, new QTableWidgetItem( QString::number( cluster->nbMolecules() ) ) );
			Vec2mf bary = cluster->getBarycenter();
			m_tableClusters->setItem( currentRow, y2++, new QTableWidgetItem( "[" + QString::number( bary.x() ) + ", " + QString::number( bary.y() ) + "]" ) );
			m_tableClusters->setItem( currentRow, y2++, new QTableWidgetItem( QString::number( cluster->getData( VoronoiCluster::Circularity ) ) ) );
			m_tableClusters->setItem( currentRow, y2++, new QTableWidgetItem( QString::number( cluster->getData( VoronoiCluster::Diameter ) ) ) );
		}
		m_tableClusters->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
		nobj->generateDisplayClusters();
	}

	clusters.clear();
	delete [] polygonsSelected;
	delete [] polygonsSelectedOnROIs;
	delete [] objsIndex;
	delete [] roisIndex;

	int elapsedTime = time.elapsed();
	time.restart();
	QTime test2 = QTime();
	test2 = test2.addMSecs( elapsedTime );
	std::cout << "Ending identification of clusters, elapsed time [" << test2.hour() << ":" << test2.minute() << ":" << test2.second() << ":" << test2.msec() << "] (h:min:s:ms)" << std::endl;

	m_currentCamera->updateGL();
}

void VoronoiWidget::exportStatsClustersObjects()
{
	bool ok;
	double factor = 3.;
	int nMinMol = 20;
	double test = m_factorDensityClustersLEdit->text().toDouble( &ok );
	if( ok )
		factor = test;
	int nMinMolTmp = m_minLocsClustersLEdit->text().toInt( &ok );
	if( ok )
		nMinMol = nMinMolTmp;

	SuperResObject * obj = m_currentCamera->getSuperResObject();
	obj->exportStats( factor, nMinMol );
}

void VoronoiWidget::exportStatsObjects()
{
	SuperResObject * obj = m_currentCamera->getSuperResObject();
	obj->exportStats();
}

void VoronoiWidget::createVoronoi()
{
	m_currentCamera->createVoronoiDiagram( m_cboxdsetCleaner->isChecked() );
	setWrapperVoronoi( m_currentCamera->getVoronoiDiagram() );
	m_currentCamera->updateGL();
}

void VoronoiWidget::toggleDisplayShapeObjs( bool _val )
{
	NeuronObjectList objects = m_currentCamera->getNeuronObjects();
	for( NeuronObjectList::iterator it = objects.begin(); it != objects.end(); it++ )
		( *it )->toggleDisplayShapeObjs( _val );
	m_currentCamera->updateGL();
}

void VoronoiWidget::toggleDisplayOutlineObjs( bool _val )
{
	NeuronObjectList objects = m_currentCamera->getNeuronObjects();
	for( NeuronObjectList::iterator it = objects.begin(); it != objects.end(); it++ )
		( *it )->toggleDisplayOutlineObjs( _val );
	m_currentCamera->updateGL();
}

void VoronoiWidget::toggleDisplayObjectEllipses(bool _val)
{
	NeuronObjectList objects = m_currentCamera->getNeuronObjects();
	for (NeuronObjectList::iterator it = objects.begin(); it != objects.end(); it++)
		(*it)->toggleDisplayEllipseObjs(_val);
	m_currentCamera->updateGL();
}

void VoronoiWidget::toggleDisplayShapeClusts( bool _val )
{
	NeuronObjectList objects = m_currentCamera->getNeuronObjects();
	for( NeuronObjectList::iterator it = objects.begin(); it != objects.end(); it++ )
		( *it )->toggleDisplayShapeClusts( _val );
	m_currentCamera->updateGL();
}

void VoronoiWidget::toggleDisplayOutlineClusts( bool _val )
{
	NeuronObjectList objects = m_currentCamera->getNeuronObjects();
	for( NeuronObjectList::iterator it = objects.begin(); it != objects.end(); it++ )
		( *it )->toggleDisplayOutlineClusts( _val );
	m_currentCamera->updateGL();
}

void VoronoiWidget::exportObjectsToClipboard()
{
	QString selected_text;
	QList<QTableWidgetItem *> indexes;
	for (unsigned int column = 0; column < m_tableObjs->columnCount(); column++)
		indexes.push_back(m_tableObjs->horizontalHeaderItem(column));
	// You need a pair of indexes to find the row changes
	for (unsigned int row = 0; row < m_tableObjs->rowCount(); row++)
		for (unsigned int column = 0; column < m_tableObjs->columnCount(); column++)
			indexes.push_back(m_tableObjs->item(row, column));
	const QTableWidgetItem * previous = indexes.first(), * current;
	selected_text.append(previous->text());
	indexes.removeFirst();
	foreach(current, indexes)
	{
		// If you are at the start of the row the row number of the previous index
		// isn't the same.  Text is followed by a row separator, which is a newline.
		if (current->row() != previous->row())
		{
			selected_text.append('\n');
		}
		// Otherwise it's the same row, so append a column separator, which is a tab.
		else
		{
			selected_text.append('\t');
		}
		selected_text.append(current->text());
		previous = current;
	}
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(selected_text);
}

void VoronoiWidget::exportClustersToClipboard()
{
	QString selected_text("Cluster index\t");
	for (unsigned int column = 0; column < m_tableClusters->columnCount(); column++){
		selected_text.append(m_tableClusters->horizontalHeaderItem(column)->text());
		if (column < m_tableClusters->columnCount() - 1)
			selected_text.append("\t");
	}
	for (unsigned int row = 0; row < m_tableClusters->rowCount(); row++){
		selected_text.append("\n");
		selected_text.append(QString::number(row)).append('\t');
		for (unsigned int column = 0; column < m_tableClusters->columnCount(); column++){
			selected_text.append(m_tableClusters->item(row, column)->text());
			if (column < m_tableClusters->columnCount() - 1)
				selected_text.append("\t");
		}
	}
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(selected_text);
}

void VoronoiWidget::exportLocalizationsIDObjects()
{
	SuperResObject * obj = m_currentCamera->getSuperResObject();
	obj->exportIDLocalizations(false);
}

void VoronoiWidget::exportLocalizationsIDClusters()
{
	SuperResObject * obj = m_currentCamera->getSuperResObject();
	obj->exportIDLocalizations(true);
}

void VoronoiWidget::changeColorSlot()
{
	QObject * sender = QObject::sender();
	if (sender == m_colorObjShapeBtn)
		changeColor(m_colorObjShapeBtn, SuperResObject::OBJECT_SHAPE);
	else if (sender == m_colorObjOutlineBtn)
		changeColor(m_colorObjOutlineBtn, SuperResObject::OBJECT_OUTLINE);
	else if (sender == m_colorObjEllipseBtn)
		changeColor(m_colorObjEllipseBtn, SuperResObject::OBJECT_ELLIPSE);
	else if (sender == m_colorClusterShapeBtn)
		changeColor(m_colorClusterShapeBtn, SuperResObject::CLUSTER_SHAPE);
	else if (sender == m_colorClusterOutlineBtn)
		changeColor(m_colorClusterOutlineBtn, SuperResObject::CLUSTER_OUTLINE);
	//else if (sender == m_colorClusterEllipseBtn)
	//	changeColor(m_colorClusterEllipseBtn, SuperResObject::CLUSTER_ELLIPSE);
}

void VoronoiWidget::changeColor(QPushButton * _btn, const int _type)
{
	SuperResObject * sobj = m_currentCamera->getCurrentObject();
	sobj->changeColor(_type);
	const Color4D color = sobj->getColor(_type);
	_btn->setStyleSheet("background-color: rgb(" + QString::number((int)(color[0] * 255.f)) + ", " + QString::number((int)(color[1] * 255.f)) + ", " + QString::number((int)(color[2] * 255.f)) + ");"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 5px;"
		"border-color: black;"
		"font: 12px;"
		"min-width: 5em;"
		"padding: 3px;"
		);
	m_currentCamera->updateGL();
}

void VoronoiWidget::transferColorToLocs()
{
	SuperResObject * sobj = m_currentCamera->getCurrentObject();
	QObject * sender = QObject::sender();
	if (sender == m_transferColorObjBtn)
		sobj->transferColorVoronoiObjsToLocs();
	else if (sender == m_transferColorClusterBtn)
		sobj->transferColorVoronoiClustersToLocs();
	m_currentCamera->updateGL();
}
