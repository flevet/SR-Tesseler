/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      FilterObjectWidget.cpp
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

#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>

#include "ObjectInterface.hpp"
#include "FilterObjectWidget.hpp"
#include "WrapperVoronoiDiagram.hpp"
#include "Camera2D.hpp"
#include "DetectionSet.hpp"

FilterObjectWidget::FilterObjectWidget( QWidget* _parent /*= 0*/, Qt::WFlags _flags /*= 0 */ ): QDockWidget( _parent, _flags )
{

}

FilterObjectWidget::FilterObjectWidget( ObjectInterface * _data, Camera2D * _cam, QWidget* _parent /*= 0*/, Qt::WFlags _flags /*= 0 */ ) : QDockWidget( _parent, _flags )
{
	this->setObjectName( "FilterObjectWidget" );
	QWidget * widgetD = new QWidget;
	m_histoCam = NULL;
	if( _data != NULL ){
		QCheckBox * m_cboxDisplay = new QCheckBox;
		m_cboxDisplay->setText( "Display" );
		m_cboxDisplay->setChecked( _data->isSelected() );
		m_cboxDisplay->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		m_cboxLog = new QCheckBox;
		m_cboxLog->setText( "Log scale" );
		m_cboxLog->setChecked( _data->isLogHistogram() );
		m_cboxLog->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		if( m_histoCam == NULL ){
			m_histoCam = new HistogramCamera( _data, _cam );
			m_histoCam->setMinimumHeight( 150 );
		}
		else
			m_histoCam->changeDataSelected( _data, _cam );
		m_histoCam->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

		m_lutList = new QComboBox();
		QStringList list;
		list << "Gray" << "Red" << "Green" << "Blue" << "Fire" << "InvFire" << "Ice" << "AllBlue" << "AllGreen" << "AllWhite" << "AllBlack";
		m_lutList->addItems( list );

		m_combo = NULL;

		QPixmap pixmap("./images/save.png");
		m_buttonSave = new QPushButton;
		QIcon buttonIcon(pixmap);
		m_buttonSave->setIcon(buttonIcon);
		m_buttonSave->setIconSize(QSize(15, 15));
		m_buttonSave->setMaximumSize(QSize(18, 18));
		m_buttonSave->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

		int curCol = 0;
		QGridLayout * layout = new QGridLayout;
		layout->addWidget( m_histoCam, 0, 0, 1, 3 );
		//layout->addWidget(m_buttonSave, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxDisplay, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxLog, 1, curCol++, 1, 1);
		layout->addWidget(m_lutList, 1, curCol++, 1, 1);
		widgetD->setLayout( layout );

		QObject::connect( m_cboxDisplay, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( changeDataSelected( bool ) ) );
		QObject::connect( m_cboxLog, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( setLog( bool ) ) );
		QObject::connect( m_lutList, SIGNAL( currentIndexChanged( const QString & ) ), m_histoCam, SLOT( changeLut( const QString & ) ) );
		QObject::connect(m_buttonSave, SIGNAL(clicked()), this, SLOT(saveHistogramData()));
	}


	/****** End creation for the filter widget for detections  ******/

	this->setAllowedAreas( Qt::AllDockWidgetAreas );
	this->setWidget(widgetD);
	this->setMinimumSize( 200, 200 );
}

FilterObjectWidget::~FilterObjectWidget()
{
	delete lineMinDetection;
	delete lineMaxDetection;
}

void FilterObjectWidget::setDetectionBoundaries( const double _min, const double _max )
{
}

void FilterObjectWidget::updateHistograms( const bool resetHistograms )
{
	m_histoCam->regenerateDataSelected();
}

void FilterObjectWidget::isLogChecked( int _val )
{
}

void FilterObjectWidget::isLogChecked( bool _val )
{
	m_histoCam->setLog( _val );
}

void FilterObjectWidget::closeEvent( QCloseEvent * )
{
	this->hide();
	emit( visibleWidget( false ) );
}

QSize FilterObjectWidget::sizeHint() const
{
	return QSize( 500, 200 );
}

void FilterObjectWidget::setHistogramData( ObjectInterface * _data, Camera2D * _cam )
{
	if( _data != NULL ){
		QWidget * widgetD = new QWidget;
		QCheckBox * m_cboxDisplay = new QCheckBox;
		m_cboxDisplay->setText( "Display" );
		m_cboxDisplay->setChecked( _data->isSelected() );
		m_cboxDisplay->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		m_cboxLog = new QCheckBox;
		m_cboxLog->setText( "Log scale" );
		m_cboxLog->setChecked( _data->isLogHistogram() );
		m_cboxLog->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		if( m_histoCam == NULL ){
			m_histoCam = new HistogramCamera( _data, _cam );
			m_histoCam->setMinimumHeight( 150 );
		}
		else
			m_histoCam->changeDataSelected( _data, _cam );
		m_histoCam->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

		m_lutList = new QComboBox();
		QStringList list;
		list << "Gray" << "Red" << "Green" << "Blue" << "Fire" << "InvFire" << "Ice" << "AllBlue" << "AllGreen" << "AllWhite" << "AllBlack";
		m_lutList->addItems( list );

		m_combo = NULL;

		QPixmap pixmap("./images/save.png");
		m_buttonSave = new QPushButton;
		QIcon buttonIcon(pixmap);
		m_buttonSave->setIcon(buttonIcon);
		m_buttonSave->setIconSize(QSize(15, 15));
		m_buttonSave->setMaximumSize(QSize(18, 18));
		m_buttonSave->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

		int curCol = 0;
		QGridLayout * layout = new QGridLayout;
		layout->addWidget( m_histoCam, 0, 0, 1, 3 );
		//layout->addWidget(m_buttonSave, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxDisplay, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxLog, 1, curCol++, 1, 1);
		layout->addWidget(m_lutList, 1, curCol++, 1, 1);
		widgetD->setLayout(layout);
		this->setWidget(widgetD);

		QObject::connect( m_cboxDisplay, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( changeDataSelected( bool ) ) );
		QObject::connect( m_cboxLog, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( setLog( bool ) ) );
		QObject::connect( m_lutList, SIGNAL( currentIndexChanged( const QString & ) ), m_histoCam, SLOT( changeLut( const QString & ) ) );
		QObject::connect(m_buttonSave, SIGNAL(clicked()), this, SLOT(saveHistogramData()));
	}
}

void FilterObjectWidget::changeData( ObjectInterface * _data, Camera2D * _cam )
{
	if( _data != NULL ){
		m_cboxLog->setChecked( _data->isLogHistogram() );
		m_histoCam->changeDataSelected( _data, _cam );
		this->setVisible( true );
	}
	else
		this->setVisible( false );
}

void FilterObjectWidget::saveHistogramData()
{
	if (m_combo == NULL) return;
	m_histoCam->saveDataHistogram(m_combo->currentText(), m_combo->currentIndex());
}

FilterDetectionWidget::FilterDetectionWidget( ObjectInterface * _data, Camera2D * _cam, QWidget* _parent /*= 0*/, Qt::WFlags _flags /*= 0 */ ):FilterObjectWidget( _parent, _flags )
{
	this->setObjectName( "FilterDetectionWidget" );
	QWidget * widgetD = new QWidget;
	m_histoCam = NULL;
	if( _data != NULL ){
		SuperResObject * sobj = _cam->getSuperResObject();

		QCheckBox * m_cboxDisplay = new QCheckBox;
		m_cboxDisplay->setText( "Display" );
		m_cboxDisplay->setChecked( _data->isSelected() );
		m_cboxDisplay->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		m_cboxLog = new QCheckBox;
		m_cboxLog->setText( "Log scale" );
		m_cboxLog->setChecked( _data->isLogHistogram() );
		m_cboxLog->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		QLabel * nbLocsLbl = new QLabel( "# localisations : [" + QString::number( sobj->getDetectionSet()->nbPoints() ) + "]" );
		nbLocsLbl->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum );

		if( m_histoCam == NULL ){
			m_histoCam = new HistogramCamera( _data, _cam );
			m_histoCam->setMinimumHeight( 150 );
		}
		else
			m_histoCam->changeDataSelected( _data, _cam );
		m_histoCam->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

		m_lutList = new QComboBox();
		QStringList list;
		list << "Gray" << "Red" << "Green" << "Blue" << "Fire" << "InvFire" << "Ice" << "AllBlue" << "AllGreen" << "AllWhite" << "AllBlack";
		m_lutList->addItems( list );

		QStringList histogramTypes;
		histogramTypes << "Intensity";
		m_combo = new QComboBox;
		m_combo->addItems(histogramTypes);

		QPixmap pixmap("./images/save.png");
		m_buttonSave = new QPushButton;
		QIcon buttonIcon(pixmap);
		m_buttonSave->setIcon(buttonIcon);
		m_buttonSave->setIconSize(QSize(15, 15));
		m_buttonSave->setMaximumSize(QSize(18, 18));
		m_buttonSave->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

		int curCol = 0;
		QGridLayout * layout = new QGridLayout;
		layout->addWidget( m_histoCam, 0, 0, 1, 3 );
		//layout->addWidget(m_buttonSave, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxDisplay, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxLog, 1, curCol++, 1, 1);
		layout->addWidget(m_lutList, 1, curCol++, 1, 1);
		layout->addWidget(nbLocsLbl, 2, 0, 1, 1);
		widgetD->setLayout( layout );

		QObject::connect( m_cboxDisplay, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( changeDataSelected( bool ) ) );
		QObject::connect( m_cboxLog, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( setLog( bool ) ) );
		QObject::connect( m_lutList, SIGNAL( currentIndexChanged( const QString & ) ), m_histoCam, SLOT( changeLut( const QString & ) ) );
		QObject::connect(m_buttonSave, SIGNAL(clicked()), this, SLOT(saveHistogramData()));
	}


	/****** End creation for the filter widget for detections  ******/

	this->setAllowedAreas( Qt::AllDockWidgetAreas );
	this->setWidget(widgetD);
	this->setMinimumSize( 200, 200 );
}

FilterDetectionWidget::~FilterDetectionWidget()
{

}

void FilterDetectionWidget::setHistogramData( ObjectInterface * _data, Camera2D * _cam )
{
	if( _data != NULL ){
		QWidget * widgetD = new QWidget;
		SuperResObject * sobj = _cam->getSuperResObject();

		QCheckBox * m_cboxDisplay = new QCheckBox;
		m_cboxDisplay->setText( "Display" );
		m_cboxDisplay->setChecked( _data->isSelected() );
		m_cboxDisplay->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		m_cboxLog = new QCheckBox;
		m_cboxLog->setText( "Log scale" );
		m_cboxLog->setChecked( _data->isLogHistogram() );
		m_cboxLog->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		QLabel * nbLocsLbl = new QLabel( "# localisations : [" + QString::number( sobj->getDetectionSet()->nbPoints() ) + "]" );
		nbLocsLbl->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum );

		if( m_histoCam == NULL ){
			m_histoCam = new HistogramCamera( _data, _cam );
			m_histoCam->setMinimumHeight( 150 );
		}
		else
			m_histoCam->changeDataSelected( _data, _cam );
		m_histoCam->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

		m_lutList = new QComboBox();
		QStringList list;
		list << "Gray" << "Red" << "Green" << "Blue" << "Fire" << "InvFire" << "Ice" << "AllBlue" << "AllGreen" << "AllWhite" << "AllBlack";
		m_lutList->addItems( list );

		QStringList histogramTypes;
		histogramTypes << "Intensity";
		m_combo = new QComboBox;
		m_combo->addItems(histogramTypes);

		QPixmap pixmap("./images/save.png");
		m_buttonSave = new QPushButton;
		QIcon buttonIcon(pixmap);
		m_buttonSave->setIcon(buttonIcon);
		m_buttonSave->setIconSize(QSize(15, 15));
		m_buttonSave->setMaximumSize(QSize(18, 18));
		m_buttonSave->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

		int curCol = 0;
		QGridLayout * layout = new QGridLayout;
		layout->addWidget( m_histoCam, 0, 0, 1, 3 );
		//layout->addWidget(m_buttonSave, 1, 1, 1, 1);
		layout->addWidget(m_cboxDisplay, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxLog, 1, curCol++, 1, 1);
		layout->addWidget(m_lutList, 1, curCol++, 1, 1);
		layout->addWidget(nbLocsLbl, 2, 0, 1, 1);
		widgetD->setLayout( layout );
		this->setWidget(widgetD);

		QObject::connect( m_cboxDisplay, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( changeDataSelected( bool ) ) );
		QObject::connect( m_cboxLog, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( setLog( bool ) ) );
		QObject::connect( m_lutList, SIGNAL( currentIndexChanged( const QString & ) ), m_histoCam, SLOT( changeLut( const QString & ) ) );
		QObject::connect(m_buttonSave, SIGNAL(clicked()), this, SLOT(saveHistogramData()));
	}
}

FilterVoronoiDiagramWidget::FilterVoronoiDiagramWidget( ObjectInterface * _data, Camera2D * _cam, QWidget* _parent /*= 0*/, Qt::WFlags _flags /*= 0 */ ):FilterObjectWidget( _parent, _flags )
{
	this->setObjectName( "FilterVoronoiDiagramWidget" );
	QWidget * widgetD = new QWidget;
	m_histoCam = NULL;
	if( _data != NULL ){
		QCheckBox * m_cboxDisplay = new QCheckBox;
		m_cboxDisplay->setText( "Display" );
		m_cboxDisplay->setChecked( _data->isSelected() );
		m_cboxDisplay->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		m_cboxLog = new QCheckBox;
		m_cboxLog->setText( "Log scale" );
		m_cboxLog->setChecked( _data->isLogHistogram() );
		m_cboxLog->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		if( m_histoCam == NULL ){
			m_histoCam = new HistogramCamera( _data, _cam );
			m_histoCam->setMinimumHeight( 150 );
		}
		else
			m_histoCam->changeDataSelected( _data, _cam );
		m_histoCam->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
		QGridLayout * layout = new QGridLayout;
		QCheckBox * m_cboxOutline = NULL;
		QCheckBox * m_cboxFillPol = new QCheckBox;
		m_cboxFillPol->setText( "Fill polygon" );
		m_cboxFillPol->setChecked( false );
		m_cboxFillPol->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
		QStringList histogramTypes;
		histogramTypes << "LocalDensity" << "MeanDistance" << "Area";
		m_combo = new QComboBox;
		m_combo->addItems( histogramTypes );
		m_combo->setCurrentIndex( _data->whatTypeHistogram() );
		m_combo->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		m_lutList = new QComboBox();
		QStringList list;
		list << "Gray" << "Red" << "Green" << "Blue" << "Fire" << "InvFire" << "Ice" << "AllBlue" << "AllGreen" << "AllWhite" << "AllBlack" << "HotCold";
		m_lutList->addItems( list );

		QPixmap pixmap("./images/save.png");
		m_buttonSave = new QPushButton;
		QIcon buttonIcon(pixmap);
		m_buttonSave->setIcon(buttonIcon);
		m_buttonSave->setIconSize(QSize(15, 15));
		m_buttonSave->setMaximumSize(QSize(18, 18));
		m_buttonSave->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

		int curCol = 0;
		layout->addWidget( m_histoCam, 0, 0, 1, 6 );
		layout->addWidget(m_buttonSave, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxDisplay, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxLog, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxFillPol, 1, curCol++, 1, 1);
		layout->addWidget(m_combo, 1, curCol++, 1, 1);
		layout->addWidget(m_lutList, 1, curCol++, 1, 1);

		QObject::connect( m_cboxFillPol, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( setPolygonFilled( bool ) ) );
		QObject::connect( m_combo, SIGNAL( currentIndexChanged( int ) ), m_histoCam, SLOT( changeTypeHisto( int ) ) );
		QObject::connect( m_cboxDisplay, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( changeDataSelected( bool ) ) );
		QObject::connect( m_cboxLog, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( setLog( bool ) ) );
		QObject::connect( m_lutList, SIGNAL( currentIndexChanged( const QString & ) ), m_histoCam, SLOT( changeLut( const QString & ) ) );
		QObject::connect(m_buttonSave, SIGNAL(clicked()), this, SLOT(saveHistogramData()));

		widgetD->setLayout( layout );
	}


	/****** End creation for the filter widget for detections  ******/

	this->setAllowedAreas( Qt::AllDockWidgetAreas );
	this->setWidget(widgetD);
	this->setMinimumSize( 200, 200 );
}

FilterVoronoiDiagramWidget::~FilterVoronoiDiagramWidget()
{

}

void FilterVoronoiDiagramWidget::setHistogramData( ObjectInterface * _data, Camera2D * _cam )
{
	if( _data != NULL ){
		QWidget * widgetD = new QWidget;
		QCheckBox * m_cboxDisplay = new QCheckBox;
		m_cboxDisplay->setText( "Display" );
		m_cboxDisplay->setChecked( _data->isSelected() );
		m_cboxDisplay->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		m_cboxLog = new QCheckBox;
		m_cboxLog->setText( "Log scale" );
		m_cboxLog->setChecked( _data->isLogHistogram() );
		m_cboxLog->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		if( m_histoCam == NULL ){
			m_histoCam = new HistogramCamera( _data, _cam );
			m_histoCam->setMinimumHeight( 150 );
		}
		else
			m_histoCam->changeDataSelected( _data, _cam );
		m_histoCam->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
		QGridLayout * layout = new QGridLayout;
		QCheckBox * m_cboxOutline = NULL;
		QCheckBox * m_cboxFillPol = new QCheckBox;
		m_cboxFillPol->setText( "Fill polygon" );
		m_cboxFillPol->setChecked( false );
		m_cboxFillPol->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
		QStringList histogramTypes;
		histogramTypes << "LocalDensity" << "MeanDistance" << "Area";
		m_combo = new QComboBox;
		m_combo->addItems( histogramTypes );
		m_combo->setCurrentIndex( _data->whatTypeHistogram() );
		m_combo->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		m_lutList = new QComboBox();
		QStringList list;
		list << "Gray" << "Red" << "Green" << "Blue" << "Fire" << "InvFire" << "Ice" << "AllBlue" << "AllGreen" << "AllWhite" << "AllBlack" << "HotCold";
		m_lutList->addItems( list );

		QPixmap pixmap("./images/save.png");
		m_buttonSave = new QPushButton;
		QIcon buttonIcon(pixmap);
		m_buttonSave->setIcon(buttonIcon);
		m_buttonSave->setIconSize(QSize(15, 15));
		m_buttonSave->setMaximumSize(QSize(18, 18));
		m_buttonSave->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

		int curCol = 0;
		layout->addWidget( m_histoCam, 0, 0, 1, 6 );
		layout->addWidget(m_buttonSave, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxDisplay, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxLog, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxFillPol, 1, curCol++, 1, 1);
		layout->addWidget(m_combo, 1, curCol++, 1, 1);
		layout->addWidget(m_lutList, 1, curCol++, 1, 1);

		QObject::connect( m_cboxFillPol, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( setPolygonFilled( bool ) ) );
		QObject::connect( m_combo, SIGNAL( currentIndexChanged( int ) ), m_histoCam, SLOT( changeTypeHisto( int ) ) );
		QObject::connect( m_cboxDisplay, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( changeDataSelected( bool ) ) );
		QObject::connect( m_cboxLog, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( setLog( bool ) ) );
		QObject::connect( m_lutList, SIGNAL( currentIndexChanged( const QString & ) ), m_histoCam, SLOT( changeLut( const QString & ) ) );
		QObject::connect(m_buttonSave, SIGNAL(clicked()), this, SLOT(saveHistogramData()));

		widgetD->setLayout( layout );
		this->setWidget(widgetD);
	}
}

FilterVoronoiObjectWidget::FilterVoronoiObjectWidget( ObjectInterface * _data, Camera2D * _cam, QWidget* _parent /*= 0*/, Qt::WFlags _flags /*= 0 */ ):FilterObjectWidget( _parent, _flags )
{
	this->setObjectName( "FilterVoronoiObjectWidget" );
	QWidget * widgetD = new QWidget;
	m_histoCam = NULL;
	if( _data != NULL ){
		QCheckBox * m_cboxDisplay = new QCheckBox;
		m_cboxDisplay->setText( "Display" );
		m_cboxDisplay->setChecked( _data->isSelected() );
		m_cboxDisplay->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		m_cboxLog = new QCheckBox;
		m_cboxLog->setText( "Log scale" );
		m_cboxLog->setChecked( _data->isLogHistogram() );
		m_cboxLog->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		if( m_histoCam == NULL ){
			m_histoCam = new HistogramCamera( _data, _cam );
			m_histoCam->setMinimumHeight( 150 );
		}
		else
			m_histoCam->changeDataSelected( _data, _cam );
		m_histoCam->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
		QGridLayout * layout = new QGridLayout;
		QCheckBox * m_cboxOutline = NULL;
		m_cboxDisplay->setText( "Shape" );
		m_cboxOutline = new QCheckBox;
		m_cboxOutline->setText( "Outline" );
		m_cboxOutline->setChecked( true );
		m_cboxOutline->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
		QCheckBox * m_cboxFillPol = new QCheckBox;
		m_cboxFillPol->setText( "Fill poly" );
		m_cboxFillPol->setChecked( true );
		m_cboxFillPol->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
		QStringList histogramTypes;
		histogramTypes << "LocalDensity" << "MeanDistance" << "Area";
		m_combo = new QComboBox;
		m_combo->addItems( histogramTypes );
		m_combo->setCurrentIndex( _data->whatTypeHistogram() );
		m_combo->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		m_lutList = new QComboBox();
		QStringList list;
		list << "Gray" << "Red" << "Green" << "Blue" << "Fire" << "InvFire" << "Ice" << "AllBlue" << "AllGreen" << "AllWhite" << "AllBlack";
		m_lutList->addItems( list );

		QPixmap pixmap("./images/save.png");
		m_buttonSave = new QPushButton;
		QIcon buttonIcon(pixmap);
		m_buttonSave->setIcon(buttonIcon);
		m_buttonSave->setIconSize(QSize(15, 15));
		m_buttonSave->setMaximumSize(QSize(18, 18));
		m_buttonSave->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

		int curCol = 0;
		layout->addWidget( m_histoCam, 0, 0, 1, 6 );
		//layout->addWidget(m_buttonSave, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxDisplay, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxOutline, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxLog, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxFillPol, 1, curCol++, 1, 1);
		layout->addWidget(m_combo, 1, curCol++, 1, 1);
		layout->addWidget(m_lutList, 1, curCol++, 1, 1);
		
		QObject::connect( m_cboxFillPol, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( setPolygonFilled( bool ) ) );
		QObject::connect( m_combo, SIGNAL( currentIndexChanged( int ) ), m_histoCam, SLOT( changeTypeHisto( int ) ) );
		QObject::connect( m_cboxDisplay, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( changeDataSelected( bool ) ) );
		QObject::connect( m_cboxLog, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( setLog( bool ) ) );
		QObject::connect( m_cboxOutline, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( setOutlineDisplay( bool ) ) );
		QObject::connect( m_lutList, SIGNAL( currentIndexChanged( const QString & ) ), m_histoCam, SLOT( changeLut( const QString & ) ) );
		QObject::connect(m_buttonSave, SIGNAL(clicked()), this, SLOT(saveHistogramData()));

		widgetD->setLayout( layout );
	}


	/****** End creation for the filter widget for detections  ******/

	this->setAllowedAreas( Qt::AllDockWidgetAreas );
	this->setWidget(widgetD);
	this->setMinimumSize( 200, 200 );
}

FilterVoronoiObjectWidget::~FilterVoronoiObjectWidget()
{

}

void FilterVoronoiObjectWidget::setHistogramData( ObjectInterface * _data, Camera2D * _cam )
{
	if( _data != NULL ){
		QWidget * widgetD = new QWidget;
		QCheckBox * m_cboxDisplay = new QCheckBox;
		m_cboxDisplay->setText( "Display" );
		m_cboxDisplay->setChecked( _data->isSelected() );
		m_cboxDisplay->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		m_cboxLog = new QCheckBox;
		m_cboxLog->setText( "Log scale" );
		m_cboxLog->setChecked( _data->isLogHistogram() );
		m_cboxLog->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		if( m_histoCam == NULL ){
			m_histoCam = new HistogramCamera( _data, _cam );
			m_histoCam->setMinimumHeight( 150 );
		}
		else
			m_histoCam->changeDataSelected( _data, _cam );
		m_histoCam->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
		QGridLayout * layout = new QGridLayout;
		QCheckBox * m_cboxOutline = NULL;
		m_cboxDisplay->setText( "Shape" );
		m_cboxOutline = new QCheckBox;
		m_cboxOutline->setText( "Outline" );
		m_cboxOutline->setChecked( true );
		m_cboxOutline->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
		QCheckBox * m_cboxFillPol = new QCheckBox;
		m_cboxFillPol->setText( "Fill poly" );
		m_cboxFillPol->setChecked( true );
		m_cboxFillPol->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
		QStringList histogramTypes;
		histogramTypes << "LocalDensity" << "MeanDistance" << "Area";
		m_combo = new QComboBox;
		m_combo->addItems( histogramTypes );
		m_combo->setCurrentIndex( _data->whatTypeHistogram() );
		m_combo->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

		m_lutList = new QComboBox();
		QStringList list;
		list << "Gray" << "Red" << "Green" << "Blue" << "Fire" << "InvFire" << "Ice" << "AllBlue" << "AllGreen" << "AllWhite" << "AllBlack";
		m_lutList->addItems( list );

		QPixmap pixmap("./images/save.png");
		m_buttonSave = new QPushButton;
		QIcon buttonIcon(pixmap);
		m_buttonSave->setIcon(buttonIcon);
		m_buttonSave->setIconSize(QSize(15, 15));
		m_buttonSave->setMaximumSize(QSize(18, 18));
		m_buttonSave->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

		int curCol = 0;
		layout->addWidget( m_histoCam, 0, 0, 1, 6 );
		//layout->addWidget(m_buttonSave, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxDisplay, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxOutline, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxLog, 1, curCol++, 1, 1);
		layout->addWidget(m_cboxFillPol, 1, curCol++, 1, 1);
		layout->addWidget(m_combo, 1, curCol++, 1, 1);
		layout->addWidget(m_lutList, 1, curCol++, 1, 1);

		QObject::connect( m_cboxFillPol, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( setPolygonFilled( bool ) ) );
		QObject::connect( m_combo, SIGNAL( currentIndexChanged( int ) ), m_histoCam, SLOT( changeTypeHisto( int ) ) );
		QObject::connect( m_cboxDisplay, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( changeDataSelected( bool ) ) );
		QObject::connect( m_cboxLog, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( setLog( bool ) ) );
		QObject::connect( m_cboxOutline, SIGNAL( clicked( bool ) ), m_histoCam, SLOT( setOutlineDisplay( bool ) ) );

		QObject::connect( m_lutList, SIGNAL( currentIndexChanged( const QString & ) ), m_histoCam, SLOT( changeLut( const QString & ) ) );
		QObject::connect(m_buttonSave, SIGNAL(clicked()), this, SLOT(saveHistogramData()));

		widgetD->setLayout( layout );
		this->setWidget(widgetD);
	}
}