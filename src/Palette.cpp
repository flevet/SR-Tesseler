/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      Palette.cpp
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

#include "Palette.hpp"

Palette::Palette( const QColor _color_begin, const QColor _color_end, const bool _autoScale ):m_autoscale( _autoScale ), m_begin( 0. ), m_end( 1. )
{
	m_gradient.setColorAt( m_begin, _color_begin );
	m_gradient.setColorAt( m_end, _color_end );

	m_gradientAutoscale = m_gradient;
}

Palette::~Palette()
{

}

void Palette::setColor( const qreal _position, const QColor & _color )
{
	Q_ASSERT( _position >= 0.0 && _position <= 1.0 );
	m_gradient.setColorAt( _position, _color );

	generateAutoscaleGradient();
}

const QColor Palette::getColor( const qreal _pos )
{
	const QGradientStops & stops = ( m_autoscale ) ? m_gradientAutoscale.stops() : m_gradient.stops();

	for( unsigned int n = 1; n < stops.size(); n++ ){
		qreal pos1 = stops[n-1].first, pos2 = stops[n].first;
		if( pos1 == _pos )
			return stops[n-1].second.rgba();
		if( pos2 == _pos )
			return stops[n].second.rgba();
		if( pos1 < _pos && _pos < pos2 ){
			QColor c2 = stops[n-1].second.rgba(), c1 = stops[n].second.rgba();
			qreal segmentLength = pos2 - pos1;
			qreal pdist = _pos - pos1;
			qreal ratio = pdist / segmentLength;
			int red = ( int )( ratio * c1.red() + ( 1 - ratio ) * c2.red() );
			int green = ( int )( ratio * c1.green() + ( 1 - ratio ) * c2.green() );
			int blue = ( int )( ratio * c1.blue() + ( 1 - ratio ) * c2.blue() );
			return QColor(  red, green, blue );
		}
	}
	return stops[0].second.rgba();
}

void Palette::generateAutoscaleGradient()
{
	double sub = m_end - m_begin;

	QLinearGradient gradientAutoS;
	const QGradientStops& stops = m_gradient.stops();
	if( m_begin != 0. )
		gradientAutoS.setColorAt( stops.first().first, stops.first().second );
	if(m_end != 1. )
		gradientAutoS.setColorAt( stops.last().first, stops.last().second );
	for( int i = 0; i < stops.size(); ++i )
	{
		qreal pos = m_begin + stops[i].first * sub;
		gradientAutoS.setColorAt( pos, stops[i].second );
	}
	m_gradientAutoscale = gradientAutoS;
}

void Palette::setPaletteAutoscale( const double _begin, const double _end )
{
	m_begin = _begin;
	m_end = _end;

	generateAutoscaleGradient();
}

void Palette::setAutoscale( const bool _autoScale )
{
	m_autoscale = _autoScale;
	if( m_autoscale )
		generateAutoscaleGradient();
}

Palette * Palette::getStaticLut( const QString & _lut )
{
	if( _lut == QString( "Gray" ) ){
		return new Palette( Qt::black, Qt::white );
	}
	else if( _lut == QString( "Red" ) ){
		return new Palette( Qt::black, Qt::red );
	}
	else if( _lut == QString( "Green" ) ){
		return new Palette( Qt::black, Qt::green );
	}
	else if( _lut == QString( "Blue" ) ){
		return new Palette( Qt::black, Qt::blue );
	}
	else if( _lut == QString( "AllGray" ) ){
		return new Palette( Qt::gray, Qt::gray );
	}
	else if( _lut == QString( "Fire" ) ){
		int r[] = {0,0,1,25,49,73,98,122,146,162,173,184,195,207,217,229,240,252,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
		int g[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,14,35,57,79,101,117,133,147,161,175,190,205,219,234,248,255,255,255,255};
		int b[] = {0,61,96,130,165,192,220,227,210,181,151,122,93,64,35,5,0,0,0,0,0,0,0,0,0,0,0,35,98,160,223,255};
		int w = 32;
		std::vector < QColor > colors;
		for( int i = 1; i < w; i++ )
			if( i % 4 == 0 )
				colors.push_back( QColor( r[i], g[i], b[i] ) );
		Palette * palette = new Palette( QColor( 0, 0, 0), QColor( 255, 255, 255 ) );
		double step = 1. / 8.;
		double cur = step;
		for( unsigned int i = 0; i < colors.size(); i++, cur += step )
			palette->setColor( cur, colors[i] );
		return palette;
	}
	else if( _lut == QString( "InvFire" ) ){
		int r[] = {0,0,1,25,49,73,98,122,146,162,173,184,195,207,217,229,240,252,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
		int g[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,14,35,57,79,101,117,133,147,161,175,190,205,219,234,248,255,255,255,255};
		int b[] = {0,61,96,130,165,192,220,227,210,181,151,122,93,64,35,5,0,0,0,0,0,0,0,0,0,0,0,35,98,160,223,255};
		int w = 32;
		std::vector < QColor > colors;
		for( int i = w - 1; i > 0; i-- )
			if( i % 4 == 0 )
				colors.push_back( QColor( r[i], g[i], b[i] ) );
		Palette * palette = new Palette( QColor( 255, 255, 255 ), QColor( 0, 0, 0 ) );
		double step = 1. / 8.;
		double cur = step;
		for( unsigned int i = 0; i < colors.size(); i++, cur += step )
			palette->setColor( cur, colors[i] );
		return palette;
	}else if( _lut == QString( "Ice" ) ){
		int r[] = {0,0,0,0,0,0,19,29,50,48,79,112,134,158,186,201,217,229,242,250,250,250,250,251,250,250,250,250,251,251,243,230};
		int g[] = {156,165,176,184,190,196,193,184,171,162,146,125,107,93,81,87,92,97,95,93,93,90,85,69,64,54,47,35,19,0,4,0};
		int b[] = {140,147,158,166,170,176,209,220,234,225,236,246,250,251,250,250,245,230,230,222,202,180,163,142,123,114,106,94,84,64,26,27};
		int w = 32;
		std::vector < QColor > colors;
		for( int i = 1; i < w; i++ )
			if( i % 4 == 0 )
				colors.push_back( QColor( r[i], g[i], b[i] ) );
		Palette * palette = new Palette( QColor( 0, 0, 0), QColor( 255, 255, 255 ) );
		double step = 1. / 8.;
		double cur = step;
		for( unsigned int i = 0; i < colors.size(); i++, cur += step )
			palette->setColor( cur, colors[i] );
		return palette;
	}
	else if( _lut == QString( "AllBlue" ) ){
		return new Palette( QColor( 0, 85, 255 ), QColor( 0, 85, 255 ) );
	}
	else if( _lut == QString( "AllGreen" ) ){
		return new Palette( QColor( 0, 170, 127 ), QColor( 0, 170, 127 ) );
	}
	else if( _lut == QString( "AllWhite" ) ){
		return new Palette( QColor( 255, 255, 255 ), QColor( 255, 255, 255 ) );
	}
	else if( _lut == QString( "AllBlack" ) ){
		return new Palette( QColor( 0, 0, 0 ), QColor( 0, 0, 0 ) );
	}
	else if( _lut == QString( "HotCold" ) ){
		/*Palette * palette = new Palette( QColor( 0, 0, 255), QColor( 170, 0, 255 ) );
		palette->setColor( 0.25, QColor(0, 170, 255) );
		palette->setColor( 0.5, QColor(255, 255, 0) );
		palette->setColor( 0.67, QColor(255, 170, 0) );
		palette->setColor( 0.8, QColor(255, 0, 0) );*/
		Palette * palette = new Palette( QColor( 0, 0, 255), QColor( 170, 0, 255 ) );
		palette->setColor( 0.1, QColor(0, 170, 255) );
		palette->setColor( 0.225, QColor(103, 255, 139) );
		palette->setColor( 0.35, QColor(255, 255, 0) );
		palette->setColor( 0.5, QColor(255, 170, 0) );
		palette->setColor( 0.7, QColor(255, 0, 0) );
		return palette;
	}
	return NULL;
}

Palette * Palette::getMonochromePalette( const int _r, const int _g, const int _b )
{
	return new Palette( QColor( _r, _g, _b ), QColor( _r, _g, _b ) );
}
