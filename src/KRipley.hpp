/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      KRipley.hpp
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

#ifndef KRipley_h__
#define KRipley_h__

#include <string>

#include "DetectionSet.hpp"
#include "nanoflann.hpp"
#include "Roi.hpp"

class KRipley{
public:
	KRipley(DetectionSet *, const float, const float);
	KRipley(DetectionSet *, const double, const double, const double, const float, const float, const bool, const RoiList &);
	~KRipley();

	void computeKRipley(const double, const double, const double, const bool, const RoiList &);
	const double computeRipleyFunction( const double );

	void exportResults(const std::string &);

	const unsigned int getNbSteps() const { return m_nbSteps; }
	double * getKs() const { return m_ks; }
	double * getLs() const { return m_ls; }
	double * getTs() const { return m_ts; }

protected:
	const double edgeCorrection( const double, const double, const double );

protected:
	double m_minR, m_maxR, m_stepR, m_density, * m_results, m_w, m_h;
	DetectionSet * m_dset;
	KdPointCloud_D * m_cloud;
	KdTree_2D_double * m_tree;

	std::vector <DetectionPoint *> m_pointsInROIs;

	double * m_ks, *m_ls, * m_ts;
	unsigned int m_nbSteps;
};

#endif // KRipley_h__