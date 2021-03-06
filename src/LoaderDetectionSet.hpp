/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      LoaderDetectionSet.hpp
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

#ifndef LoaderDetectionSet_h__
#define LoaderDetectionSet_h__

#include <QString>

#include "DetectionSet.hpp"

class LoaderDetectionSet{
public:
	virtual ~LoaderDetectionSet(){}

	virtual DetectionSet * loadFile(const QString &) = 0;

	static DetectionSet * generateDetectionSetFromVector(const std::vector < DetectionSet * > &);
	static LoaderDetectionSet * getInstance(const QString &);

protected:
	LoaderDetectionSet(const QString & _separator) :m_separator(_separator){}

protected:
	QString m_separator;
};

class LoaderDetectionSetPALMTracer : public LoaderDetectionSet{
public:
	LoaderDetectionSetPALMTracer(const QString &);
	~LoaderDetectionSetPALMTracer();

	DetectionSet * loadFile( const QString & );
};

class LoaderDetectionSetPalmTracer2 : public LoaderDetectionSet{
public:
	LoaderDetectionSetPalmTracer2(const QString &);
	~LoaderDetectionSetPalmTracer2();

	DetectionSet * loadFile(const QString &);
};

#endif // LoaderDetectionSet_h__