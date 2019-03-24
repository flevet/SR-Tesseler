/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      MiscFilterWidget.cpp
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

#include "MiscFilterWidget.hpp"
#include "Camera2D.hpp"
#include "gl2ps.h"

MiscFilterWidget::MiscFilterWidget( Camera2D * _cam, QWidget* _parent /*= 0*/, Qt::WFlags _flags /*= 0 */ ):QDockWidget( _parent, _flags )
{
	this->setObjectName( "MiscFilterWidget" );
	m_currentCamera = _cam;

	QWidget * widget = new QWidget;
	m_smoothPointCB = new QCheckBox( "Smooth point" );
	m_smoothPointCB->setChecked( false );
	m_smoothPointCB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_sizePointLbl = new QLabel( "Size point [1-8]:" );
	m_sizePointLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_sizePointSpn = new QSpinBox;
	m_sizePointSpn->setRange( 1, 8 );
	m_sizePointSpn->setValue( 1 );
	m_sizePointSpn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_smoothLineCB = new QCheckBox( "Smooth line" );
	m_smoothLineCB->setChecked( false );
	m_smoothLineCB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_widthLineLbl = new QLabel( "Line width [1-8]:" );
	m_widthLineLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_widthLineSpn = new QSpinBox;
	m_widthLineSpn->setRange( 1, 8 );
	m_widthLineSpn->setValue( 1 );
	m_widthLineSpn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	QLabel * backColorLbl = new QLabel( "Background color:" );
	backColorLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_colorBackBtn = new QPushButton();
	m_colorBackBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_colorBackBtn->setStyleSheet( "background-color: rgb(0, 0, 0);"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 5px;"
		"border-color: black;"
		"font: 12px;"
		"min-width: 5em;"
		"padding: 3px;"
		);
	QPushButton * savePositionBtn = new QPushButton( "Save position" );
	savePositionBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	QPushButton * loadPositionBtn = new QPushButton("Load position");
	loadPositionBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	QPushButton * snapViewerBtn = new QPushButton("Snap viewer");
	snapViewerBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

	m_strokeWidthLbl = new QLabel("Stroke width:");
	m_strokeWidthLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_strokeWidthLEdit = new QLineEdit("0.5");
	m_strokeWidthLEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	QHBoxLayout * layoutStrokeW = new QHBoxLayout;
	layoutStrokeW->addWidget(m_strokeWidthLbl);
	layoutStrokeW->addWidget(m_strokeWidthLEdit);
	QWidget * wisgetStrokeW = new QWidget;
	wisgetStrokeW->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	wisgetStrokeW->setLayout(layoutStrokeW);
	QPushButton * exportVectorBtn = new QPushButton("Vectorial snap");
	exportVectorBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

	QGridLayout * layoutMisc = new QGridLayout;
	int columnCount = 0;
	layoutMisc->addWidget( m_smoothPointCB, 0, columnCount++, 1, 1 );
	layoutMisc->addWidget( m_sizePointLbl, 0, columnCount++, 1, 1 );
	layoutMisc->addWidget( m_sizePointSpn, 0, columnCount++, 1, 1 );
	layoutMisc->addWidget(savePositionBtn, 0, columnCount++, 1, 1);
	columnCount = 0;
	layoutMisc->addWidget( m_smoothLineCB, 1, columnCount++, 1, 1 );
	layoutMisc->addWidget( m_widthLineLbl, 1, columnCount++, 1, 1 );
	layoutMisc->addWidget( m_widthLineSpn, 1, columnCount++, 1, 1 );
	layoutMisc->addWidget(loadPositionBtn, 1, columnCount++, 1, 1);
	columnCount = 0;
	layoutMisc->addWidget( backColorLbl, 2, columnCount++, 1, 1 );
	layoutMisc->addWidget( m_colorBackBtn, 2, columnCount++, 1, 1 );
	layoutMisc->addWidget(snapViewerBtn, 2, 3, 1, 1);
	columnCount = 0;
	layoutMisc->addWidget(wisgetStrokeW, 3, 2, 1, 1);
	layoutMisc->addWidget(exportVectorBtn, 3, 3, 1, 1);

	widget->setLayout( layoutMisc );
	this->setWidget( widget );

	QObject::connect( m_sizePointSpn, SIGNAL( valueChanged ( int ) ), this, SLOT( setSizePoint( int ) ) );
	QObject::connect( m_smoothPointCB, SIGNAL( toggled( bool ) ), this, SLOT( setPointSmooth( bool ) ) );
	QObject::connect( m_widthLineSpn, SIGNAL( valueChanged ( int ) ), this, SLOT( setLineWidth( int ) ) );
	QObject::connect( m_smoothLineCB, SIGNAL( toggled( bool ) ), this, SLOT( setLineSmooth( bool ) ) );
	QObject::connect( m_colorBackBtn, SIGNAL( pressed() ), this, SLOT( changeBackgroundColor() ) );

	QObject::connect(savePositionBtn, SIGNAL(pressed()), this, SLOT(savePosition()));
	QObject::connect(loadPositionBtn, SIGNAL(pressed()), this, SLOT(loadPosition()));
	QObject::connect(snapViewerBtn, SIGNAL(pressed()), this, SLOT(snapViewer()));
	QObject::connect(exportVectorBtn, SIGNAL(pressed()), this, SLOT(exportInVectorialFile()));
}

MiscFilterWidget::~MiscFilterWidget()
{
	delete m_smoothPointCB;
	delete m_sizePointLbl;
	delete m_sizePointSpn;
}

void MiscFilterWidget::setSizePoint( int _val )
{
	m_currentCamera->setSizePoint( _val );
	m_currentCamera->updateGL();
}

void MiscFilterWidget::setPointSmooth( bool _val )
{
	m_currentCamera->setSmoothPoint( _val );
	m_currentCamera->updateGL();
}

void MiscFilterWidget::setLineWidth( int _val )
{
	m_currentCamera->setLineWidth( _val );
	m_currentCamera->updateGL();
}

void MiscFilterWidget::setLineSmooth( bool _val )
{
	m_currentCamera->setLineSmooth( _val );
	m_currentCamera->updateGL();
}

void MiscFilterWidget::changeBackgroundColor()
{
	m_currentCamera->changeBackgroundColor();
	const Color4B color = m_currentCamera->getBackgroundColor();
	m_colorBackBtn->setStyleSheet( "background-color: rgb(" + QString::number( ( int )color[0] ) + ", " + QString::number( ( int )color[1] ) + ", " + QString::number( ( int )color[2] ) + ");"
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

void MiscFilterWidget::savePosition()
{
	double values[5];
	m_currentCamera->getPositionZoomParameters(values);

	QString filename = QFileDialog::getSaveFileName(this, QObject::tr("Save position..."), m_currentCamera->getSuperResObject()->getDir().c_str(), QObject::tr("Position files (*.txt)"), 0, QFileDialog::DontUseNativeDialog);

	if (filename.isEmpty()) return;

	std::ofstream fs(filename.toAscii().data());
	fs << "[sr-tesseler position_file]" << std::endl;
	for (unsigned int n = 0; n < 5; n++){
		fs << values[n];
		if (n < 4)
			fs << " ";
	}
	fs << std::endl;
	fs.close();
}

void MiscFilterWidget::loadPosition()
{
	const QString filename = QFileDialog::getOpenFileName(this,
		tr("Open File"),
		m_currentCamera->getSuperResObject()->getDir().c_str(),
		tr("Position Files (*.txt)"), 0, QFileDialog::DontUseNativeDialog);

	if (filename.isEmpty()) return;

	std::ifstream fs(filename.toAscii().data());
	if (!fs.is_open()) std::cout << "Problem opening position file" << std::endl;
	std::stringstream buffer;
	buffer << fs.rdbuf();

	QString fileText(buffer.str().c_str());
	QStringList lines = fileText.split("\n");

	if (lines.at(0) != "[sr-tesseler position_file]"){
		std::cout << "Trying to open an uncorrect position file" << std::endl;
		fs.close();
		return;
	}

	QStringList datas = lines.at(1).split(" ");
	if (datas.size() != 5){
		std::cout << "Trying to open an uncorrect position file" << std::endl;
		fs.close();
		return;
	}

	bool ok;
	double values[5];// widthProjection=values[0], heightProjection=values[1], xv=values[2], yv=values[3], zoomFactor=values[4];
	for (unsigned int n = 0; n < 5; n++)
		values[n] = datas.at(n).toDouble(&ok);

	m_currentCamera->setPositionZoomParameters(values);

	fs.close();
}

void MiscFilterWidget::snapViewer()
{
	QString filename = QFileDialog::getSaveFileName(this, QObject::tr("Save viewer..."), m_currentCamera->getSuperResObject()->getDir().c_str(), QObject::tr("Image files (*.jpg *.tif *.png)"), 0, QFileDialog::DontUseNativeDialog);
	if (filename.isEmpty()) return;
	int index = filename.lastIndexOf(".");
	if (index == -1) 
		filename.append(".png");
	else{
		QString extension = filename.mid(index);
		if (extension != ".jpg" && extension != ".tif" && extension != ".png")
			filename.replace(extension, ".png");
	}
	m_currentCamera->snap(filename);
}

void MiscFilterWidget::exportInVectorialFile()
{
	QString filename = QFileDialog::getSaveFileName(this, QObject::tr("Save viewer in vectorial form..."), m_currentCamera->getSuperResObject()->getDir().c_str(), QObject::tr("SVG files (*.svg)"), 0, QFileDialog::DontUseNativeDialog);
	if (filename.isEmpty()) return;
	int index = filename.lastIndexOf(".");
	if (index == -1)
		filename.append(".svg");
	else{
		QString extension = filename.mid(index);
		if (extension != ".svg")
			filename.replace(extension, ".svg");
	}

	bool ok = false;
	double tmp = m_strokeWidthLEdit->text().toDouble(&ok), strokeW = ok ? tmp : 0.5;
	setGL2PSStrokeWidth(strokeW);
	m_currentCamera->exportInVectorialFile(filename);
}