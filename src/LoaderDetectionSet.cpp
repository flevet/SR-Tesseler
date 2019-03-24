/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      LoaderDetectionSet.cpp
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
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "LoaderDetectionSet.hpp"
#include "GeneralTools.hpp"

DetectionSet * LoaderDetectionSet::generateDetectionSetFromVector(const std::vector < DetectionSet * > & _detections)
{
	return new DetectionSet(_detections);
}

LoaderDetectionSet * LoaderDetectionSet::getInstance(const QString & _filename)
{
	std::ifstream fs;
	fs.open(_filename.toAscii().data());
	std::string s;
	std::getline(fs, s);
	QString firstLine(s.c_str());
	QString separator;
	if (firstLine.contains('\t'))
		separator = '\t';
	else if (firstLine.contains(','))
		separator = ',';
	else if (firstLine.contains(' '))
		separator = ' ';
	if (separator.isEmpty())
		return NULL;
	if (_filename.endsWith(".txt")){
		QStringList words = firstLine.split(separator);
		unsigned int nbWords = 0;
		for (unsigned int n = 0; n < words.size(); n++)
			if (!words.at(n).isEmpty())
				nbWords++;
		if (firstLine.startsWith("Width")){
			bool palTracer2Dataset = firstLine.contains("Spectral");
			fs.close();
			if (palTracer2Dataset)
				return new LoaderDetectionSetPalmTracer2(separator);
		}
		else if (firstLine.startsWith("2D")){
			fs.close();
			return new LoaderDetectionSetPALMTracer(separator);
		}
		fs.close();
		return NULL;
	}
	fs.close();
	return NULL;
}

LoaderDetectionSetPALMTracer::LoaderDetectionSetPALMTracer(const QString & _separator) :LoaderDetectionSet(_separator)
{
}

LoaderDetectionSetPALMTracer::~LoaderDetectionSetPALMTracer()
{

}

DetectionSet * LoaderDetectionSetPALMTracer::loadFile( const QString & _filename )
{
	std::vector < DetectionPoint > points;
	std::vector < double > xs, ys;
	std::vector <unsigned int> intensities;
	std::vector < unsigned short > times;
	int nbPoints = 0, nbSlices = 0;

	std::ifstream fs;
	fs.open( _filename.toAscii().data() );

	double index2, mean, surface, perimetrer, morpho, maxX, maxY, chi2, y, x, z, binaryY, sigmaX, sigmaY, angleRad, AngleDeg, intensityGauss, intensity;
	std::string s;
	std::getline( fs,s );
	std::istringstream is( s );
	std::string tmp;
	for( int i = 0; i < 4; i++ ){
		is >> tmp;
	}
	is >> nbSlices;

	double xmin = DBL_MAX, xmax = DBL_MIN, ymin = DBL_MAX, ymax = DBL_MIN;

	for(int n = 0; n < nbSlices; n++){
		std::getline(fs,s);
		std::istringstream is2(s);
		std::string tmp;
		for(int i = 0; i < 11; i++){
			is2 >> tmp;
		}
		double nbDetec;
		is2 >> nbDetec;

		std::getline( fs, s );
		for(int j = 0; j < nbDetec; j++){
			std::getline( fs, s );
			std::istringstream is3( s );
			is3 >> index2 >> mean >> surface >> intensity >> perimetrer >> morpho >> maxX >> intensityGauss >> chi2 >> y >> x >> z >> binaryY >> sigmaX >> sigmaY >> angleRad >> AngleDeg;

			//if( chi2 > 0.7 )
			{
				//in order to avoid that the correction gives coordinates < 0 or > w | h
				x += 2.;
				y += 2.;

				if( x < xmin ) xmin = x;
				if( x > xmax ) xmax = x;
				if( y < ymin ) ymin = y;
				if( y > ymax ) ymax = y;

				points.push_back( DetectionPoint( x, y, /*( z + 1.75 ) * 10000. )*/z ) );
				xs.push_back(x);
				ys.push_back(y);
				if( intensityGauss > 0. ){
					intensities.push_back( (unsigned int)intensityGauss );
				}
				else
					intensities.push_back((unsigned int)intensity);
				times.push_back( (unsigned short)n );
				nbPoints++;
			}
		}
	}
	fs.close();
	DetectionSet * dset = new DetectionSet( xs, ys, times, intensities, nbSlices, xs.size() );
	return dset;
}

LoaderDetectionSetPalmTracer2::LoaderDetectionSetPalmTracer2(const QString & _separator) :LoaderDetectionSet(_separator)
{

}

LoaderDetectionSetPalmTracer2::~LoaderDetectionSetPalmTracer2()
{

}

DetectionSet * LoaderDetectionSetPalmTracer2::loadFile(const QString & _filename)
{
	std::vector < double > xs, ys;
	std::vector < unsigned int > intensities;
	std::vector < unsigned short > times;
	int w, h, nbPoints = 0, nbSlices = 0;
	double calXY, calTime;
	unsigned int currentLine = 0;

	std::ifstream fs;
	fs.open(_filename.toAscii().data());

	std::string s;
	std::getline(fs, s);
	std::getline(fs, s);
	std::istringstream is(s);
	is >> w >> h >> nbSlices >> nbPoints >> calXY >> calTime;

	std::getline(fs, s);
	double plane, index, x, y, z, mse, sigmaX, sigmaY, id, channel, integratedInt, angle, mseZ, pairD;

	double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX, ymax = -DBL_MAX, zmin = DBL_MAX, zmax = -DBL_MAX;

	currentLine += 3;
	int n = 0;
	//for (int n = 0; n < nbPoints; n++){
	while (std::getline(fs, s) && n < nbPoints){
		//std::getline(fs, s);
		std::istringstream is2(s);

		is2 >> id >> plane >> index >> channel >> integratedInt >> x >> y >> sigmaX >> sigmaY >> angle >> mse >> z >> mseZ >> pairD;
		z *= 1000.;
		z /= 160;
		if (x < xmin) xmin = x;
		if (x > xmax) xmax = x;
		if (y < ymin) ymin = y;
		if (y > ymax) ymax = y;
		if (z < zmin) zmin = z;
		if (z > zmax) zmax = z;

		xs.push_back(x);
		ys.push_back(y);
		if (integratedInt > 0.){
			intensities.push_back(integratedInt);
		}
		else
			intensities.push_back(integratedInt);
		times.push_back((unsigned short)(plane - 1));
		n++;
	}
	xs.resize(n);
	xs.shrink_to_fit();
	ys.resize(n);
	ys.shrink_to_fit();
	intensities.resize(n);
	intensities.shrink_to_fit();
	times.resize(n);
	times.shrink_to_fit();

	fs.close();

	DetectionSet * dset = new DetectionSet(xs, ys, times, intensities, nbSlices, xs.size());
	return dset;
}