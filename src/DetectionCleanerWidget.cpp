/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      DetectionCleanerWidget.cpp
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
#include <QBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QButtonGroup>
#include <QTextDocument>
#include <QScrollArea>
#include <fstream>

#include "DetectionCleanerWidget.hpp"
#include "Camera2D.hpp"
#include "DetectionSet.hpp"
#include "DetectionCleaner.hpp"

DetectionCleanerWidget::DetectionCleanerWidget( Camera2D * _cam, QWidget* _parent /*= 0*/ ):QTabWidget( _parent )
{
	QWidget * interactionWidget = new QWidget;
	QWidget * statsWidget = new QWidget;

	QRegExp rx("[0-9.]+");
	QValidator * validator = new QRegExpValidator(rx, this);

	QGroupBox * groupProcess = new QGroupBox( QObject::tr( "Cleaning process" ) );
	groupProcess->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
	m_rbuttonFixedN = new QRadioButton( "Fixed size" );
	m_rbuttonFixedN->setChecked( true );
	m_rbuttonFixedN->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_rbuttonPhotonN = new QRadioButton( "Photon size" );
	m_rbuttonPhotonN->setChecked( false );
	m_rbuttonPhotonN->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_rbuttonPhotonBackGN = new QRadioButton( "Photon/background size" );
	m_rbuttonPhotonBackGN->setChecked( false );
	m_rbuttonPhotonBackGN->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_buttonProcess = new QPushButton( "Cleaning process" );
	m_buttonProcess->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_lblFixed = new QLabel( "Size:" );
	m_lblFixed->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_leditFixedNeigh = new QLineEdit( "0.3" );
	m_leditFixedNeigh->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_leditFixedNeigh->setValidator(validator);
	m_lblPixel = new QLabel("Pixel value:");
	m_lblPixel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_lblPixel->setEnabled( false );
	m_leditPixelSize = new QLineEdit( "0.1" );
	m_leditPixelSize->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_leditPixelSize->setEnabled( false );
	m_leditPixelSize->setValidator(validator);
	m_lblBack = new QLabel("Background value:");
	m_lblBack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_lblBack->setEnabled( false );
	m_leditBackground = new QLineEdit( "0.3" );
	m_leditBackground->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_leditBackground->setEnabled( false );
	m_leditBackground->setValidator(validator);
	m_lblInt2Photon = new QLabel("Intensity to photon ratio:");
	m_lblInt2Photon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_lblInt2Photon->setEnabled(false);
	m_lEditInt2Photon = new QLineEdit("0.039");
	m_lEditInt2Photon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_lEditInt2Photon->setEnabled(false);
	m_lEditInt2Photon->setValidator(validator);
	m_cboxFixedMaxDarkTime = new QCheckBox("Fixed max dark time:");
	m_cboxFixedMaxDarkTime->setChecked(false);
	m_cboxFixedMaxDarkTime->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_leditMaxDarkTime = new QLineEdit("20");
	m_leditMaxDarkTime->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_leditMaxDarkTime->setValidator(validator);
	m_buttonExport = new QPushButton("Export stats");
	m_buttonExport->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_buttonExport->setEnabled(false);
	m_bgroup = new QButtonGroup;
	m_bgroup->addButton( m_rbuttonFixedN );
	m_bgroup->addButton( m_rbuttonPhotonN );
	m_bgroup->addButton( m_rbuttonPhotonBackGN );
	QGridLayout * layoutProcess = new QGridLayout;
	int columnCount = 0;
	layoutProcess->addWidget( m_rbuttonFixedN, 0, columnCount, 1, 1 );
	layoutProcess->addWidget( m_lblFixed, 1, columnCount, 1, 1 );
	layoutProcess->addWidget( m_lblPixel, 2, columnCount, 1, 1 );
	layoutProcess->addWidget(m_cboxFixedMaxDarkTime, 3, columnCount++, 1, 1);
	layoutProcess->addWidget(m_rbuttonPhotonN, 0, columnCount, 1, 1);
	layoutProcess->addWidget( m_leditFixedNeigh, 1, columnCount, 1, 1 );
	layoutProcess->addWidget( m_leditPixelSize, 2, columnCount, 1, 1 );
	layoutProcess->addWidget(m_leditMaxDarkTime, 3, columnCount++, 1, 1);
	layoutProcess->addWidget(m_rbuttonPhotonBackGN, 0, columnCount, 1, 1);
	layoutProcess->addWidget(m_lblInt2Photon, 1, columnCount, 1, 1);
	layoutProcess->addWidget(m_lblBack, 2, columnCount++, 1, 1);
	layoutProcess->addWidget( m_buttonProcess, 0, columnCount, 1, 1 );
	layoutProcess->addWidget(m_lEditInt2Photon, 1, columnCount, 1, 1);
	layoutProcess->addWidget(m_leditBackground, 2, columnCount, 1, 1);
	layoutProcess->addWidget(m_buttonExport, 3, columnCount++, 1, 1);
	groupProcess->setLayout(layoutProcess);


	m_filterDetectionsWidget = new FilterObjectWidget( NULL, _cam );
	m_filterDetectionsWidget->setWindowTitle( "Detections" );
	m_filterDetectionsWidget->setFeatures( QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetVerticalTitleBar );
	m_filterDetectionsWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

	QWidget * empty = new QWidget;
	empty->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

	QVBoxLayout * layoutInteraction = new QVBoxLayout;
	layoutInteraction->addWidget( groupProcess );
	layoutInteraction->addWidget( m_filterDetectionsWidget );
	layoutInteraction->addWidget( empty );
	interactionWidget->setLayout( layoutInteraction );

	m_dcToffsViewer = new DetectionCleanerGLViewer( "Toff" );
	m_dcToffsViewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_dcTOnsViewer = new DetectionCleanerGLViewer( "Ton" );
	m_dcTOnsViewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_dcBlinksViewer = new DetectionCleanerGLViewer( "# blinks" );
	m_dcBlinksViewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QGridLayout * layoutInfos = new QGridLayout;
	layoutInfos->addWidget( m_dcToffsViewer, 0, 0, 1, 1 );
	layoutInfos->addWidget( m_dcTOnsViewer, 1, 0, 1, 1 );
	layoutInfos->addWidget( m_dcBlinksViewer, 2, 0, 1, 1 );
	QWidget * tmp = new QWidget;
	tmp->setLayout( layoutInfos );

	m_statsTEdit = new QPlainTextEdit;
	m_statsTEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_statsTEdit->setReadOnly(true);
	m_statsTEdit->setTextInteractionFlags(m_statsTEdit->textInteractionFlags() | Qt::TextSelectableByKeyboard);

	QVBoxLayout * layoutStats = new QVBoxLayout;
	layoutStats->addWidget( tmp );
	layoutStats->addWidget(m_statsTEdit);
	statsWidget->setLayout( layoutStats );

	this->addTab( interactionWidget, tr( "Cleaner infos" ) );
	this->addTab( statsWidget, tr( "Cleaner stats" ) );

	QObject::connect( m_bgroup, SIGNAL( buttonClicked( QAbstractButton * ) ), this, SLOT( changeButton( QAbstractButton * ) ) );
	QObject::connect(m_buttonExport, SIGNAL(clicked()), this, SLOT(exportStats()));
	setCurrentCamera(_cam);

	if (_cam->getSuperResObject()->getDetectionSet()->hasSigmaPerLocalization()){
		m_rbuttonPhotonN->setEnabled(true);
		m_rbuttonPhotonBackGN->setEnabled(true);
	}
	else{
		m_rbuttonPhotonN->setEnabled(false);
		m_rbuttonPhotonBackGN->setEnabled(false);
	}
}

DetectionCleanerWidget::~DetectionCleanerWidget()
{

}

double DetectionCleanerWidget::getSizeFixedNeighborhood() const
{
	bool ok = true;
	double valLine = m_leditFixedNeigh->text().toDouble(&ok), val;
	val = ( ok ) ? valLine : .3;
	return val;
}

double DetectionCleanerWidget::getPixelSize() const
{
	bool ok = true;
	double valLine = m_leditPixelSize->text().toDouble( &ok ), val;
	val = ( ok ) ? valLine : .3;
	return val;
}

double DetectionCleanerWidget::getBackgroundValue() const
{
	bool ok = true;
	double valLine = m_leditBackground->text().toDouble( &ok ), val;
	val = ( ok ) ? valLine : .3;
	return val;
}

double DetectionCleanerWidget::getInt2PhotonRatio() const
{
	bool ok = true;
	double valLine = m_lEditInt2Photon->text().toDouble(&ok), val;
	val = (ok) ? valLine : .039;
	return val;
}

int DetectionCleanerWidget::getMaxDarkTime() const
{
	bool ok = true;
	double valLine = m_leditMaxDarkTime->text().toInt(&ok), val;
	val = (ok) ? valLine : 20;
	return val;
}

void DetectionCleanerWidget::setCurrentCamera( Camera2D * _camera )
{
	m_currentCamera = _camera;
	setDetectionCleaner( _camera->getDetectionCleaner() );
}

void DetectionCleanerWidget::setDetectionCleaner( DetectionCleaner * _cleaner )
{
	if( _cleaner == NULL ){
		m_dcBlinksViewer->setVisible( false );
		m_dcToffsViewer->setVisible( false );
		m_dcTOnsViewer->setVisible( false );
		m_statsTEdit->setVisible(false);
		m_filterDetectionsWidget->setVisible( false );
	}
	else{
		m_dcBlinksViewer->setEquation( _cleaner->getEquationBlinks() );
		m_dcToffsViewer->setEquation( _cleaner->getEquationTOffs() );
		m_dcTOnsViewer->setEquation( _cleaner->getEquationTOns() );
		m_statsTEdit->clear();
		m_statsTEdit->appendPlainText(_cleaner->getStats());
		m_filterDetectionsWidget->setHistogramData(m_currentCamera->getDetectionSetCleaned(), m_currentCamera);
		m_dcBlinksViewer->setVisible( true );
		m_dcToffsViewer->setVisible( true );
		m_dcTOnsViewer->setVisible( true );
		m_statsTEdit->setVisible(true);
		m_filterDetectionsWidget->setVisible(true);
	}
	this->setEnableExport(_cleaner != NULL);
}

void DetectionCleanerWidget::createDetectionCleaner()
{
	m_currentCamera->cleanDetections();
	this->setDetectionCleaner( m_currentCamera->getDetectionCleaner() );
	m_currentCamera->updateGL();
}

void DetectionCleanerWidget::changeButton( QAbstractButton * _button )
{
	if( _button == m_rbuttonFixedN ){
		m_lblFixed->setEnabled( true );
		m_leditFixedNeigh->setEnabled( true );
		m_lblPixel->setEnabled( false );
		m_leditPixelSize->setEnabled( false );
		m_lblBack->setEnabled( false );
		m_leditBackground->setEnabled( false );
		m_lblInt2Photon->setEnabled(false);
		m_lEditInt2Photon->setEnabled(false);
	}
	else if( _button == m_rbuttonPhotonN ){
		m_lblFixed->setEnabled( false );
		m_leditFixedNeigh->setEnabled( false );
		m_lblPixel->setEnabled( false );
		m_leditPixelSize->setEnabled( false );
		m_lblBack->setEnabled( false );
		m_leditBackground->setEnabled( false );
		m_lblInt2Photon->setEnabled(true);
		m_lEditInt2Photon->setEnabled(true);
	}
	else if( _button == m_rbuttonPhotonBackGN ){
		m_lblFixed->setEnabled( false );
		m_leditFixedNeigh->setEnabled( false );
		m_lblPixel->setEnabled( true );
		m_leditPixelSize->setEnabled( true );
		m_lblBack->setEnabled( true );
		m_leditBackground->setEnabled( true );
		m_lblInt2Photon->setEnabled(true);
		m_lEditInt2Photon->setEnabled(true);
	}
}

unsigned char DetectionCleanerWidget::getOptions() const
{
	unsigned char options = 0;
	if( m_rbuttonFixedN->isChecked() ) options = options | DetectionCleaner::FixedDistanceFlag;
	if( m_rbuttonPhotonN->isChecked() ) options = options | DetectionCleaner::PhotonDistanceFlag;
	if( m_rbuttonPhotonBackGN->isChecked() ) options = options | DetectionCleaner::PhotonBackgroundDistanceFlag;
	if (m_cboxFixedMaxDarkTime->isChecked()) options = options | DetectionCleaner::FixedMaxDarkTimeFlag;
	return options;
}

void DetectionCleanerWidget::exportStats()
{
	DetectionCleaner * cleaner = m_currentCamera->getSuperResObject()->getDetectionCleaner();
	QString dir = m_currentCamera->getSuperResObject()->getDir().c_str();
	if (!dir.endsWith("/")) dir.append("/");
	QString nameTOffs(dir), nameTOns(dir), nameBlinks(dir);
	nameTOffs.append("toffs.txt");
	nameTOns.append("tons.txt");
	nameBlinks.append("blinks.txt");

	EquationFit * eqnTons = cleaner->getEquationTOns(), *eqnToffs = cleaner->getEquationTOffs(), * eqnBlinks = cleaner->getEquationBlinks();
	std::ofstream fs(nameTOffs.toAscii().data());
	fs << "TOffs(# of frames)\t# of molecules" << std::endl;
	double * values = eqnToffs->getValues(), * ts = eqnToffs->getTs();
	for (int n = 0; n < eqnToffs->getNbTs(); n++)
		fs << ts[n] << "\t" << values[n] << std::endl;
	fs.close();
	fs.clear();

	fs.open(nameTOns.toAscii().data());
	fs << "TOns(# of frames)\t# of molecules" << std::endl;
	values = eqnTons->getValues();
	ts = eqnTons->getTs();
	for (int n = 0; n < eqnTons->getNbTs(); n++)
		fs << ts[n] << "\t" << values[n] << std::endl;
	fs.close();
	fs.clear();

	fs.open(nameBlinks.toAscii().data());
	fs << "# of blinks\t# of molecules" << std::endl;
	values = eqnBlinks->getValues();
	ts = eqnBlinks->getTs();
	for (int n = 0; n < eqnBlinks->getNbTs(); n++)
		fs << ts[n] << "\t" << values[n] << std::endl;
	fs.close();
	fs.clear();
}
