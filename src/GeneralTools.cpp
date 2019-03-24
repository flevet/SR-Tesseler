/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      GeneralTools.cpp
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

#include <vector>
#include <algorithm>
#include <math.h>
#include <float.h>
#include <QDir>
#include <QFileDialog>

#include "GeneralTools.hpp"
#include "ImageViewer.hpp"
#include "lmcurve.h"

ImageViewer * GeneralTools::m_imw = NULL;

double leeFunction2( double n, const double * p ){
	return pow( p[0], n ) * ( 1 - p[0] );
}

double expDecayHalfLife2( double t, const double * p ){
	return p[0] + p[1] * exp( -t / p[2] );
}

double expDecayValue2( double t, const double * p ){
	return p[0] * exp( -t * p[1] );
}

EquationFit::EquationFit()
{
	m_values = m_fitsValues = m_ts = m_paramsEqn = NULL;
}

EquationFit::EquationFit( double * _ts, double * _values, const int _nbTs, const int _typeEqn )
{
	m_values = m_fitsValues = m_ts = m_paramsEqn = NULL;
	setEquation( _ts, _values, _nbTs, _typeEqn );
}

void EquationFit::setEquation( double * _ts, double * _values, const int _nbTs, const int _typeEqn )
{
	if( m_values != NULL )
		delete [] m_values;
	if( m_fitsValues != NULL )
		delete [] m_fitsValues;
	if( m_ts != NULL )
		delete [] m_ts;
	if( m_paramsEqn != NULL )
		delete [] m_paramsEqn;

	m_nbTs = _nbTs;
	m_typeEqn = _typeEqn;

	m_ts = new double[m_nbTs];
	memcpy( m_ts, _ts, m_nbTs * sizeof( double ) );
	m_values = new double[m_nbTs];
	memcpy( m_values, _values, m_nbTs * sizeof( double ) );

	switch( m_typeEqn )
	{
	case LeeFunction:
		{
			m_nbParamEqn = 1;
			m_paramsEqn = new double[m_nbParamEqn];
			m_paramsEqn[0] = m_values[0];
			m_function = &leeFunction2;
			break;
		}
	case ExpDecayHalLife:
		{
			m_nbParamEqn = 3;
			m_paramsEqn = new double[m_nbParamEqn];
			m_paramsEqn[0] = m_values[m_nbTs-1]; m_paramsEqn[1] = m_values[0]; m_paramsEqn[2] = 2;
			m_function = &expDecayHalfLife2;
			break;
		}
	case ExpDecayValue:
		{
			m_nbParamEqn = 2;
			m_paramsEqn = new double[m_nbParamEqn];
			m_paramsEqn[0] = m_values[0]; m_paramsEqn[1] = 2;
			m_function = &expDecayValue2;
			break;
		}
	default:
		break;
	}

	lm_control_struct control;// = lm_control_double;
	lm_status_struct status;
	control.verbosity = 9;
	//printf( "Fitting ...\n" );
	lmcurve( m_nbParamEqn, m_paramsEqn, m_nbTs, m_ts, m_values, m_function, &control, &status );
	/*printf( "Results:\n" );
	printf( "status after %d function evaluations:\n  %s\n", status.nfev, lm_infmsg[status.outcome] );
	printf("obtained parameters:\n");
	for ( int i = 0; i < m_nbParamEqn; ++i)
		printf("  par[%i] = %12g\n", i, m_paramsEqn[i]);
	printf("obtained norm:\n  %12g\n", status.fnorm );*/
	m_fitsValues = new double[m_nbTs];
	//printf("fitting data as follows:\n");
	for ( int i = 0; i < m_nbTs; ++i){
		//printf( "  t[%2d]=%4g y=%6g fit=%10g residue=%12g\n", i, m_ts[i], m_values[i], m_function( m_ts[i], m_paramsEqn ), m_values[i] - m_function( m_ts[i], m_paramsEqn ) );
		m_fitsValues[i] = m_function( m_ts[i], m_paramsEqn );
	}

	switch( m_typeEqn ){
		case LeeFunction:
			m_eqn = ( "y = " + QString::number( m_paramsEqn[0] ) + "^x*( 1 - " + QString::number( m_paramsEqn[0] ) + ")" );
			break;
		case ExpDecayHalLife:
			m_eqn = ( "y = " + QString::number( m_paramsEqn[0] ) + " + " + QString::number( m_paramsEqn[1] ) + "*exp(-x/" + QString::number( m_paramsEqn[2] ) + ")" );
			break;
		case ExpDecayValue:
			m_eqn = ( "y = " + QString::number( m_paramsEqn[0] ) + "*exp(-" + QString::number( m_paramsEqn[1] ) + "x)" );
			break;
		default:
			break;
	}
}

EquationFit::~EquationFit()
{
	if( m_values != NULL )
		delete [] m_values;
	if( m_fitsValues != NULL )
		delete [] m_fitsValues;
	if( m_ts != NULL )
		delete [] m_ts;
	if( m_paramsEqn != NULL )
		delete [] m_paramsEqn;
}

double EquationFit::getFitValues( const double _t )
{
	return m_function( _t, m_paramsEqn );
}

ArrayStatistics GeneralTools::generateArrayStatistics( double * _data, const int _nb )
{
	double nb = _nb;
	ArrayStatistics stats;
	if( _nb == 0 ) return stats;
	stats.m_mean = stats.m_median = stats.m_stdDev = 0.;
	stats.m_max = DBL_MIN;
	stats.m_min = DBL_MAX;

	std::vector < double > vectMedian( _nb );
	for( int n = 0; n < _nb; n++ ){
		stats.m_mean += ( _data[n] / nb );
		vectMedian[n] = _data[n];
		if( _data[n] > stats.m_max )
			stats.m_max = _data[n];
		if( _data[n] < stats.m_min )
			stats.m_min = _data[n];
	}
	double nbForDev = nb - 1.;
	for( int n = 0; n < _nb; n++ ){
		double deviation = vectMedian[n] - stats.m_mean;
		stats.m_stdDev += ( ( deviation * deviation ) / nbForDev );
	}
	stats.m_stdDev = sqrt( stats.m_stdDev );
	std::sort( vectMedian.begin(), vectMedian.end() );
	stats.m_median = vectMedian[vectMedian.size() / 2];

	return stats;
}

ArrayStatistics GeneralTools::generateInverseArrayStatistics( double * _invData, const int _nb )
{
	double nbData = 0.;
	ArrayStatistics stats;
	if( _nb == 0 ) return stats;
	stats.m_mean = stats.m_median = stats.m_stdDev = 0.;
	stats.m_max = DBL_MIN;
	stats.m_min = DBL_MAX;
	for( int n = 0; n < _nb; n++ )
		nbData += _invData[n];
	std::vector < double > vectMedian( ( int )nbData );
	std::vector < double >::iterator it = vectMedian.begin();
	for( int n = 0; n < _nb; n++ ){
		stats.m_mean += ( ( ( double )n * _invData[n] ) / nbData );
		std::fill_n( it, ( int )_invData[n], n );
		it += _invData[n];
		if( _invData[n] > 0 && _invData[n] > stats.m_max )
			stats.m_max = _invData[n];
		if( _invData[n] > 0 && _invData[n] < stats.m_min )
			stats.m_min = _invData[n];
	}

	double nbForDev = nbData - 1.;
	for( int n = 0; n < nbData; n++ ){
		double deviation = vectMedian[n] - stats.m_mean;
		stats.m_stdDev += ( ( deviation * deviation ) / nbForDev );
	}
	stats.m_stdDev = sqrt( stats.m_stdDev );
	std::sort( vectMedian.begin(), vectMedian.end() );
	stats.m_median = vectMedian[vectMedian.size() / 2];

	return stats;
}

std::ostream & operator<<( std::ostream & _os, const ArrayStatistics & _stats )
{
	return _os << "[Mean : " << _stats.m_mean << ", median : " << _stats.m_median << ", std dev : " << _stats.m_stdDev << ", min : " << _stats.m_min << ", max = " << _stats.m_max << "]";
}

MyTimer::MyTimer()
{
	m_time.start();
}

MyTimer::~MyTimer()
{

}

const QString MyTimer::getTimeElapsed()
{
	int elapsedTime = m_time.elapsed();
	m_time.restart();
	QTime test = QTime();
	test = test.addMSecs( elapsedTime );
	QString s( "[" + QString::number( test.hour() ) + ":" + QString::number( test.minute() ) + ":" + QString::number( test.second() ) + ":" + QString::number( test.msec() ) + "] (h:min:s:ms)" );
	return s;
}
