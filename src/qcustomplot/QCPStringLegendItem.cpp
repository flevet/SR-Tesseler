/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      QCPStringLegendItem.cpp
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

#include "QCPStringLegendItem.h"

QCPStringLegendItem::QCPStringLegendItem(QCPLegend *pParent, const QString& strText)
	: QCPAbstractLegendItem(pParent)
	, m_strText(strText)
{
}

QString QCPStringLegendItem::text() const
{
	return m_strText;
}

void QCPStringLegendItem::setText(const QString& strText)
{
	m_strText = strText;
}

void QCPStringLegendItem::draw(QCPPainter *pPainter)
{
	pPainter->setFont(mFont);
	pPainter->setPen(QPen(mTextColor));
	QRectF textRect = pPainter->fontMetrics().boundingRect(0, 0, 0, 0, Qt::TextDontClip, m_strText);
	pPainter->drawText(mRect.x() + mMargins.left(), mRect.y(), textRect.width(), textRect.height(), Qt::TextDontClip | Qt::AlignHCenter, m_strText);
}

QSize QCPStringLegendItem::minimumSizeHint() const
{
	QSize cSize(0, 0);
	QFontMetrics fontMetrics(mFont);
	QRect textRect = textRect = fontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip, m_strText);
	cSize.setWidth(textRect.width() + mMargins.left() + mMargins.right());
	cSize.setHeight(textRect.height() + mMargins.top() + mMargins.bottom());
	return cSize;
}