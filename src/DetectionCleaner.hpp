/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      DetectionCleaner.hpp
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

#ifndef DetectionCleaner_h__
#define DetectionCleaner_h__

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <list>
#include <QString>

#include "GeneralTools.hpp"
#include "Vec2.hpp"
#include "Vec3.hpp"
#include "Vec4.hpp"

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point_2;
typedef CGAL::Polygon_2<K> Polygon_2;

class DetectionSet;

class CleanerPoint{
public:
	CleanerPoint();
	CleanerPoint(DetectionPoint *, const double, const double, const double);
	~CleanerPoint();

	void set(DetectionPoint *, const double, const double, const double);

	inline DetectionPoint * getPoint() const {return m_point;}
	inline double getT() const {return m_t;}
	inline double getIntensity() const{return m_intensity;}
	inline double getSigma() const{ return m_sigma; }

protected:
	DetectionPoint * m_point;
	double m_t, m_intensity, m_sigma;
	bool m_done;

	friend class DetectionCleaner;
	friend void quicksort( CleanerPoint **, double *, const int, const int );
};

class DetectionCleaner{
public:
	enum CleanerOptionFlags{ FixedDistanceFlag = 0x01, PhotonDistanceFlag = 0x02, PhotonBackgroundDistanceFlag = 0x04, FixedMaxDarkTimeFlag = 0x08 };

	DetectionCleaner(DetectionSet *, const double, const double, const double, const double, const int, const unsigned char, const std::string &);
	~DetectionCleaner();

	//void draw() const;
	inline int getMaxDarkTime() const {return m_maxDarkTime;}
	inline EquationFit * getEquationBlinks() const {return m_eqnBlinks;}
	inline EquationFit * getEquationTOns() const {return m_eqnTOns;}
	inline EquationFit * getEquationTOffs() const {return m_eqnTOffs;}
	inline const QString & getStats() const {return m_statsCleaner;}

	inline double * getXS() const {return m_xs;};
	inline double * getYS() const {return m_ys;};
	inline unsigned short * getTS() const {return m_ts;};
	inline unsigned int * getPhotons() const {return m_nbPhotons;};
	const int getNbClean() const {return m_nbTotalClean;}

	typedef double(DetectionCleaner::*DistanceFunction)(double, double);
	double fixedDistance(double, double);
	double photonDistance(double, double);
	double photonBackgroundDistance(double, double);

protected:
	void determineMaxDarkTimePaper( CleanerPoint *, const int, unsigned int *, unsigned int *, const int );
	void cleanDetectionSet( CleanerPoint *, const int, unsigned int *, unsigned int *, const int, CleanerPoint **, int &, Vec4md *, int &, const int );

	int computeNbEmissionBurst( CleanerPoint *, const int, unsigned int *, unsigned int *, const int );
	int computeAnalysisParameters( CleanerPoint *, const int, unsigned int *, unsigned int *, const int, const int );

	void averagingPosition(Vec2md &, CleanerPoint **, const int);

protected:
	double * m_xs, * m_ys;
	unsigned short * m_ts;
	unsigned int * m_nbPhotons;
	int m_nbTotalClean;
	bool m_toggleDisplay, m_debug, m_hasSigma;
	double m_totalRemoved, m_totalAdded, m_totalDetections;
	double m_sizeNeigh, m_pixelValue, m_background, m_ratioInt2Photon;

	EquationFit * m_eqnBlinks, * m_eqnTOns, * m_eqnTOffs;
	QString m_statsCleaner;

	int m_maxDarkTime, m_nbEmBurst;
	unsigned char m_options;
};

#endif // DetectionCleaner_h__