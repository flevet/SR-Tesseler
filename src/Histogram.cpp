/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      Histogram.cpp
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

#include <float.h>
#include "Histogram.hpp"
#include "DetectionSet.hpp"
#include "WrapperVoronoiDiagram.hpp"

int Histogram::BINS = 256;
int Histogram::NORMAL = 0;
int Histogram::LOG = 1;

std::ostream & operator<<(std::ostream & _os, const HistParam & _params)
{
	return _os << "[MinH : " << _params.m_minH << ", maxH : " << _params.m_maxH << ", stepX : " << _params.m_stepX << ", maxY : " << _params.m_maxY << ", currentMin = " << _params.m_currentMin << ", currentMax = " << _params.m_currentMax << "]";
}



Histogram::Histogram()
{
	m_values[NORMAL] = m_values[LOG] = NULL;
	m_logOrNot = NORMAL;
}

Histogram::Histogram( ObjectInterface * _data, const short _log, const int _type ):m_logOrNot( _log ), m_type( _type )
{
	DetectionSet * detections = dynamic_cast < DetectionSet * >( _data );
	if( detections )
		createHistogram( detections );
	WrapperVoronoiDiagram * wrapper = dynamic_cast < WrapperVoronoiDiagram * >( _data );
	if( wrapper )
		createHistogram( wrapper );
	VoronoiObject * obj = dynamic_cast < VoronoiObject * >( _data );
	if( obj )
		createHistogram( obj );
}

Histogram::Histogram( const Histogram & _o ):m_logOrNot( _o.m_logOrNot ), m_type( _o.m_type )
{
	m_values[NORMAL] = new double[Histogram::BINS];
	memcpy( m_values[NORMAL], _o.m_values[NORMAL], Histogram::BINS * sizeof( double ) );
	m_values[LOG] = new double[Histogram::BINS];
	memcpy( m_values[LOG], _o.m_values[LOG], Histogram::BINS * sizeof( double ) );
	m_params[NORMAL] = HistParam( _o.m_params[NORMAL] );
	m_params[LOG] = HistParam( _o.m_params[LOG] );
}

void Histogram::createHistogram( DetectionSet * _data )
{
	m_values[NORMAL] = m_values[LOG] = NULL;
	m_params[NORMAL].m_minH = m_params[LOG].m_minH = DBL_MAX;
	m_params[NORMAL].m_maxH = m_params[NORMAL].m_maxY = m_params[LOG].m_maxH = m_params[LOG].m_maxY = DBL_MIN;

	m_values[NORMAL] = new double[Histogram::BINS];
	m_values[LOG] = new double[Histogram::BINS];
	for(int i = 0; i < Histogram::BINS; i++)
		m_values[NORMAL][i] = m_values[LOG][i] = 0.;
	for(unsigned int i = 0; i < _data->size(); i++){
		if( _data->isDataSelected( i ) )
		{
			double intensity = _data->getIntensity( i );
			if( intensity < m_params[NORMAL].m_minH )
				m_params[NORMAL].m_minH = intensity;
			if( intensity > m_params[NORMAL].m_maxH )
				m_params[NORMAL].m_maxH = intensity;
			intensity = MiscFunction::log10Custom( intensity );
			if( intensity < m_params[LOG].m_minH )
				m_params[LOG].m_minH = intensity;
			if( intensity > m_params[LOG].m_maxH )
				m_params[LOG].m_maxH = intensity;
		}
	}
	m_params[NORMAL].m_stepX = (m_params[NORMAL].m_maxH - m_params[NORMAL].m_minH) / (double)( Histogram::BINS - 1 );
	m_params[LOG].m_stepX = (m_params[LOG].m_maxH - m_params[LOG].m_minH) / (double)( Histogram::BINS - 1 );
	m_params[NORMAL].m_maxH += m_params[NORMAL].m_stepX; m_params[LOG].m_maxH += m_params[LOG].m_stepX;
	m_params[NORMAL].m_stepX = ( m_params[NORMAL].m_maxH - m_params[NORMAL].m_minH ) / ( double )( Histogram::BINS - 1 );
	m_params[LOG].m_stepX = ( m_params[LOG].m_maxH - m_params[LOG].m_minH ) / ( double )( Histogram::BINS - 1 );
	for(unsigned int i = 0; i < _data->size(); i++){
		if( _data->isDataSelected( i ) ){
			double intensity = _data->getIntensity( i );
			ushort index = ( intensity - m_params[NORMAL].m_minH ) / m_params[NORMAL].m_stepX;
			if( index < Histogram::BINS )
				m_values[NORMAL][index]++;
			intensity = MiscFunction::log10Custom( intensity );
			index = ( intensity - m_params[LOG].m_minH ) / m_params[LOG].m_stepX;
			if( index < Histogram::BINS )
				m_values[LOG][index]++;
		}
	}
	for( int n = 0; n < 2; n++ )
		for( int i = 0; i < Histogram::BINS; i++ )
			if( m_values[n][i] > m_params[n].m_maxY )
				m_params[n].m_maxY = m_values[n][i];
	for( int n = 0; n < 2; n++ ){
		m_params[n].m_currentMin = m_params[n].m_minH;
		m_params[n].m_currentMax = m_params[n].m_maxH;
	}
}

void Histogram::createHistogram( WrapperVoronoiDiagram * _data )
{
	m_values[NORMAL] = m_values[LOG] = NULL;
	m_params[NORMAL].m_minH = m_params[LOG].m_minH = DBL_MAX;
	m_params[NORMAL].m_maxH = m_params[NORMAL].m_maxY = m_params[LOG].m_maxH = m_params[LOG].m_maxY = DBL_MIN;

	m_values[NORMAL] = new double[Histogram::BINS];
	m_values[LOG] = new double[Histogram::BINS];
	for(int i = 0; i < Histogram::BINS; i++)
		m_values[NORMAL][i] = m_values[LOG][i] = 0.;
	for(unsigned int i = 0; i < _data->nbMolecules(); i++){
		if( _data->isDataSelected( i ) )
		{
			double val = _data->getInfosData( m_type, i );
			if( val < m_params[NORMAL].m_minH )
				m_params[NORMAL].m_minH = val;
			if( val > m_params[NORMAL].m_maxH )
				m_params[NORMAL].m_maxH = val;
			val = _data->getInfosDataLog( m_type, i );
			if( val < m_params[LOG].m_minH )
				m_params[LOG].m_minH = val;
			if( val > m_params[LOG].m_maxH )
				m_params[LOG].m_maxH = val;
		}
	}
	m_params[NORMAL].m_stepX = (m_params[NORMAL].m_maxH - m_params[NORMAL].m_minH) / (double)( Histogram::BINS - 1 );
	m_params[LOG].m_stepX = (m_params[LOG].m_maxH - m_params[LOG].m_minH) / (double)( Histogram::BINS - 1 );
	m_params[NORMAL].m_maxH += m_params[NORMAL].m_stepX; m_params[LOG].m_maxH += m_params[LOG].m_stepX;
	m_params[NORMAL].m_stepX = ( m_params[NORMAL].m_maxH - m_params[NORMAL].m_minH ) / ( double )( Histogram::BINS - 1 );
	m_params[LOG].m_stepX = ( m_params[LOG].m_maxH - m_params[LOG].m_minH ) / ( double )( Histogram::BINS - 1 );
	for(unsigned int i = 0; i < _data->nbMolecules(); i++){
		if( _data->isDataSelected( i ) ){
			double val = _data->getInfosData( m_type, i );
			ushort index = ( val - m_params[NORMAL].m_minH ) / m_params[NORMAL].m_stepX;
			if( index < Histogram::BINS )
				m_values[NORMAL][index]++;
			val = _data->getInfosDataLog( m_type, i );
			index = ( val - m_params[LOG].m_minH ) / m_params[LOG].m_stepX;
			if( index < Histogram::BINS )
				m_values[LOG][index]++;
		}
	}
	for( int n = 0; n < 2; n++ )
		for( int i = 0; i < Histogram::BINS; i++ )
			if( m_values[n][i] > m_params[n].m_maxY )
				m_params[n].m_maxY = m_values[n][i];
	for( int n = 0; n < 2; n++ ){
		m_params[n].m_currentMin = m_params[n].m_minH;
		m_params[n].m_currentMax = m_params[n].m_maxH;
	}
}

void Histogram::createHistogram( VoronoiObject *_data )
{
	m_values[NORMAL] = m_values[LOG] = NULL;
	m_params[NORMAL].m_minH = m_params[LOG].m_minH = DBL_MAX;
	m_params[NORMAL].m_maxH = m_params[NORMAL].m_maxY = m_params[LOG].m_maxH = m_params[LOG].m_maxY = DBL_MIN;

	m_values[NORMAL] = new double[Histogram::BINS];
	m_values[LOG] = new double[Histogram::BINS];
	for(int i = 0; i < Histogram::BINS; i++)
		m_values[NORMAL][i] = m_values[LOG][i] = 0.;
	for(unsigned int i = 0; i < _data->nbMolecules(); i++){
		if( _data->isDataSelected( i ) )
		{
			double val = _data->getInfosData( m_type, i );
			if( val < m_params[NORMAL].m_minH )
				m_params[NORMAL].m_minH = val;
			if( val > m_params[NORMAL].m_maxH )
				m_params[NORMAL].m_maxH = val;
			val = _data->getInfosDataLog( m_type, i );
			if( val < m_params[LOG].m_minH )
				m_params[LOG].m_minH = val;
			if( val > m_params[LOG].m_maxH )
				m_params[LOG].m_maxH = val;
		}
	}
	m_params[NORMAL].m_stepX = (m_params[NORMAL].m_maxH - m_params[NORMAL].m_minH) / (double)( Histogram::BINS - 1 );
	m_params[LOG].m_stepX = (m_params[LOG].m_maxH - m_params[LOG].m_minH) / (double)( Histogram::BINS - 1 );
	m_params[NORMAL].m_maxH += m_params[NORMAL].m_stepX; m_params[LOG].m_maxH += m_params[LOG].m_stepX;
	m_params[NORMAL].m_stepX = ( m_params[NORMAL].m_maxH - m_params[NORMAL].m_minH ) / ( double )( Histogram::BINS - 1 );
	m_params[LOG].m_stepX = ( m_params[LOG].m_maxH - m_params[LOG].m_minH ) / ( double )( Histogram::BINS - 1 );
	for(unsigned int i = 0; i < _data->nbMolecules(); i++){
		if( _data->isDataSelected( i ) ){
			double val = _data->getInfosData( m_type, i );
			ushort index = ( val - m_params[NORMAL].m_minH ) / m_params[NORMAL].m_stepX;
			if( index < Histogram::BINS )
				m_values[NORMAL][index]++;
			val = _data->getInfosDataLog( m_type, i );
			index = ( val - m_params[LOG].m_minH ) / m_params[LOG].m_stepX;
			if( index < Histogram::BINS )
				m_values[LOG][index]++;
		}
	}
	for( int n = 0; n < 2; n++ )
		for( int i = 0; i < Histogram::BINS; i++ )
			if( m_values[n][i] > m_params[n].m_maxY )
				m_params[n].m_maxY = m_values[n][i];
	for( int n = 0; n < 2; n++ ){
		m_params[n].m_currentMin = m_params[n].m_minH;
		m_params[n].m_currentMax = m_params[n].m_maxH;
	}
}

Histogram::~Histogram()
{
	for( int n = 0; n < 2; n++ )
		if( m_values[n] != NULL )
			delete [] m_values[n];
}

void Histogram::setParameters( double & _minH, double & _maxH, double & _stepX, double & _maxY )
{
	_minH = m_params[m_logOrNot].m_minH;
	_maxH = m_params[m_logOrNot].m_maxH;
	_stepX = m_params[m_logOrNot].m_stepX;
	_maxY = m_params[m_logOrNot].m_maxY;
}

double Histogram::getMinH() const
{
	return m_params[m_logOrNot].m_minH;
}

double Histogram::getMaxH() const
{
	return m_params[m_logOrNot].m_maxH;
}

double Histogram::getMinY() const
{
	return m_params[m_logOrNot].m_maxY;
}

double Histogram::getStep() const
{
	return m_params[m_logOrNot].m_stepX;
}

double * Histogram::getHistogram()
{
	return m_values[m_logOrNot];
}

void Histogram::eraseBounds()
{
	for( int n = 0; n < 2; n++ ){
		m_params[n].m_currentMin = DBL_MIN;
		m_params[n].m_currentMax = DBL_MAX;
	}
}


void Histogram::resetBounds()
{
	for( int n = 0; n < 2; n++ ){
		m_params[n].m_currentMin = m_params[n].m_minH;
		m_params[n].m_currentMax = m_params[n].m_maxH;
	}
}

void Histogram::setLog( const int _log )
{
	m_logOrNot = _log;
}

void Histogram::setMin( const double _min )
{
	m_params[m_logOrNot].m_currentMin = _min;
}

void Histogram::setMax( const double _max )
{
	m_params[m_logOrNot].m_currentMax = _max;
}

void Histogram::setBounds( const double _min, const double _max )
{
	m_params[m_logOrNot].m_currentMin = _min;
	m_params[m_logOrNot].m_currentMax = _max;
}

void Histogram::getBounds( double & _min, double & _max )
{
	_min = m_params[m_logOrNot].m_currentMin; 
	_max = m_params[m_logOrNot].m_currentMax;
}
