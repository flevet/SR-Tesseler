/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      QCPBarsWithColors.cpp
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

#include "QCPBarsWithColors.h"

QCPBarsWithColors::QCPBarsWithColors(QCPAxis * _keyAxis, QCPAxis * _valueAxis):QCPBars(_keyAxis, _valueAxis)
{
	m_colors = new QMap < double, QColor >;
}

QCPBarsWithColors::~QCPBarsWithColors()
{
	delete m_colors;
}

void QCPBarsWithColors::setDataWithColors(const QVector<double> &key, const QVector<double> &value, const QVector<QColor> &colors)
{
	mData->clear();
	m_colors->clear();
	int n = key.size();
	n = qMin(n, value.size());
	QCPBarData newData;
	for (int i = 0; i<n; ++i)
	{
		newData.key = key[i];
		newData.value = value[i];
		mData->insertMulti(newData.key, newData);
		m_colors->insertMulti(newData.key, colors[i]);
	}
}

void QCPBarsWithColors::draw(QCPPainter *painter)
{
	if (!mKeyAxis || !mValueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
	if (mData->isEmpty()) return;

	QCPBarDataMap::const_iterator it, lower, upperEnd;
	getVisibleDataBounds(lower, upperEnd);
	for (it = lower; it != upperEnd; ++it)
	{
		// check data validity if flag set:
#ifdef QCUSTOMPLOT_CHECK_DATA
		if (QCP::isInvalidData(it.value().key, it.value().value))
			qDebug() << Q_FUNC_INFO << "Data point at" << it.key() << "of drawn range invalid." << "Plottable name:" << name();
#endif
		QPolygonF barPolygon = getBarPolygon(it.key(), it.value().value);
		// draw bar fill:
		if (mainBrush().style() != Qt::NoBrush && mainBrush().color().alpha() != 0)
		{
			applyFillAntialiasingHint(painter);
			painter->setPen(Qt::NoPen);
			painter->setBrush(m_colors->value(it.key()));
			//painter->setBrush(mainBrush());
			painter->drawPolygon(barPolygon);
		}
		// draw bar line:
		if (mainPen().style() != Qt::NoPen && mainPen().color().alpha() != 0)
		{
			applyDefaultAntialiasingHint(painter);
			//painter->setPen(mainPen());
			painter->setPen(m_colors->value(it.key()));
			painter->setBrush(Qt::NoBrush);
			painter->drawPolyline(barPolygon);
		}
	}
}