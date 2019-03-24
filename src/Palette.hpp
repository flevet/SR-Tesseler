/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      Palette.hpp
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

#ifndef Palette_h__
#define Palette_h__

#include <QLinearGradient>

#include "Palette.hpp"

class Palette{
public:
	Palette( const QColor = Qt::black, const QColor = Qt::white, const bool = true );
	~Palette();

	void setColor( const qreal, const QColor & );
	const QColor getColor( const qreal );
	void setPaletteAutoscale( const double _begin, const double _end );
	void setAutoscale( const bool );

	inline const bool isAutoScale() const {return m_autoscale;}
	inline void setGradient( const QLinearGradient & _gradient ) {m_gradient = _gradient;}
	inline const QLinearGradient & linearGradient() const {return m_gradient;}
	inline void setGradientAutoscale( const QLinearGradient & _gradientAutoscale ) {m_gradientAutoscale = _gradientAutoscale;}
	const QLinearGradient& linearGradientAutoscale() const { return m_gradientAutoscale; }

	static Palette * getStaticLut( const QString & );
	static Palette * getMonochromePalette( const int, const int, const int );

protected:
	void generateAutoscaleGradient();

protected:
	QLinearGradient m_gradient, m_gradientAutoscale;

	bool m_autoscale;
	double m_begin, m_end;
};

#endif // Palette_h__