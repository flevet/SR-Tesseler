/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      QCPHistogram.cpp
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

#include "QCPHistogram.h"
#include "QCPStringLegendItem.h"
#include "QCPBarsWithColors.h"


QCPHistogram::QCPHistogram(QWidget * _parent) :QCustomPlot(_parent), m_palette(NULL), m_buttonLeft(false), m_buttonRight(false)
{
	m_bins = NULL;
	m_ts = NULL;
}

QCPHistogram::~QCPHistogram()
{
	if (m_palette != NULL)
		delete m_palette;
	if(m_bins != NULL)
		delete[] m_bins;
	if (m_ts != NULL)
		delete[] m_ts;
}

void QCPHistogram::setInfos(double * _values, const unsigned int _nbValues, double * _ts, double * _bins, const unsigned int _nbBins, Palette * _palette)
{
	m_stats = GeneralTools::generateArrayStatistics(_values, _nbValues);

	if (m_bins != NULL)
		delete[] m_bins;
	if (m_ts != NULL)
		delete[] m_ts;
	m_nbBins = _nbBins;
	m_bins = new double[m_nbBins];
	m_ts = new double[m_nbBins];
	memcpy(m_ts, _ts, m_nbBins * sizeof(double));
	memcpy(m_bins, _bins, m_nbBins * sizeof(double));
	
	if (_palette != NULL){
		if (m_palette != NULL)
			delete m_palette;
		m_palette = new Palette(*_palette);
	}

	update();
}

void QCPHistogram::setPalette(Palette * _pal){ 
	if (m_palette != NULL) 
		delete m_palette; 
	m_palette = new Palette(*_pal);

	update();
}

void QCPHistogram::update()
{
	double * bins = m_bins, *ts = m_ts, maxY = 0.;
	unsigned int nbBins = m_nbBins;
	QVector<double> x1(nbBins), y1(nbBins);
	for (unsigned int n = 0; n < nbBins; n++){
		x1[n] = ts[n];
		y1[n] = bins[n];
		if (y1[n] > maxY) maxY = y1[n];
	}
	double minX = x1[0], maxX = x1[nbBins - 1], bin = (maxX - minX) / (double)nbBins, step = 1. / (double)nbBins;
	QVector<QColor> colors(nbBins);
	for (unsigned int n = 0; n < nbBins; n++)
		colors[n] = m_palette->getColor((double)n * step);

	this->clearGraphs();
	this->clearPlottables();
	this->clearItems();
	
	//Histogram
	QCPBarsWithColors *newBars = new QCPBarsWithColors(this->xAxis, this->yAxis);
	this->addPlottable(newBars);
	newBars->setName("Experimental values");
	newBars->setDataWithColors(x1, y1, colors);
	this->xAxis->setRange(minX, maxX);
	this->yAxis->setRange(0, maxY);
	newBars->setWidth(bin);

	// Bounds min/max
	QCPItemLine *arrow = new QCPItemLine(this);
	QPen penLines(Qt::black);
	penLines.setWidth(1);
	this->addItem(arrow);
	arrow->setPen(penLines);
	arrow->start->setCoords(minX - bin / 2., 0.);
	arrow->end->setCoords(minX - bin / 2., maxY);
	arrow = new QCPItemLine(this);
	this->addItem(arrow);
	arrow->setPen(penLines);
	arrow->start->setCoords(maxX + bin / 2., 0.);
	arrow->end->setCoords(maxX + bin / 2., maxY);

	this->legend->clearItems();
	this->legend->setVisible(true);
	QString strLabel = QString("Mean: %1").arg(m_stats.m_mean);
	this->legend->insertRow(this->legend->rowCount());
	this->legend->addElement(this->legend->rowCount() - 1, 0, new QCPStringLegendItem(this->legend, strLabel));
	strLabel = QString("Median: %1").arg(m_stats.m_median);
	this->legend->insertRow(this->legend->rowCount());
	this->legend->addElement(this->legend->rowCount() - 1, 0, new QCPStringLegendItem(this->legend, strLabel));
	strLabel = QString("Std dev: %1").arg(m_stats.m_stdDev);
	this->legend->insertRow(this->legend->rowCount());
	this->legend->addElement(this->legend->rowCount() - 1, 0, new QCPStringLegendItem(this->legend, strLabel));
	this->replot();
}

void QCPHistogram::mousePressEvent(QMouseEvent * _event)
{
	if (_event->modifiers() == Qt::ShiftModifier){
		QCustomPlot::mousePressEvent(_event);
		return;
	}
	switch (_event->button())
	{
	case Qt::LeftButton:
	{
		int itemcount = this->itemCount();
		if (this->itemCount() != 2) return;
		QCPItemLine * lineC = qobject_cast<QCPItemLine*>(this->item(0));
		QCPItemLine * lineO = qobject_cast<QCPItemLine*>(this->item(1));
		if (!lineC || !lineO) return;
		
		m_buttonLeft = true;
		double x = this->xAxis->pixelToCoord(_event->pos().x());
		if (x > lineO->start->coords().x())
			x = lineO->start->coords().x();
		lineC->start->setCoords(x, lineC->start->coords().y());
		lineC->end->setCoords(x, lineC->end->coords().y());

	
		this->replot();
		
		emit(actionNeededSignal("changeBoundsCustom"));
		break;
	}
	case Qt::MidButton:
	{
		emit(actionNeededSignal("modify"));
		break;
	}
	case Qt::RightButton:
	{
		int itemcount = this->itemCount();
		if (this->itemCount() != 2) return;
		QCPItemLine * lineC = qobject_cast<QCPItemLine*>(this->item(1));
		QCPItemLine * lineO = qobject_cast<QCPItemLine*>(this->item(0));
		if (!lineC || !lineO) return;

		m_buttonRight = true;
		double x = this->xAxis->pixelToCoord(_event->pos().x());
		if (x < lineO->start->coords().x())
			x = lineO->start->coords().x();
		lineC->start->setCoords(x, lineC->start->coords().y());
		lineC->end->setCoords(x, lineC->end->coords().y());

		this->replot();

		emit(actionNeededSignal("changeBoundsCustom"));
		break;
	}
	default:
		break;
	}
}

void QCPHistogram::mouseMoveEvent(QMouseEvent * _event)
{
	QCustomPlot::mouseMoveEvent(_event);

	this->replot();
}

void QCPHistogram::mouseReleaseEvent(QMouseEvent * _event)
{
	QCustomPlot::mouseReleaseEvent(_event);
	m_buttonLeft = m_buttonRight = false;
}