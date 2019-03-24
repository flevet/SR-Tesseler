/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      DetectionCleaner.cpp
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

#include <windows.h>
#include <iostream>
#include <GL/gl.h>
#include <fstream>
#include <limits.h>
#include <math.h>
#include <CGAL/ch_graham_andrew.h>
#include <QTime>
#include <qmath.h>

#include "DetectionCleaner.hpp"
#include "DetectionSet.hpp"
#include "lmcurve.h"
#include "GeneralTools.hpp"

void swap( CleanerPoint ** _array, double * _ts, const int index1, const int index2 ){
	CleanerPoint * tmp = _array[index1];
	_array[index1] = _array[index2];
	_array[index2] = tmp;
	double tmp2 =_ts[index1];
	_ts[index1] = _ts[index2];
	_ts[index2] = tmp2;
}

void quicksort( CleanerPoint ** _array, double * _ts, const int _start, const int _end ){
	int left = _start - 1, right = _end + 1;
	double pivot = _ts[_start];
	if( _start >= _end )
		return;
	while( 1 ){
		do right--; while( _ts[right] > pivot );
		do left++; while( _ts[left] < pivot );

		if( left < right )
			swap( _array, _ts, left, right );
		else break;
	}
	quicksort( _array, _ts, _start, right );
	quicksort( _array, _ts, right + 1, _end );
}

CleanerPoint::CleanerPoint():m_t( 0 ), m_done(false)
{
	m_point = NULL;
}

CleanerPoint::CleanerPoint(DetectionPoint * _dp, const double _t, const double _intensity, const double _sigma) : m_t(_t), m_intensity(_intensity), m_sigma(_sigma), m_done(false)
{
	m_point = _dp;
}

CleanerPoint::~CleanerPoint()
{
}

void CleanerPoint::set(DetectionPoint * _dp, const double _t, const double _intensity, const double _sigma)
{
	m_point = _dp;
	m_t = _t;
	m_intensity = _intensity;
	m_sigma = _sigma;
}

DetectionCleaner::DetectionCleaner(DetectionSet * _dset, const double _sizeNeigh, const double _pixelValue, const double _background, const double _ratioInt2Photon, const int _maxDarkTime, const unsigned char _options, const std::string & _dir) :m_sizeNeigh(_sizeNeigh), m_pixelValue(_pixelValue), m_background(_background), m_ratioInt2Photon(_ratioInt2Photon), m_maxDarkTime(_maxDarkTime), m_options(_options)
{
	m_totalRemoved = m_totalAdded = m_totalDetections = 0.;

	int nbTime = _dset->nbSlices();
	m_hasSigma = _dset->hasSigmaPerLocalization();

	QTime time, test;
	time.start();
	std::cout << "Beginning cleaning of the detection set" << std::endl;

	//Creation of the cleanerPoints in a single dimension array, needs to have the nb points per time and starting points in the array then
	CleanerPoint * cpoints = new CleanerPoint[_dset->getNbPoints()];
	DetectionPoint * originalPoints = _dset->getPoints();
	uint * firstPointTime = _dset->getFirstPoint(), * nbPointsTime = _dset->getSizePoints();
	double * intensities = _dset->getIntensities();
	CleanerPoint * ptrC = cpoints;
	for( int t = 0; t < nbTime; t++ )
		for( int n = 0; n < nbPointsTime[t]; n++ ){
			int index = firstPointTime[t] + n;
			( *ptrC++ ).set( &originalPoints[index], t, intensities[index], m_hasSigma ? _dset->getSigma(index) : 0. );
		}

	CleanerPoint ** unchangedCPoints = new CleanerPoint*[_dset->getNbPoints()];
	Vec4md * newCPoints = new Vec4md[_dset->getNbPoints()];
	int currentUnchanged = 0, currentNew = 0;

	int targetNbMolecules = 0, maxDarkTime = 20;
	determineMaxDarkTimePaper( cpoints, _dset->getNbPoints(), firstPointTime, nbPointsTime, nbTime );
	for( int n = 0; n < _dset->getNbPoints(); n++ )
		cpoints[n].m_done = false;
	cleanDetectionSet( cpoints, _dset->getNbPoints(), firstPointTime, nbPointsTime, nbTime, unchangedCPoints, currentUnchanged, newCPoints, currentNew, m_maxDarkTime );

	//For voronoiDiagram
	m_nbTotalClean = currentUnchanged + currentNew;
	m_xs = new double[m_nbTotalClean];
	m_ys = new double[m_nbTotalClean];
	m_ts = new unsigned short[m_nbTotalClean];
	m_nbPhotons = new unsigned int[m_nbTotalClean];
	double * ptrX = m_xs, * ptrY = m_ys;
	unsigned short * ptrT = m_ts;
	unsigned int * ptrPhotons = m_nbPhotons;
	for( int i = 0; i < currentUnchanged; i++ ){
		*ptrX = unchangedCPoints[i]->m_point->x();
		*ptrY = unchangedCPoints[i]->m_point->y();
		*ptrT = ( unsigned short )unchangedCPoints[i]->m_t;
		*ptrPhotons = ( unsigned int )unchangedCPoints[i]->getIntensity();
		ptrX++;
		ptrY++;
		ptrT++;
		ptrPhotons++;
	}
	for( int i = 0; i < currentNew; i++ ){
		*ptrX = newCPoints[i].x();
		*ptrY = newCPoints[i].y();
		*ptrT = ( unsigned short )newCPoints[i].z();
		*ptrPhotons = ( unsigned int )newCPoints[i].w();
		ptrX++;
		ptrY++;
		ptrT++;
		ptrPhotons++;
	}

	delete [] unchangedCPoints;
	delete [] newCPoints;
	delete [] cpoints;

	double blinkFit = m_eqnBlinks->getParams()[0];
	double tonFit = m_eqnTOns->getParams()[1];
	double kd = blinkFit * tonFit;
	double kb = tonFit - kd;
	double nblink = 1 + ( kd / kb );
	double controlNbMol = ( double )m_nbEmBurst / nblink;
	double err = ( ( fabs( m_nbTotalClean - controlNbMol ) ) / m_nbTotalClean ) * 100.;

	QString tau( 0x03C4 );
	m_statsCleaner += "k_d / ( k_d + k_b ) = " + QString::number(blinkFit) + "\nk_d + k_b = " + QString::number(tonFit) + "\nk_d = " + QString::number(kd) + "\nk_b = " + QString::number(kb) + "\nN_blinks = 1 + (k_d / k_b) = " + QString::number(nblink) + "\n# detections for " + tau + "_" + QString::number(m_maxDarkTime) + " = " + QString::number(m_nbTotalClean) + "\n\nControl by blinks (#emission burst / N_blinks):\n# emission burst = " + QString::number(m_nbEmBurst) + "\n# detections for control by blinks = " + QString::number(m_nbEmBurst) + " / " + QString::number(nblink) + " = " + QString::number(controlNbMol) + "\n\nNormalized difference: (" + QString::number(m_nbTotalClean) + " - " + QString::number(controlNbMol) + " ) / " + QString::number(m_nbTotalClean) + " = " + QString::number(err) + "%";

	m_totalDetections = _dset->getNbPoints();
	m_totalRemoved = m_totalDetections - currentUnchanged;
	m_totalAdded = currentNew;

	int elapsedTime = time.elapsed();
	time.restart();
	test = QTime();
	test = test.addMSecs( elapsedTime );
	std::cout << "Ending cleaning of the detection set, elapsed time [" << test.hour() << ":" << test.minute() << ":" << test.second() << ":" << test.msec() << "] (h:min:s:ms)" << std::endl;
	m_toggleDisplay = true;
}

DetectionCleaner::~DetectionCleaner()
{

}

void DetectionCleaner::determineMaxDarkTimePaper( CleanerPoint * _cpoints, const int _nbTotalData, unsigned int * _firstPointTime, unsigned int * _nbPointsTime, const int _nbTime )
{
	m_nbEmBurst = computeNbEmissionBurst( _cpoints, _nbTotalData, _firstPointTime, _nbPointsTime, _nbTime );
	int darkTimeByStats = computeAnalysisParameters( _cpoints, _nbTotalData, _firstPointTime, _nbPointsTime, _nbTime, m_maxDarkTime );
	m_maxDarkTime = (m_options & DetectionCleaner::FixedMaxDarkTimeFlag) ? m_maxDarkTime : darkTimeByStats;
}

void DetectionCleaner::cleanDetectionSet( CleanerPoint * _cpoints, const int _nbTotalData, unsigned int * _firstPointTime, unsigned int * _nbPointsTime, const int _nbTime, CleanerPoint ** _unchangedCPoints, int & _currentUnchanged, Vec4md * _newCPoints, int & _currentNew, const int _darkTime )
{
	//Definition of the distance function pointer
	DetectionCleaner::DistanceFunction dfunction;
	if( m_options & DetectionCleaner::FixedDistanceFlag ) dfunction = &DetectionCleaner::fixedDistance;
	if( m_options & DetectionCleaner::PhotonDistanceFlag ) dfunction = &DetectionCleaner::photonDistance;
	if( m_options & DetectionCleaner::PhotonBackgroundDistanceFlag ) dfunction = &DetectionCleaner::photonBackgroundDistance;

	CleanerPoint ** tmpCPoints = new CleanerPoint *[_nbTotalData];
	int size = 0;
	for( int t = 0; t < _nbTime; t++ ){
		for( int n = _firstPointTime[t]; n < _firstPointTime[t] + _nbPointsTime[t]; n++ ){
			if( !_cpoints[n].m_done ){
				_cpoints[n].m_done = true;
				Vec2md barycenter(_cpoints[n].getPoint()->x(), _cpoints[n].getPoint()->y());
				size = 0;
				tmpCPoints[size++] = &_cpoints[n];
				int currentNeigh = 0, currentBlinks = 0, timeN = t + 1, nbDetections = 0;
				double x = _cpoints[n].getPoint()->x(), y = _cpoints[n].getPoint()->y(), totalIntensity = 0.;
				while( currentBlinks <= _darkTime && timeN < _nbTime ){
					bool neighFound = false;
					int index;
					double d = pow((this->*dfunction)(_cpoints[n].getIntensity(), m_hasSigma ? _cpoints[n].getSigma() : 0.), 2);
					for( int n2 = _firstPointTime[timeN]; n2 < _firstPointTime[timeN] + _nbPointsTime[timeN]; n2++ ){
						if( !_cpoints[n2].m_done ){
							double x2 = _cpoints[n2].getPoint()->x() - barycenter.x(), y2 = _cpoints[n2].getPoint()->y() - barycenter.y();
							double length = x2 * x2 + y2 * y2;
							if( length < d ){
								neighFound = true;
								index = n2;
								d = length;
							}
						}
					}
					if( neighFound ){
						nbDetections++;
						totalIntensity += _cpoints[index].getIntensity();
						_cpoints[index].m_done = true;
						currentBlinks = 0;
						tmpCPoints[size++] = &_cpoints[index];
						averagingPosition(barycenter, tmpCPoints, size);
					}
					else
						currentBlinks++;
					timeN++;
				}
				if( nbDetections == 0 )
					_unchangedCPoints[_currentUnchanged++] = &_cpoints[n];
				else
					_newCPoints[_currentNew++].set( barycenter.x(), barycenter.y(), _cpoints[n].getT(), totalIntensity );
				size = 0;
			}
		}
	}
}


int DetectionCleaner::computeNbEmissionBurst( CleanerPoint * _cpoints, const int _nbTotalData, unsigned int * _firstPointTime, unsigned int * _nbPointsTime, const int _nbTime )
{
	//Definition of the distance function pointer
	DetectionCleaner::DistanceFunction dfunction;
	if( m_options & DetectionCleaner::FixedDistanceFlag ) dfunction = &DetectionCleaner::fixedDistance;
	if( m_options & DetectionCleaner::PhotonDistanceFlag ) dfunction = &DetectionCleaner::photonDistance;
	if( m_options & DetectionCleaner::PhotonBackgroundDistanceFlag ) dfunction = &DetectionCleaner::photonBackgroundDistance;

	/****Computation of the number of emission burst (done with a dark time of 0) ********/
	CleanerPoint ** tmpCPoints = new CleanerPoint *[_nbTotalData];
	int size = 0, nbEmBurst = 0, maxDarkTime = 1;
	for( int n = 0; n < _nbTotalData; n++ )
		_cpoints[n].m_done = false;
	for( int t = 0; t < _nbTime; t++ ){
		for( int n = _firstPointTime[t]; n < _firstPointTime[t] + _nbPointsTime[t]; n++ ){
			if( !_cpoints[n].m_done ){
				_cpoints[n].m_done = true;
				size = 0;
				tmpCPoints[size++] = &_cpoints[n];
				int currentNeigh = 0, currentDarkTime = 0, timeN = t + 1;
				Vec2md barycenter(_cpoints[n].getPoint()->x(), _cpoints[n].getPoint()->y());
				while (currentDarkTime < maxDarkTime && timeN < _nbTime){
					bool neighFound = false;
					int index;			
					double d = pow((this->*dfunction)(_cpoints[n].getIntensity(), m_hasSigma ? _cpoints[n].getSigma() : 0.), 2);
					for( int n2 = _firstPointTime[timeN]; n2 < _firstPointTime[timeN] + _nbPointsTime[timeN]; n2++ ){
						if( !_cpoints[n2].m_done ){
							double x2 = _cpoints[n2].getPoint()->x() - barycenter.x(), y2 = _cpoints[n2].getPoint()->y() - barycenter.y();
							double length = x2 * x2 + y2 * y2;
							if( length < d ){
								neighFound = true;
								index = n2;
								d = length;
							}
						}
					}
					if( neighFound ){
						tmpCPoints[size++] = &_cpoints[index];
						averagingPosition(barycenter, tmpCPoints, size);
						_cpoints[index].m_done = true;
						currentDarkTime = 0;
					}
					else
						currentDarkTime++;
					timeN++;
				}
				nbEmBurst++;
			}
		}
	}
	return nbEmBurst;
}

int DetectionCleaner::computeAnalysisParameters( CleanerPoint * _cpoints, const int _nbTotalData, unsigned int * _firstPointTime, unsigned int * _nbPointsTime, const int _nbTime, const int _maxDarkTime )
{
	//Definition of the distance function pointer
	DetectionCleaner::DistanceFunction dfunction;
	if( m_options & DetectionCleaner::FixedDistanceFlag ) dfunction = &DetectionCleaner::fixedDistance;
	if( m_options & DetectionCleaner::PhotonDistanceFlag ) dfunction = &DetectionCleaner::photonDistance;
	if( m_options & DetectionCleaner::PhotonBackgroundDistanceFlag ) dfunction = &DetectionCleaner::photonBackgroundDistance;

	/****** Computation of the parameters for the analysis *********/
	double * blinks = new double[_nbTime];
	memset( blinks, 0, _nbTime * sizeof( double ) );
	double * toffs = new double[_maxDarkTime];
	memset( toffs, 0, _maxDarkTime * sizeof( double ) );
	double * tons = new double[_nbTime];
	memset( tons, 0, _nbTime * sizeof( double ) );

	CleanerPoint ** tmpCPoints = new CleanerPoint *[_nbTotalData];
	int size = 0;
	int totalDetec = 0;
	for( int n = 0; n < _nbTotalData; n++ )
		_cpoints[n].m_done = false;
	for( int t = 0; t < _nbTime; t++ ){
		for( int n = _firstPointTime[t]; n < _firstPointTime[t] + _nbPointsTime[t]; n++ ){
			if( !_cpoints[n].m_done ){
				_cpoints[n].m_done = true;
				tmpCPoints[size++] = &_cpoints[n];
				int currentNeigh = 0, currentDarkTime = 0, timeN = t + 1, nbDetections = 1, nbBlinks = 0, nbOn = 1, tBleach = 1;
				double x = _cpoints[n].getPoint()->x(), y = _cpoints[n].getPoint()->y();
				while( currentDarkTime < _maxDarkTime && timeN < _nbTime ){
					bool neighFound = false;
					int index;
					double d = pow((this->*dfunction)(_cpoints[n].getIntensity(), m_hasSigma ? _cpoints[n].getSigma() : 0.), 2);
					for( int n2 = _firstPointTime[timeN]; n2 < _firstPointTime[timeN] + _nbPointsTime[timeN]; n2++ ){
						if( !_cpoints[n2].m_done ){
							double x2 = _cpoints[n2].getPoint()->x() - x, y2 = _cpoints[n2].getPoint()->y() - y;
							double length = x2 * x2 + y2 * y2;
							if( length < d ){
								neighFound = true;
								index = n2;
								d = length;
							}
						}
					}
					if( neighFound ){
						nbDetections++;
						nbOn++;
						tmpCPoints[size++] = &_cpoints[index];
						x = y = 0.;
						double dsize = size;
						for( int i = 0; i < size; i++ ){
							x += ( tmpCPoints[i]->getPoint()->x() / dsize );
							y += ( tmpCPoints[i]->getPoint()->y() / dsize );
						}
						_cpoints[index].m_done = true;
						if( currentDarkTime != 0 ){
							nbBlinks++;
							toffs[currentDarkTime]++;
						}
						currentDarkTime = 0;
					}
					else{
						currentDarkTime++;
						tons[nbOn]++;
						nbOn = 0;
						tBleach = timeN;
					}
					timeN++;
				}
				if( nbOn != 0 && nbOn < _nbTime ){
					tons[nbOn]++;
				}
				totalDetec += nbDetections;
				blinks[nbBlinks]++;
				size = 0;
			}
		}
	}

	/****************************** Fit for blinks *******************************/
	double totalBlinks = 0.;
	for( int i = 0; i < _nbTime; i++ )
		if( blinks[i] > 0 )
			totalBlinks += blinks[i];
	for( int n = 0; n < _maxDarkTime; n++ )
		blinks[n] = blinks[n] / totalBlinks;// * ( n + 1 );
	double * ts = new double[_maxDarkTime];
	for( int i = 0; i < _maxDarkTime; i++ )
		ts[i] = i;//+1;

	m_eqnBlinks = new EquationFit( ts, blinks, _maxDarkTime, EquationFit::LeeFunction );
	for( int i = 1; i < _maxDarkTime; i++ ){
		ts[i-1] = i;
		tons[i-1] = tons[i];
	}
	m_eqnTOns = new EquationFit( ts, tons, _maxDarkTime, EquationFit::ExpDecayValue );
	delete [] ts;
	ts = new double[_maxDarkTime-1];
	for( int i = 1; i < _maxDarkTime; i++ ){
		ts[i-1] = i;
		toffs[i-1] = toffs[i];
	}
	m_eqnTOffs = new EquationFit( ts, toffs, _maxDarkTime-1, EquationFit::ExpDecayHalLife );
	
	delete [] blinks;
	delete [] tons;
	delete [] toffs;
	delete [] ts;

	return ( 3 * m_eqnTOffs->getParams()[2] + 0.5 );
}

double DetectionCleaner::fixedDistance(double _photons, double _sigma)
{
	return m_sizeNeigh;
}

double DetectionCleaner::photonDistance(double _photons, double _sigma)
{
	return (2. * _sigma) / sqrt(m_ratioInt2Photon * _photons);
}

double DetectionCleaner::photonBackgroundDistance(double _photons, double _sigma)
{
	double term1 = (_sigma * _sigma + ((pow(m_pixelValue, 2) / 12)) / (m_ratioInt2Photon * _photons));
	double term2 = 4. * sqrt( M_PI ) * pow( _sigma, 3 ) * pow( m_background, 2 );
	term2 /= (m_pixelValue * pow((m_ratioInt2Photon * _photons), 2));
	return 2. * sqrt( term1 + term2 );
}

void DetectionCleaner::averagingPosition(Vec2md & _position, CleanerPoint ** _points, const int _nbPoints)
{
	double x = 0., y = 0., dsize = _nbPoints;
	for (int i = 0; i < _nbPoints; i++){
		x += (_points[i]->getPoint()->x() / dsize);
		y += (_points[i]->getPoint()->y() / dsize);
	}
	_position.set(x, y);
}