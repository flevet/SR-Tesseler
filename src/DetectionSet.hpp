/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      DetectionSet.hpp
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

#ifndef DetectionSet_h__
#define DetectionSet_h__

#include <iostream>
#include <vector>
#include "Vec3.hpp"
#include "Vec4.hpp"
#include "ObjectInterface.hpp"

class DetectionSet: public ObjectInterface{
public:
	DetectionSet();
	DetectionSet( const DetectionSet & );
	DetectionSet( const std::vector < DetectionSet * > & );
	DetectionSet(const std::vector< double > &, const std::vector< double > &, const std::vector< unsigned short > &, const std::vector< unsigned int > &, const int, const int);
	DetectionSet(double *, double *, unsigned short *, unsigned int *, const int, const int);
	virtual ~DetectionSet();

	//virtual void createTesselerFile( const char * filename );
	//virtual void createCSVFile(const char * filename);
	virtual const std::string & getDir() const;
	virtual const std::string & getName() const;
	virtual std::string type() const;
	virtual void save();

	virtual bool createFile(const char *);
	virtual void createTesselerFile(std::ifstream &, const char, std::vector < std::string > &);
	virtual void createSebastienFile(std::ifstream &, const char, std::vector < std::string > &);
	virtual void createOtherFileFormat(std::ifstream &, const char, std::vector < std::string > &);

	virtual DetectionSet * copy( DetectionSet * = NULL );
	virtual void setDir( const std::string & _dir );
	virtual void setName( const std::string & _name );

	virtual int nbPoints()const;
	virtual int nbSlices() const;

	virtual DetectionPoint * getPoints();
	virtual unsigned int * getFirstPoint();
	virtual unsigned int * getSizePoints();

	int size() const;
	DetectionPoint & operator[]( int );
	const DetectionPoint & operator[]( int ) const;
	bool isDataSelected( const int );
	void regenerateIntensityColorVector();


	inline double * getIntensities() const {return m_intensities;}
	inline double getIntensity( const int _idx ) const {return m_intensities[_idx];}
	inline double * getSigmas() const { return m_sigmas; }
	inline double getSigma(const int _idx) const { return m_sigmas[_idx]; }
	inline Color4D * getColors() const { return m_colors; }
	inline int getNbPoints() const {return m_nbPoints;}
	inline unsigned int * getFirsts() const { return m_firstsPoint; }
	inline unsigned int * getSizes() const { return m_sizePoints; }

	void getHistogramParameters( double &, double &, double &, double &, const int, const bool );
	double * getHistogram( const int, const bool ) const;
	void forceRegenerateSelection();
	void determineSelection( const bool = false );
	void resetDataSelection();

	void draw() const;

	void createDisplayPoints( const double, const double );

	bool isCleanable() const;

	void setColors(Color4D *);
	void colorLocsOfObject(unsigned int *, const int, const Color4D &);


	inline const float getWidth() const {return m_w;}
	inline const float getHeight() const {return m_h;}
	inline const bool hasSigmaPerLocalization() const { return m_sigmas != NULL; }

protected:
	std::string m_dir, m_name;
	float intensityMin, intensityMax, m_w, m_h;
	int m_nbPoints, m_nbSlices;

	DetectionPoint * m_points, * m_displayPoints;
	double * m_intensities, * m_sigmas;
	unsigned int * m_firstsPoint, * m_sizePoints;

	Color4D * m_colors;
};

#endif // DetectionSet_h__