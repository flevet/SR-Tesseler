/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      Histogram.hpp
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

#ifndef Histogram2_h__
#define Histogram2_h__

#include <iostream>

class ObjectInterface;
class DetectionSet;
class WrapperVoronoiDiagram;
class VoronoiObject;

struct HistParam{
public:
	HistParam(){
		m_minH = m_maxH = m_stepX = m_maxY = m_currentMin = m_currentMax = 0.;
	}
	HistParam( const HistParam & _o ):m_minH(_o.m_minH), m_maxH(_o.m_maxH), m_stepX(_o.m_stepX), m_maxY(_o.m_maxY), m_currentMin(_o.m_currentMin), m_currentMax(_o.m_currentMax){

	}

public:
	double m_minH, m_maxH, m_stepX, m_maxY, m_currentMin, m_currentMax;


	friend std::ostream & operator<<(std::ostream &, const HistParam &);
};
class Histogram
{
public:
	Histogram();
	Histogram( ObjectInterface *, const short, const int );
	Histogram( const Histogram & );
	~Histogram();

	inline double getMin() const {return m_params[m_logOrNot].m_currentMin;}
	inline double getMax() const {return m_params[m_logOrNot].m_currentMax;}
	inline bool isLog() const {return m_logOrNot == LOG;}
	inline int getType() const {return m_type;}
	inline double * getValues() const { return m_values[m_logOrNot]; }
	void createHistogram( DetectionSet * );
	void createHistogram( WrapperVoronoiDiagram * );
	void createHistogram( VoronoiObject * );
	void setMin( const double _min );
	void setMax( const double _max );
	void setBounds( const double _min, const double _max );
	void getBounds( double & _min, double & _max );
	double getMinH() const;
	double getMaxH() const;
	double getMinY() const;
	double getStep() const;
	double * getHistogram();
	void setParameters( double &, double &, double &, double & );
	void eraseBounds();
	void resetBounds();
	void setLog( const int );

protected:
	struct HistParam m_params[2];//First for normal histogram, second for log histogram;
	double * m_values[2];
	short m_logOrNot;
	int m_type;

public:
	static int BINS, NORMAL, LOG;
};

#endif // Histogram2_h__