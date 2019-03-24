/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      QCPHistogram.h
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

#ifndef QCPHistogram_h__
#define QCPHistogram_h__

#include "qcustomplot.h"
#include "..\Palette.hpp"
#include "..\GeneralTools.hpp"

class QCPHistogram: public QCustomPlot
{
	Q_OBJECT

public:
	QCPHistogram(QWidget * = 0);
	~QCPHistogram();

	void setPalette(Palette *);
	void setInfos(double *, const unsigned int, double *, double *, const unsigned int, Palette * = NULL);
	void update();

	void mousePressEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);

	inline Palette * getPalette() const { return m_palette; }
	
signals:
	void actionNeededSignal(const QString &);

protected:
	double * m_ts, * m_bins;
	unsigned int m_nbBins;
	Palette * m_palette;
	ArrayStatistics m_stats;
	bool m_buttonLeft, m_buttonRight;
};

#endif // QCPHistogram_h__