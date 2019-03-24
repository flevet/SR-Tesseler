/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      MiscQuantificationWidget.cpp
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

#include <QLineEdit>
#include <QGridLayout>
#include <QButtonGroup>
#include <QHeaderView>
#include <QFileDialog>
#include <QColorDialog>
#include <fstream>

#include "MiscQuantificationWidget.hpp"
#include "Camera2D.hpp"
#include "DetectionSet.hpp"
#include "DBScan.hpp"
#include "KRipley.hpp"

MiscQuantificationWidget::MiscQuantificationWidget(Camera2D * _cam, QWidget* _parent) : QTabWidget(_parent), m_lsSelected(true)
{
	QRegExp rx("[0-9.]+");
	QValidator * validator = new QRegExpValidator(rx, this);
	unsigned int columnCount = 0;

	QWidget * ripleyFunctionsWidget = new QWidget;
	QWidget * DBScanWidget = new QWidget;

	/************************************************************************/
	/* For DBScan                                                           */
	/************************************************************************/
	m_groupDBScan = new QGroupBox(QObject::tr("DBScan"));
	m_groupDBScan->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_distanceDBScanLbl = new QLabel("Distance:");
	m_distanceDBScanLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_leditDistanceDBScan = new QLineEdit("50");
	m_leditDistanceDBScan->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	QLabel * minDDBScanLbl = new QLabel("Min # locs:");
	minDDBScanLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_leditMinDDBScan = new QLineEdit("50");
	m_leditMinDDBScan->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_buttonDBScan = new QPushButton("DBScan");
	m_buttonDBScan->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_buttonExportDBSCANRes = new QPushButton("Export results");
	m_buttonExportDBSCANRes->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	QLabel * minNbPtsClusterDBSCAN = new QLabel("Min # locs in cluster:");
	minNbPtsClusterDBSCAN->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_leditMinPtsPerCluster = new QLineEdit("15");
	m_leditMinPtsPerCluster->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_cboxOneColorDBSCAN = new QCheckBox("One color");
	m_cboxOneColorDBSCAN->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_cboxOneColorDBSCAN->setChecked(true);
	m_cboxColorPerObjDBSCAN = new QCheckBox("Random color per cluster");
	m_cboxColorPerObjDBSCAN->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_cboxColorPerObjDBSCAN->setChecked(true);
	QButtonGroup * bgroupDBSCAN = new QButtonGroup;
	bgroupDBSCAN->addButton(m_cboxOneColorDBSCAN);
	bgroupDBSCAN->addButton(m_cboxColorPerObjDBSCAN);
	m_cboxDisplayDBSCANLabels = new QCheckBox("Display labels");
	m_cboxDisplayDBSCANLabels->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_cboxDisplayDBSCANLabels->setChecked(true);

	m_colorBack.set(0, 0.67, 0.5, 1);
	QLabel * backColorLbl = new QLabel("Background color:");
	backColorLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_colorBackBtn = new QPushButton();
	m_colorBackBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_colorBackBtn->setStyleSheet("background-color: rgb(0, 170, 127);"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 5px;"
		"border-color: black;"
		"font: 12px;"
		"min-width: 5em;"
		"padding: 3px;"
		);
	m_colorObj.set(1, 0, 0, 1);
	QLabel * objColorLbl = new QLabel("Cluster color:");
	objColorLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_colorObjsBtn = new QPushButton();
	m_colorObjsBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_colorObjsBtn->setStyleSheet("background-color: rgb(255, 0, 0);"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 5px;"
		"border-color: black;"
		"font: 12px;"
		"min-width: 5em;"
		"padding: 3px;"
		);

	m_customPlotDBSCAN = new QCPHistogram();
	m_customPlotDBSCAN->setMinimumHeight(300);
	m_customPlotDBSCAN->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_customPlotDBSCAN->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
	m_customPlotDBSCAN->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
	//m_customPlotDBSCAN->moveLayer(m_customPlotDBSCAN->layer("grid"), m_customPlotDBSCAN->layer("main"), QCustomPlot::limAbove);
	m_customPlotDBSCAN->legend->setTextColor(Qt::black);
	QFont fontLegend("Helvetica", 9);
	fontLegend.setBold(true);
	m_customPlotDBSCAN->legend->setFont(fontLegend);
	m_customPlotDBSCAN->legend->setBrush(Qt::NoBrush);
	m_customPlotDBSCAN->legend->setBorderPen(Qt::NoPen);
	QColor background = QWidget::palette().color(QWidget::backgroundRole());
	m_customPlotDBSCAN->setBackground(background);
	m_customPlotDBSCAN->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
	m_tableObjs = new QTableWidget;
	m_tableObjs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QStringList tableHeader;
	tableHeader << "Size" << "# detections" << "Major axis" << "Minor axis";
	m_tableObjs->setColumnCount(tableHeader.size());
	m_tableObjs->setHorizontalHeaderLabels(tableHeader);
	m_cboxPCAEllipse = new QCheckBox("PCA ellipse");
	m_cboxPCAEllipse->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_cboxBoundingEllipse = new QCheckBox("Bounding ellipse");
	m_cboxBoundingEllipse->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_buttonGroupEllipse = new QButtonGroup;
	m_buttonGroupEllipse->addButton(m_cboxPCAEllipse);
	m_buttonGroupEllipse->addButton(m_cboxBoundingEllipse);
	m_cboxPCAEllipse->setChecked(true);
	QGridLayout * layoutDBScan = new QGridLayout;
	columnCount = 0;
	layoutDBScan->addWidget(m_distanceDBScanLbl, 0, 0, 1, 1);
	layoutDBScan->addWidget(m_leditDistanceDBScan, 0, 1, 1, 1);
	layoutDBScan->addWidget(minDDBScanLbl, 0, 2, 1, 1);
	layoutDBScan->addWidget(m_leditMinDDBScan, 0, 3, 1, 1);
	layoutDBScan->addWidget(m_buttonDBScan, 0, 4, 1, 1);
	layoutDBScan->addWidget(minNbPtsClusterDBSCAN, 1, 0, 1, 1);
	layoutDBScan->addWidget(m_leditMinPtsPerCluster, 1, 1, 1, 1);
	layoutDBScan->addWidget(m_cboxOneColorDBSCAN, 1, 2, 1, 1);
	layoutDBScan->addWidget(m_cboxColorPerObjDBSCAN, 1, 3, 1, 1);
	layoutDBScan->addWidget(m_buttonExportDBSCANRes, 1, 4, 1, 1);
	layoutDBScan->addWidget(m_cboxDisplayDBSCANLabels, 2, 0, 1, 1);
	layoutDBScan->addWidget(m_cboxPCAEllipse, 2, 1, 1, 1);
	layoutDBScan->addWidget(m_cboxBoundingEllipse, 2, 2, 1, 1);

	layoutDBScan->addWidget(backColorLbl, 3, 0, 1, 1);
	layoutDBScan->addWidget(m_colorBackBtn, 3, 1, 1, 1);
	layoutDBScan->addWidget(objColorLbl, 3, 2, 1, 1);
	layoutDBScan->addWidget(m_colorObjsBtn, 3, 3, 1, 1);

	layoutDBScan->addWidget(m_customPlotDBSCAN, 4, 0, 1, 5);
	layoutDBScan->addWidget(m_tableObjs, 5, 0, 1, 5);
	m_groupDBScan->setLayout(layoutDBScan);

	/************************************************************************/
	/* For KRipley                                                         */
	/************************************************************************/
	m_groupKRipley = new QGroupBox(QObject::tr("K-Ripley"));
	m_groupKRipley->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_minKRipleyLbl = new QLabel("Min radius:");
	m_minKRipleyLbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	m_minKRipleyEdit = new QLineEdit("10");
	m_minKRipleyEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	m_maxKRipleyLbl = new QLabel("Max radius:");
	m_maxKRipleyLbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	m_maxKRipleyEdit = new QLineEdit("200");
	m_maxKRipleyEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	m_stepKRipleyLbl = new QLabel("Step radius:");
	m_stepKRipleyLbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	m_stepKRipleyEdit = new QLineEdit("10");
	m_stepKRipleyEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	m_cboxLsDisplayKRipley = new QCheckBox("Display L function");
	m_cboxLsDisplayKRipley->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	m_cboxLsDisplayKRipley->setChecked(true);
	m_cboxRipleyOnROIs = new QCheckBox("On ROIs");
	m_cboxRipleyOnROIs->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	m_cboxRipleyOnROIs->setChecked(false);
	m_buttonKRipley = new QPushButton("K-Ripley");
	m_buttonKRipley->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	m_customPlotKRipley = new QCustomPlot();
	m_customPlotKRipley->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_customPlotKRipley->setMinimumHeight(300);
	m_customPlotKRipley->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_customPlotKRipley->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
	m_customPlotKRipley->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
	//m_customPlotKRipley->moveLayer(m_customPlotDBSCAN->layer("grid"), m_customPlotDBSCAN->layer("main"), QCustomPlot::limAbove);
	m_customPlotKRipley->legend->setTextColor(Qt::black);
	m_customPlotKRipley->legend->setFont(fontLegend);
	m_customPlotKRipley->legend->setBrush(Qt::NoBrush);
	m_customPlotKRipley->legend->setBorderPen(Qt::NoPen);
	m_customPlotKRipley->setBackground(background);
	m_customPlotKRipley->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
	m_resKRipleyLbl = new QLabel("");
	m_resKRipleyLbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	m_buttonExportKRipleyRes = new QPushButton("Export results");
	m_buttonExportKRipleyRes->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	QGridLayout * layoutKripley = new QGridLayout;
	columnCount = 0;
	layoutKripley->addWidget(m_minKRipleyLbl, 0, 0, 1, 1);
	layoutKripley->addWidget(m_minKRipleyEdit, 0, 1, 1, 1);
	layoutKripley->addWidget(m_maxKRipleyLbl, 0, 2, 1, 1);
	layoutKripley->addWidget(m_maxKRipleyEdit, 0, 3, 1, 1);
	layoutKripley->addWidget(m_buttonKRipley, 0, 4, 1, 1);
	layoutKripley->addWidget(m_stepKRipleyLbl, 1, 0, 1, 1);
	layoutKripley->addWidget(m_stepKRipleyEdit, 1, 1, 1, 1);
	layoutKripley->addWidget(m_cboxLsDisplayKRipley, 1, 2, 1, 1);
	layoutKripley->addWidget(m_buttonExportKRipleyRes, 1, 4, 1, 1);
	layoutKripley->addWidget(m_customPlotKRipley, 2, 0, 1, 5);
	layoutKripley->addWidget(m_resKRipleyLbl, 3, 0, 1, 5);
	m_groupKRipley->setLayout(layoutKripley);

	/*QVBoxLayout * layoutMisc = new QVBoxLayout;
	layoutMisc->addWidget(m_groupKRipley);
	layoutMisc->addWidget(m_groupDBScan);
	this->setLayout(layoutMisc);*/

	QVBoxLayout * layoutRipleyW = new QVBoxLayout;
	layoutRipleyW->addWidget(m_groupKRipley);
	QWidget * emptyRipleyW = new QWidget;
	emptyRipleyW->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	layoutRipleyW->addWidget(emptyRipleyW);
	ripleyFunctionsWidget->setLayout(layoutRipleyW);

	QVBoxLayout * layoutDBScanW = new QVBoxLayout;
	layoutDBScanW->addWidget(m_groupDBScan);
	DBScanWidget->setLayout(layoutDBScanW);

	this->addTab(ripleyFunctionsWidget, tr("Ripley's functions"));
	this->addTab(DBScanWidget, tr("DBScan"));

	setCurrentCamera(_cam);

	QObject::connect(m_buttonDBScan, SIGNAL(pressed()), this, SLOT(computeDBSCAN()));
	QObject::connect(m_buttonExportDBSCANRes, SIGNAL(pressed()), this, SLOT(exportDBSCANResults()));
	QObject::connect(m_buttonExportKRipleyRes, SIGNAL(pressed()), this, SLOT(exportKRipleyResults()));
	QObject::connect(m_buttonKRipley, SIGNAL(pressed()), this, SLOT(computeKRipley()));
	QObject::connect(m_cboxLsDisplayKRipley, SIGNAL(toggled(bool)), this, SLOT(toggleRipleyFunctionDisplay(bool)));

	QObject::connect(m_cboxDisplayDBSCANLabels, SIGNAL(toggled(bool)), _cam, SLOT(toggleDisplayDBSCANClusterLabels(bool)));

	QObject::connect(m_colorBackBtn, SIGNAL(pressed()), this, SLOT(changeBackgroundColor()));
	QObject::connect(m_colorObjsBtn, SIGNAL(pressed()), this, SLOT(changeObjectColor()));
}

MiscQuantificationWidget::~MiscQuantificationWidget()
{
}

void MiscQuantificationWidget::computeDBSCAN()
{
	bool ok = true;
	unsigned int tmpUI = m_leditMinDDBScan->text().toUInt(&ok), minLocs = (ok) ? tmpUI : 10, nbLocsInsideDBClusters = 0;
	double tmpD = m_leditDistanceDBScan->text().toDouble(&ok), d = (ok) ? tmpD : 0.3;
	unsigned int valUI = m_leditMinPtsPerCluster->text().toUInt(&ok);
	unsigned int nbMinInClusters = ok ? valUI : 15;

	SuperResObject * sobj = m_currentCamera->getSuperResObject();
	DBScan * dbscan = sobj->getDBSCAN();
	DetectionSet * dset = m_currentCamera->getDetectionSet();
	/*if (dbscan != NULL)
		delete dbscan;
	dbscan = new DBScan(dset, d, minLocs, nbMinInClusters);*/
	dbscan->execute(d, minLocs, nbMinInClusters, m_cboxPCAEllipse->isChecked());

	if (m_cboxOneColorDBSCAN->isChecked()){
		unsigned int * tmp = new unsigned int[dset->nbPoints()];
		for (unsigned int n = 0; n < dset->nbPoints(); n++) tmp[n] = n;
		dset->colorLocsOfObject(tmp, dset->nbPoints(), m_colorBack);
		delete[] tmp;

		unsigned int * indexes = dbscan->getColorLocsSelected(nbLocsInsideDBClusters, nbMinInClusters);
		dset->colorLocsOfObject(indexes, nbLocsInsideDBClusters, m_colorObj);
		delete[] indexes;
	}
	else{
		Color4D * colors = dbscan->getColorPerClusters(nbMinInClusters);
		dset->setColors(colors);
		delete[] colors;
	}

	//Creation of the histogram of the clusters size
	unsigned int nbBins = 50;
	double * bins = new double[nbBins], *ts = new double[nbBins], minH = DBL_MAX, maxH = 0.;
	memset(bins, 0, nbBins * sizeof(double));
	double * values = dbscan->getSizeClusters();
	double * majors = dbscan->getMajorAxisClusters();
	double * minors = dbscan->getMinorAxisClusters();
	unsigned int nbClusters = dbscan->getNbClusters();
	for (unsigned int n = 0; n < nbClusters; n++){
		if (values[n] > maxH) maxH = values[n];
		if (values[n] < minH) minH = values[n];
	}
	double step = (maxH - minH) / (double)nbBins;
	for (unsigned int n = 0; n < nbClusters; n++){
		unsigned int index = floor((values[n] - minH) / step);
		if (index < nbBins)
			bins[index] = bins[index] + 1.;
	}
	for (unsigned int n = 0; n < nbBins; n++)
		ts[n] = minH + (double)n * step;

	m_customPlotDBSCAN->setInfos(values, nbClusters, ts, bins, nbBins, Palette::getStaticLut("AllGreen"));

	m_tableObjs->clear();
	QStringList tableHeader;
	tableHeader << "Size" << "# detections" << "Major axis" << "Minor axis";
	m_tableObjs->setHorizontalHeaderLabels(tableHeader);
	m_tableObjs->setRowCount(nbClusters);
	if (nbClusters > 0){
		double * nbLocsClusters = dbscan->getNbLocsClusters();
		for (int i = 0; i < nbClusters; i++){
			int y = 0;
			m_tableObjs->setItem(i, y++, new QTableWidgetItem(QString::number(values[i])));
			m_tableObjs->setItem(i, y++, new QTableWidgetItem(QString::number(nbLocsClusters[i])));
			m_tableObjs->setItem(i, y++, new QTableWidgetItem(QString::number(majors[i])));
			m_tableObjs->setItem(i, y++, new QTableWidgetItem(QString::number(minors[i])));
		}
	}
	m_tableObjs->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

	delete[] bins;
	delete[] ts;
	m_currentCamera->updateGL();
}

void MiscQuantificationWidget::computeKRipley()
{
	SuperResObject * sobj = m_currentCamera->getSuperResObject();
	KRipley * kripley = sobj->getKRipley();

	bool ok = true;
	double tmpD1 = m_minKRipleyEdit->text().toDouble(&ok), minR = (ok) ? tmpD1 : 0.1;
	double tmpD2 = m_maxKRipleyEdit->text().toDouble(&ok), maxR = (ok) ? tmpD2 : 10;
	double tmpD3 = m_stepKRipleyEdit->text().toDouble(&ok), stepR = (ok) ? tmpD3 : 0.1;
	DetectionSet * dset = m_currentCamera->getDetectionSet();
	if (dset != NULL){
		SuperResObject * sobj = m_currentCamera->getSuperResObject();
		/*if (kripley != NULL)
			delete kripley;
		kripley = new KRipley(dset, minR, maxR, stepR, sobj->getWidth(), sobj->getHeight());*/
		kripley->computeKRipley(minR, maxR, stepR, m_cboxRipleyOnROIs->isChecked(), sobj->getRois());
		/*std::string filename = sobj->getDir();
		filename.append("/kripley.txt");
		kripley->exportResults(filename);*/
		setKripleyCurveDisplay();
	}
}

void MiscQuantificationWidget::setKripleyCurveDisplay()
{
	SuperResObject * sobj = m_currentCamera->getSuperResObject();
	KRipley * kripley = sobj->getKRipley();

	if (kripley == NULL) return;

	unsigned int nbBins = kripley->getNbSteps(), indexMaxY = 0, indexMaxYForL = 0;
	double * ts = kripley->getTs();
	double * values = m_lsSelected ? kripley->getLs() : kripley->getKs(), maxValue = -DBL_MAX, minValue = DBL_MAX;
	double * lValues = kripley->getLs();
	QVector<double> x1(nbBins), y1(nbBins);
	for (unsigned int j = 0; j < nbBins; j++){
		x1[j] = ts[j];
		y1[j] = values[j];
		if (y1[j] > maxValue) maxValue = y1[j];
		if (y1[j] < minValue) minValue = y1[j];
		if (y1[j] > y1[indexMaxY]) indexMaxY = j;
	
		if (lValues[j] > lValues[indexMaxYForL]) indexMaxYForL = j;
	}

	QCustomPlot * customPlot = m_customPlotKRipley;
	customPlot->clearGraphs();
	customPlot->clearItems();

	if (indexMaxY > 0 && indexMaxY < nbBins - 1){
		QCPItemLine *arrow = new QCPItemLine(customPlot);
		QPen penLines(Qt::black);
		penLines.setWidth(1);
		customPlot->addItem(arrow);
		arrow->setPen(penLines);
		arrow->start->setCoords(ts[indexMaxY], 0);
		arrow->end->setCoords(ts[indexMaxY], maxValue);
		m_resKRipleyLbl->setText(QString("Radius of maximum aggregation: %1").arg(ts[indexMaxY]));
	}
	if (indexMaxYForL > 0 && indexMaxYForL < nbBins - 1)
		m_resKRipleyLbl->setText(QString("Radius of maximum aggregation: %1").arg(ts[indexMaxYForL]));
	else
		m_resKRipleyLbl->setText("No radius of maximum aggregation was found");

	unsigned int currentGraph = 0;
	customPlot->legend->clearItems();
	customPlot->legend->setVisible(true);
	customPlot->legend->setFont(QFont("Helvetica", 9));
	customPlot->addGraph();
	customPlot->graph(currentGraph)->setPen(QPen(Qt::blue));
	customPlot->graph(currentGraph)->setName( m_lsSelected ? "L Ripley" : "K Ripley");
	customPlot->graph(currentGraph)->setData(x1, y1);
	customPlot->yAxis->setRange(minValue, maxValue);
	customPlot->xAxis->setRange(ts[0], ts[nbBins - 1]);
	customPlot->replot();
	customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}

void MiscQuantificationWidget::toggleRipleyFunctionDisplay(bool _val)
{
	m_lsSelected = _val;
	setKripleyCurveDisplay();
}

void MiscQuantificationWidget::exportDBSCANResults()
{
	SuperResObject * sobj = m_currentCamera->getSuperResObject();
	DBScan * dbscan = sobj->getDBSCAN();

	QString nameXls(sobj->getDir().c_str());
	nameXls.append("./DBSCAN_results.xls");
	nameXls = QFileDialog::getSaveFileName(NULL, QObject::tr("Save stats..."), nameXls, QObject::tr("Stats files (*.xls)"), 0, QFileDialog::DontUseNativeDialog);
	if (nameXls.isEmpty()) return;
	std::ofstream fs(nameXls.toAscii().data());
	if (!fs){
		std::cout << "System failed to open " << nameXls.toAscii().data() << std::endl;
		return;
	}
	else
		std::cout << "Saving stats in file " << nameXls.toAscii().data() << std::endl;

	double * values = dbscan->getSizeClusters();
	double * majors = dbscan->getMajorAxisClusters();
	double * minors = dbscan->getMinorAxisClusters();
	unsigned int nbClusters = dbscan->getNbClusters();
	double * nbLocsClusters = dbscan->getNbLocsClusters();
	fs << "Index\tSize\t# locs\tMajor axis\tMinor axis" << std::endl;
	for (int i = 0; i < nbClusters; i++)
		fs << (i + 1) << "\t" << values[i] << "\t" << nbLocsClusters[i] << "\t" << majors[i] << "\t" << minors[i] << std::endl;
	fs.close();
}

void MiscQuantificationWidget::exportKRipleyResults()
{
	SuperResObject * sobj = m_currentCamera->getSuperResObject();
	KRipley * kripley = sobj->getKRipley();

	QString nameXls(sobj->getDir().c_str());
	nameXls.append("./KRipley_results.xls");
	nameXls = QFileDialog::getSaveFileName(NULL, QObject::tr("Save stats..."), nameXls, QObject::tr("Stats files (*.xls)"), 0, QFileDialog::DontUseNativeDialog);
	if (nameXls.isEmpty()) return;
	std::ofstream fs(nameXls.toAscii().data());
	if (!fs){
		std::cout << "System failed to open " << nameXls.toAscii().data() << std::endl;
		return;
	}
	else
		std::cout << "Saving stats in file " << nameXls.toAscii().data() << std::endl;

	double * ks = kripley->getKs(), *ls = kripley->getLs(), *ts = kripley->getTs();
	unsigned int nbSteps = kripley->getNbSteps();
	fs << "Radius\tK value\tL value" << std::endl;
	for (int i = 0; i < nbSteps; i++)
		fs << ts[i] << "\t" << ks[i] << "\t" << ls[i] << std::endl;
	fs.close();
}

void MiscQuantificationWidget::changeBackgroundColor()
{
	QColor color = QColorDialog::getColor(QColor((int)(m_colorBack[0] * 255), (int)(m_colorBack[1] * 255), (int)(m_colorBack[2] * 255)));
	if (color.isValid())
		m_colorBack.set(color.redF(), color.greenF(), color.blueF(), 1);
	m_colorBackBtn->setStyleSheet("background-color: rgb(" + QString::number((int)(m_colorBack[0] * 255)) + ", " + QString::number((int)(m_colorBack[1] * 255)) + ", " + QString::number((int)(m_colorBack[2] * 255)) + ");"
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

void MiscQuantificationWidget::changeObjectColor()
{
	QColor color = QColorDialog::getColor(QColor((int)(m_colorObj[0] * 255), (int)(m_colorObj[1] * 255), (int)(m_colorObj[2] * 255)));
	if (color.isValid())
		m_colorObj.set(color.redF(), color.greenF(), color.blueF(), 1);
	m_colorObjsBtn->setStyleSheet("background-color: rgb(" + QString::number((int)(m_colorObj[0] * 255)) + ", " + QString::number((int)(m_colorObj[1] * 255)) + ", " + QString::number((int)(m_colorObj[2] * 255)) + ");"
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