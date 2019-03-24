/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      ImageViewer.cpp
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

#include <QApplication>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QTime>
#include <QMessageBox>
#include "ImageViewer.hpp"
#include "GeneralTools.hpp"
#include "SuperResObject.hpp"
#include "DetectionSet.hpp"
#ifdef PALM_TRACER
#include "FilesDeterminator.hpp"
#endif


ImageViewer::ImageViewer() : m_openSelectionMenu(NULL), m_openDirAct(NULL), m_openLocFileAct(NULL)
{
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	this->setWindowIcon( QIcon("./images/voronIcon1.PNG") );

	m_hbar = new QScrollBar( Qt::Horizontal );
	m_hbar->setMinimum( 0 );
	m_hbar->setMaximum( 50 );
	m_vbar = new QScrollBar( Qt::Vertical );
	m_vbar->setMinimum( 0 );
	m_vbar->setMaximum( 50 );

	m_camera = new Camera2D( m_hbar, m_vbar );
	m_camera->setFocusPolicy(Qt::StrongFocus);
	m_camera->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	createActions();

	m_editToolBar = new QToolBar(tr("Edit"));
	m_editToolBar->addAction( m_openAct );
	m_editToolBar->addAction(moveAct);
	m_editToolBar->addAction(zoomAct);
	m_editToolBar->addAction(m_gridAct);
	m_editToolBar->addAction( m_roisAct );
	m_editToolBar->addAction( m_aboutAct );
	m_editToolBar->setContentsMargins(0, 0, 0, 0);

	m_hbar->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
	m_hbar->hide();
	m_vbar->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
	m_vbar->hide();

	m_centralLayout = new QGridLayout;
	m_centralLayout->setMargin(10);
	m_centralLayout->addWidget(m_editToolBar, 0, 0, 1, 2);
	m_centralLayout->addWidget(m_camera, 1, 0, 1, 1);
	m_centralLayout->addWidget(m_vbar, 1, 1, 2, 1);
	m_centralLayout->addWidget(m_hbar, 2, 0, 1, 1);

	m_statusBar = new QWidget;
	QHBoxLayout * hlayout = new QHBoxLayout;
	m_labelStatusPosition = new QLabel("Ready");
	hlayout->addWidget(m_labelStatusPosition);
	m_labelStatusZoom = new QLabel("");
	hlayout->addWidget(m_labelStatusZoom);
	m_progress = new QProgressBar;
	hlayout->addWidget( m_progress );
	m_statusBar->setLayout( hlayout );

	m_centralLayout->addWidget(m_statusBar, 4, 0, 1, 1);

	this->setLayout(m_centralLayout);

	setWindowTitle(tr("SR-Tesseler: Viewer"));
	resize(1000, 800);
	show();

	createConnections();
	m_initialized = false;

	GeneralTools::m_imw = this;
}

void ImageViewer::createActions()
{
	m_openAct = new QAction(QIcon("./images/open.png"), tr("&Open"), this);
	m_openAct->setStatusTip(tr("Open"));
	m_openAct->setEnabled( true );
	zoomAct = new QAction(QIcon("./images/zoom.png"), tr("Zoom"), this);
	zoomAct->setStatusTip(tr("Zoom"));
	zoomAct->setEnabled(false);
	moveAct = new QAction(QIcon("./images/hand.png"), tr("Move image"), this);
	moveAct->setStatusTip(tr("Move image"));
	moveAct->setEnabled(false);
	m_gridAct = new QAction(QIcon("./images/grid.png"), tr("&Toggle Grid"), this);
	m_gridAct->setStatusTip(tr("Toggle Grid"));
	m_gridAct->setEnabled(false);
	m_roisAct = new QAction( QIcon( "./images/roi.png" ), tr( "Define roi" ), this );
	m_roisAct->setStatusTip( tr( "Define roi" ) );
	m_aboutAct = new QAction( QIcon( "./images/about.png" ), tr( "About..." ), this );
	m_aboutAct->setStatusTip( tr( "About..." ) );
}

void ImageViewer::createConnections()
{
#ifdef PALM_TRACER
		QObject::connect(m_openAct, SIGNAL(triggered()), this, SLOT(displayOrHideMenu()));
		m_openDirAct = new QAction(tr("Open PalmTracer"), this);
		m_openDirAct->setEnabled(true);
		QObject::connect(m_openDirAct, SIGNAL(triggered()), this, SLOT(openPalmTracerDataset()));
		m_openLocFileAct = new QAction(tr("Open Localization File"), this);
		m_openLocFileAct->setEnabled(true);
		QObject::connect(m_openLocFileAct, SIGNAL(triggered()), this, SLOT(openLocalizationDataset()));
		m_openSelectionMenu = new QMenu(this);
		if (m_openDirAct != NULL) m_openSelectionMenu->addAction(m_openDirAct);
		if (m_openLocFileAct != NULL) m_openSelectionMenu->addAction(m_openLocFileAct);
#else
		QObject::connect(m_openAct, SIGNAL(triggered()), this, SLOT(openLocalizationDataset()));
#endif
		//Connections for buttons of the toolbar
	QObject::connect(zoomAct, SIGNAL(triggered()), this, SLOT(setInteractionCamera()));
	QObject::connect(moveAct, SIGNAL(triggered()), this, SLOT(setInteractionCamera()));
	QObject::connect(m_gridAct, SIGNAL(triggered()), m_camera, SLOT(toggleGridDisplay()));
	QObject::connect( m_roisAct, SIGNAL( triggered() ), this, SLOT( setInteractionCamera() ));
	QObject::connect( m_aboutAct, SIGNAL( triggered() ), this, SLOT( aboutDialog() ));

	QObject::connect( m_camera, SIGNAL( updateSizeViewer() ), this, SLOT(adjustSizeViewer()) );
	QObject::connect( m_camera, SIGNAL( setStatusBarPosition(const QString &) ), this, SLOT( setStatusBarPosition(const QString &) ) );
	QObject::connect( m_camera, SIGNAL( setStatusBarZoom(const QString &) ), this, SLOT( setStatusBarZoom(const QString &) ) );
}

void ImageViewer::displayOrHideMenu()
{
	QObject * obj = QObject::sender();
	QAction * action = dynamic_cast <QAction *>(obj);

	if (action->text().compare("&Open") == 0)
		m_openSelectionMenu->exec(QCursor::pos());
}

ImageViewer::~ImageViewer()
{
	delete m_camera;
	delete m_hbar;
	delete m_vbar;
	delete zoomAct;
	delete moveAct;
	delete m_gridAct;

	Roi::destroyUnitCircle();
}

QSize ImageViewer::sizeHint() const
{
	int left, right, top, bottom;
	m_centralLayout->getContentsMargins(&left, &top, &right, &bottom);
	if(!m_initialized)
		return QWidget::sizeHint();

	int sizeNearBorders = 100;

	int sizeToAddW = 11, sizeToAddH = 16;
	QRect screen = QApplication::desktop()->screenGeometry();
	QPoint pos = this->pos();
	QSize sizeOtherElmts(left + right + sizeToAddW, m_editToolBar->size().height() + top + bottom + m_statusBar->size().height() + sizeToAddH);
	QSize size(m_camera->getZoomedWidth(), m_camera->getZoomedHeight());
	QSize total(size + sizeOtherElmts);
	int sizeBordureX = screen.width() - (pos.x()+width());
	if(sizeBordureX < 0)
		sizeBordureX = 0;
	if(sizeBordureX > sizeNearBorders)
		sizeBordureX = sizeNearBorders;
	int sizeBordureY = screen.height() - (pos.y()+height());
	if(sizeBordureY < 0)
		sizeBordureY = 0;
	if(sizeBordureY > sizeNearBorders)
		sizeBordureY = sizeNearBorders;
	int w = ((pos.x()+total.width()) < (screen.width() - sizeBordureX))?total.width():(screen.width()-pos.x() - sizeBordureX);
	int h = ((pos.y()+total.height()) < (screen.height() - sizeBordureY))?total.height():(screen.height()-pos.y() - sizeBordureY);
	if( w < total.width() ){
		m_hbar->show();
		total += QSize( 0, m_hbar->size().height() );
	}
	else
		m_hbar->hide();
	if( h < total.height() ){
		m_vbar->show();
		total += QSize( m_vbar->size().width(), 0 );
	}
	else
		m_vbar->hide();
	return QSize(w, h);
}

void ImageViewer::openPalmTracerDataset()
{
	QObject * obj = QObject::sender();
	QAction * action = dynamic_cast <QAction *>(obj);
#ifdef PALM_TRACER	
	if (action->text().compare("Open PalmTracer") == 0 || action->text().compare("Open PalmTracer2") == 0){
		FilesDeterminatorByDirectory fd;
		SuperResObject * obj = fd.openDataset();
		if (obj == NULL) return;
		m_camera->setSuperResObject(obj);
		m_camera->setDimension(obj->getWidth(), obj->getHeight());
		m_initialized = true;
		zoomAct->setEnabled(true);
		moveAct->setEnabled(true);
		m_gridAct->setEnabled(true);
		m_camera->adjustSize();
		m_camera->zoom(1.f, 0.f, 0.f);
}
#endif
	if (action->text().compare("Open Localization File") == 0)
		openLocalizationDataset();
}

void ImageViewer::openLocalizationDataset()
{
	const QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open File"),
		QDir::currentPath() + "/Data",
		tr("Localization Files (*.txt *.csv)"), 0, QFileDialog::DontUseNativeDialog);

	if (!fileName.isEmpty()) {
		int index = fileName.lastIndexOf('/');
		index++;
		QString dir(fileName);
		dir.replace(index, fileName.size() - index, "");
		QString name(fileName);
		name.replace(0, index, "");
		DetectionSet * detections = new DetectionSet();
		bool worked = detections->createFile(fileName.toAscii().data());
		if (!worked){
			delete detections;
			std::cout << "Loading of " << fileName.toAscii().data() << " have failed" << std::endl;
			return;
		}
		detections->createDisplayPoints(detections->getWidth(), detections->getHeight());
		SuperResObject * obj = new SuperResObject(dir.toAscii().data(), "Color1", detections->getWidth(), detections->getHeight());
		obj->setDetectionSet(detections);
		m_camera->setSuperResObject(obj);
		m_camera->setDimension(obj->getWidth(), obj->getHeight());
		m_initialized = true;
		zoomAct->setEnabled(true);
		moveAct->setEnabled(true);
		m_gridAct->setEnabled(true);
		m_camera->adjustSize();
		m_camera->zoom(1.f, 0.f, 0.f);
	}
}

void ImageViewer::setInteractionCamera()
{
	QAction * action = dynamic_cast < QAction * > ( QObject::sender() );
	if(action)
		m_camera->setInteraction(action->text());
}

void ImageViewer::adjustSizeViewer()
{
	resize( sizeHint() );
}

void ImageViewer::setStatusBarPosition( const QString & mess )
{
	m_labelStatusPosition->setText( mess );
}

void ImageViewer::setStatusBarZoom( const QString & mess )
{
	m_labelStatusZoom->setText(mess);
}

void ImageViewer::closeEvent( QCloseEvent * _ev )
{
	m_camera->closeAll();
	QWidget::closeEvent( _ev );
}

void ImageViewer::aboutDialog()
{
	QString text( "SR-Tesseler is developed by Florian Levet (florian.levet@inserm.fr),\n" );
	text.append( "research engineer in the Quantitative Imaging of the Cell team,\n" );
	text.append( "directed by Jean-Baptiste Sibarita.\n" );
	text.append( "F.L. and J.B.S. are part of the Interdisciplinary Institute for Neuroscience.\n" );
	text.append( "http://www.iins.u-bordeaux.fr/\n" );
	text.append( "F.L. is part of the Bordeaux Imaging Center.\n" );
	text.append( "http://www.bic.u-bordeaux.fr/\n" );
	text.append("\nSR-Tesseler version 1.0.0.1");
	QMessageBox message( QMessageBox::NoIcon, "About...", text );
	message.setIconPixmap( QPixmap( "./images/voronIcon1_2.PNG" ) );
	message.setWindowIcon( QIcon( "./images/voronIcon1.PNG" ) );
	message.setMinimumWidth( 1200 );
	message.exec();
}
