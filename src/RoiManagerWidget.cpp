/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      RoiManagerWidget.cpp
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

#include <QGridLayout>
#include <QFileDialog>
#include <fstream>

#include "RoiManagerWidget.hpp"

RoiManagerWidget::RoiManagerWidget( Camera2D * _camera, QWidget * _parent, Qt::WFlags _f ):QWidget( _parent, _f )
{
	m_camera = _camera;

	m_groupRois = new QGroupBox( QObject::tr( "ROI Manager" ) );
	m_groupRois->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_roisCombo = new QComboBox;
	m_roisCombo->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_loadRoisBtn = new QPushButton( "Load rois" );
	m_loadRoisBtn->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_saveRoisBtn = new QPushButton( "Save rois" );
	m_saveRoisBtn->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_discardCurRoiBtn = new QPushButton( "Discard current ROI" );
	m_discardCurRoiBtn->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_discardAllRoisBtn = new QPushButton( "Discard all ROIs" );
	m_discardAllRoisBtn->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxDisplayLabelRoi = new QCheckBox( "Display label" );
	m_cboxDisplayLabelRoi->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_cboxDisplayLabelRoi->setChecked( true );
	QGridLayout * layoutRoi = new QGridLayout;
	layoutRoi->addWidget( m_roisCombo, 0, 0, 1, 2 );
	layoutRoi->addWidget( m_cboxDisplayLabelRoi, 0, 3, 1, 1 );
	layoutRoi->addWidget( m_loadRoisBtn, 1, 0, 1, 1 );
	layoutRoi->addWidget( m_saveRoisBtn, 1, 1, 1, 1 );
	layoutRoi->addWidget( m_discardCurRoiBtn, 1, 2, 1, 1 );
	layoutRoi->addWidget( m_discardAllRoisBtn, 1, 3, 1, 1 );
	m_groupRois->setLayout( layoutRoi );
	m_groupRois->setVisible( true );

	QWidget * empty3 = new QWidget;
	empty3->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	QVBoxLayout * layout = new QVBoxLayout;
	layout->addWidget( m_groupRois );
	layout->addWidget( empty3 );
	this->setLayout( layout );

	QObject::connect( m_loadRoisBtn, SIGNAL( pressed() ), _camera, SLOT( loadRois() ) );//
	QObject::connect( m_saveRoisBtn, SIGNAL( pressed() ), this, SLOT( saveRois() ) );
	QObject::connect( m_discardCurRoiBtn, SIGNAL( pressed() ), this, SLOT( discardCurrentRoi() ) );
	QObject::connect( m_discardAllRoisBtn, SIGNAL( pressed() ), this, SLOT( discardAllRois() ) );
	QObject::connect( m_cboxDisplayLabelRoi, SIGNAL( toggled( bool ) ), _camera, SLOT( toggleDisplayLabelRoi( bool ) ) );
}

RoiManagerWidget::~RoiManagerWidget()
{

}

void RoiManagerWidget::discardCurrentRoi()
{
	unsigned int indeROI = m_roisCombo->currentIndex();
	if( indeROI >= m_roisCombo->count() ) return;
	int nbRois = m_camera->getSuperResObject()->nbRois();
	m_camera->getSuperResObject()->discardRoi( indeROI );
	m_camera->updateGL();
	m_roisCombo->clear();
	if( nbRois == 1 ) return;
	for(int n = 1; n < nbRois; n++ )
		m_roisCombo->addItem( "Roi " + QString::number( n ) );
}

void RoiManagerWidget::discardAllRois()
{
	m_camera->getSuperResObject()->discardAllRois();
	m_camera->updateGL();
	m_roisCombo->clear();
}

void RoiManagerWidget::addRoiToRoiManager()
{
	m_roisCombo->addItem( "Roi " + QString::number( m_roisCombo->count() + 1 ) );
	m_roisCombo->setCurrentIndex( m_roisCombo->count() - 1 );
}

void RoiManagerWidget::saveRois()
{
	SuperResObject * obj = m_camera->getSuperResObject();
	const RoiList & rois = obj->getRois();

	QString tmp( obj->getDir().c_str() );
	QString nameXls( tmp );
	nameXls.append( "/userDefinedRois.txt" );
	nameXls = QFileDialog::getSaveFileName(this, QObject::tr("Save ROIs to..."), nameXls, QObject::tr("Text files (*.txt)"), 0, QFileDialog::DontUseNativeDialog);
	if( nameXls.isEmpty() ){
		std::cout << "Empty name" << std::endl;
		return;
	}
	std::cout << nameXls.toAscii().data() << std::endl;
	std::ofstream fs( nameXls.toAscii().data() );
	fs << rois.size() << std::endl;

	for( RoiList::const_iterator it = rois.begin(); it != rois.end(); it++ ){
		const Roi & roi = *it;
		fs << roi.size() << std::endl;
		/*for( Roi::Vertex_const_iterator it2 = roi.vertices_begin(); it2 != roi.vertices_end(); it2++ )
			fs << it2->x() << " " << it2->y() << std::endl;*/
		for( Roi::const_iterator it2 = roi.begin(); it2 != roi.end(); it2++ )
			fs << it2->x() << " " << it2->y() << std::endl;
	}
	fs.close();
}
