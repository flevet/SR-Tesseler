/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      MainFilterDialog.cpp
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

#include <QVBoxLayout>

#include "MainFilterDialog.hpp"
#include "FilterObjectWidget.hpp"
#include "DetectionSet.hpp"
#include "MiscFilterWidget.hpp"


MainFilterDialog::MainFilterDialog( Camera2D * _cam, QWidget* _parent /*= 0*/, Qt::WFlags _flags /*= 0 */ )
{
	m_filterDetectionsWidget = NULL;

	m_miscFilter = new MiscFilterWidget( _cam );
	m_miscFilter->setFeatures( QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetVerticalTitleBar );
	m_miscFilter->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum );
	m_filterDetectionsWidget = new FilterDetectionWidget( _cam->getDetectionSet(), _cam );
	m_filterDetectionsWidget->setWindowTitle( "Detections" );
	m_filterDetectionsWidget->setFeatures( QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetVerticalTitleBar );
	m_filterDetectionsWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	QWidget * empty = new QWidget;
	empty->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

	QVBoxLayout * layout = new QVBoxLayout;
	layout->addWidget( m_miscFilter );
	layout->addWidget( m_filterDetectionsWidget );
	layout->addWidget( empty );

	this->setLayout( layout );
	setCurrentCamera( _cam );

}

void MainFilterDialog::setCurrentCamera( Camera2D * _cam )
{
	m_currentCamera = _cam;
	m_miscFilter->setCurrentCamera( m_currentCamera );
	m_filterDetectionsWidget->changeData( m_currentCamera->getDetectionSet(), m_currentCamera );
}
