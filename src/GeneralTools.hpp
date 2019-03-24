/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      GeneralTools.hpp
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

#ifndef GeneralTools_h__
#define GeneralTools_h__

#include <iostream>
#include <QString>
#include <QTime>

class ImageViewer;

class EquationFit{
public:
	enum EquationType
	{
		LeeFunction = 0,
		ExpDecayHalLife = 1,
		ExpDecayValue = 2,
		DoubleGaussian = 3
	};

	EquationFit();
	EquationFit( double *, double *, const int, const int );
	~EquationFit();

	inline double * getTs() const {return m_ts;}
	inline double * getValues() const {return m_values;}
	inline double * getFitValues() const {return m_fitsValues;}
	inline double * getParams() const {return m_paramsEqn;}
	inline int getNbParam() const {return m_nbParamEqn;}
	inline int getNbTs() const {return m_nbTs;}
	inline int typeEqn() const {return m_typeEqn;}
	inline const QString & getEquation() const {return m_eqn;}

	void setEquation( double *, double *, const int, const int );

	double getFitValues( const double );

protected:
	double * m_values, * m_fitsValues, * m_paramsEqn, * m_ts;
	int m_nbParamEqn, m_nbTs, m_typeEqn;
	QString m_eqn;

	double ( *m_function )( double, const double * );
};

class ArrayStatistics{
public:
	ArrayStatistics():m_mean(0.),m_median(0.),m_stdDev(0.),m_min(0.),m_max(0.){}
	double m_mean, m_median, m_stdDev, m_max, m_min;

	friend std::ostream & operator<<( std::ostream &, const ArrayStatistics & );
};

class GeneralTools{
public:
	static ArrayStatistics generateArrayStatistics( double *, const int );
	static ArrayStatistics generateInverseArrayStatistics( double *, const int );

public:
	static ImageViewer * m_imw;
};

class MyTimer{
public:
	MyTimer();
	~MyTimer();

	const QString getTimeElapsed();

protected:
	QTime m_time;
};

#endif // GeneralTools_h__