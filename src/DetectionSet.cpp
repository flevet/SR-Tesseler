/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      DetectionSet.cpp
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

#include <Windows.h>
#include <GL/gl.h>
#include <float.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <random>
#include <cmath>

#include "DetectionSet.hpp"
#include "ImageViewer.hpp"

std::vector < std::string > & split(const std::string & s, char delim, std::vector < std::string > & elems){
	std::stringstream ss(s);
	std::string item;
	int cpt = 0;
	while (std::getline(ss, item, delim)){
		if (!item.empty())
			elems.push_back( item );
		//elems[cpt++] = item;
	}
	return elems;
}

DetectionSet::DetectionSet():ObjectInterface(),m_w(0), m_h(0)
{
	m_points = m_displayPoints = NULL;
	m_firstsPoint = m_sizePoints = NULL;
	m_intensities = m_sigmas = NULL;
	m_colors = NULL;
}

DetectionSet::DetectionSet( const DetectionSet & o ):ObjectInterface( o ), m_dir(o.m_dir), m_name(o.m_name), intensityMin(o.intensityMin), intensityMax(o.intensityMax), m_nbPoints( o.m_nbPoints ), m_nbSlices( o.m_nbSlices ), m_w(o.m_w), m_h(o.m_h)
{
	m_points = m_displayPoints = NULL;
	m_firstsPoint = m_sizePoints = NULL;
	m_intensities = m_sigmas = NULL;
	m_colors = NULL;

	m_firstsPoint = new unsigned int[m_nbSlices];
	m_sizePoints = new unsigned int[m_nbSlices];
	m_points = new DetectionPoint[m_nbPoints];
	memcpy( m_points, o.m_points, m_nbPoints * sizeof( DetectionPoint ) );
	m_intensities = new double[m_nbPoints];
	memcpy(m_intensities, o.m_intensities, m_nbPoints * sizeof(double));
	if (o.m_sigmas != NULL){
		m_sigmas = new double[m_nbPoints];
		memcpy(m_sigmas, o.m_sigmas, m_nbPoints * sizeof(double));
	}
	else
		m_sigmas = NULL;
	m_stats = new ArrayStatistics[1];
	m_stats[0] = ArrayStatistics( o.m_stats[0] );
}

DetectionSet::DetectionSet( const std::vector < DetectionSet * > & vect ):ObjectInterface()
{
	m_points = m_displayPoints = NULL;
	m_firstsPoint = m_sizePoints = NULL;
	m_intensities = m_sigmas = NULL;
	m_colors = NULL;

	m_nbPoints = 0;
	m_nbSlices = 0;
	m_w = m_h = 0.f;
	bool hasSignmas = true;
	for( std::vector < DetectionSet * >::const_iterator it2 = vect.begin(); it2 != vect.end(); it2++ ){
		DetectionSet * o = *it2;
		m_nbPoints += o->m_nbPoints;
		m_nbSlices += o->m_nbSlices;
		if( o->m_w > m_w )
			m_w = o->m_w;
		if( o->m_h > m_h )
			m_h = o->m_h;
		hasSignmas = hasSignmas & o->m_sigmas != NULL;
	}
	m_firstsPoint = new unsigned int[m_nbSlices];
	m_sizePoints = new unsigned int[m_nbSlices];
	m_points = new DetectionPoint[m_nbPoints];
	m_intensities = new double[m_nbPoints];
	if (hasSignmas)
		m_sigmas = new double[m_nbPoints];
	else
		m_sigmas = NULL;

	unsigned int * ptrFirst = m_firstsPoint, * ptrSize = m_sizePoints;
	DetectionPoint * ptrP = m_points;
	double * ptrI = m_intensities, * ptrS = m_sigmas;
	int addingForStart = 0;
	for( std::vector < DetectionSet * >::const_iterator it2 = vect.begin(); it2 != vect.end(); it2++ ){
		DetectionSet * o = *it2;
		for( int n = 0; n < o->m_nbSlices; n++ )
			( *ptrFirst++ ) = addingForStart + o->m_firstsPoint[n];
		memcpy( ptrSize, o->m_sizePoints, o->m_nbSlices * sizeof( unsigned int ) );
		memcpy( ptrP, o->m_points, o->m_nbPoints * sizeof( DetectionPoint ) );
		memcpy( ptrI, o->m_intensities, o->m_nbPoints * sizeof( double ) );
		ptrSize += o->m_nbSlices;
		ptrP += o->m_nbPoints;
		ptrI += o->m_nbPoints;
		if (hasSignmas){
			memcpy(ptrS, o->m_sigmas, o->m_nbPoints * sizeof(double));
			ptrS += o->m_nbPoints;
		}
		addingForStart += o->m_firstsPoint[o->m_nbSlices - 1] + o->m_sizePoints[o->m_nbSlices - 1];
	}

	m_stats = new ArrayStatistics[1];
	m_stats[0] = GeneralTools::generateArrayStatistics( m_intensities, m_nbPoints );

	m_nbHisto = 1;
	m_histograms = new Histogram *[m_nbHisto];
	m_histograms[0] = NULL;
	computeHistograms();
	m_palette = Palette::getStaticLut( "InvFire" );
	m_colors = new Color4D[m_nbPoints];
	m_selection = new bool[m_nbPoints];
	forceRegenerateSelection();
}

DetectionSet::DetectionSet(const std::vector< double > & _xs, const std::vector< double > & _ys, const std::vector< unsigned short > & _ts, const std::vector< unsigned int > & _photons, const int _nbSlices, const int _nbPoints):ObjectInterface()
{
	m_points = m_displayPoints = NULL;
	m_firstsPoint = m_sizePoints = NULL;
	m_intensities = m_sigmas = NULL;
	m_colors = NULL;

	intensityMin = FLT_MAX;
	intensityMax = FLT_MIN;
	m_nbPoints = _nbPoints;
	m_nbSlices = _nbSlices;

	m_firstsPoint = new unsigned int[m_nbSlices];
	m_sizePoints = new unsigned int[m_nbSlices];
	m_points = new DetectionPoint[m_nbPoints];
	m_intensities = new double[m_nbPoints];
	m_sigmas = NULL;

	memset(m_firstsPoint, 0, m_nbSlices * sizeof(unsigned int));
	memset(m_sizePoints, 0, m_nbSlices * sizeof(unsigned int));

	for (int n = 0; n < m_nbPoints; n++){
		unsigned int currentT = _ts[n];
		m_sizePoints[currentT]++;
	}
	unsigned int * tmpCount = new unsigned int[m_nbSlices];
	memset(tmpCount, 0, m_nbSlices * sizeof(unsigned int));
	int cpt = 0;
	for (int n = 0; n < m_nbSlices; n++){
		m_firstsPoint[n] = cpt;
		cpt += m_sizePoints[n];
	}
	m_w = m_h = 0;
	for (int n = 0; n < m_nbPoints; n++){
		unsigned int currentT = _ts[n];
		m_points[m_firstsPoint[currentT] + tmpCount[currentT]].set(_xs[n], _ys[n], 0.);
		tmpCount[currentT]++;
		m_intensities[n] = _photons[n];
		if (m_intensities[n] < intensityMin)
			intensityMin = m_intensities[n];
		if (m_intensities[n] > intensityMax)
			intensityMax = m_intensities[n];
		if (_xs[n] > m_w)
			m_w = _xs[n];
		if (_ys[n] > m_h)
			m_h = _ys[n];
	}
	m_stats = new ArrayStatistics[1];
	m_stats[0] = GeneralTools::generateArrayStatistics(m_intensities, m_nbPoints);

	m_nbHisto = 1;
	m_histograms = new Histogram *[m_nbHisto];
	m_histograms[0] = NULL;
	computeHistograms();
	m_palette = Palette::getStaticLut("AllGreen");
	m_colors = new Color4D[m_nbPoints];
	m_selection = new bool[m_nbPoints];
	forceRegenerateSelection();
}


DetectionSet::DetectionSet( double * _xs, double * _ys, unsigned short * _ts, unsigned int * _photons, const int _nbSlices, const int _nbPoints ):ObjectInterface()
{
	m_points = m_displayPoints = NULL;
	m_firstsPoint = m_sizePoints = NULL;
	m_intensities = m_sigmas = NULL;
	m_colors = NULL;

	intensityMin = FLT_MAX;
	intensityMax = FLT_MIN;
	m_nbPoints = _nbPoints;
	m_nbSlices = _nbSlices;

	m_firstsPoint = new unsigned int[m_nbSlices];
	m_sizePoints = new unsigned int[m_nbSlices];
	m_points = new DetectionPoint[m_nbPoints];
	m_intensities = new double[m_nbPoints];
	m_sigmas = NULL;

	memset( m_firstsPoint, 0, m_nbSlices * sizeof( unsigned int ) );
	memset( m_sizePoints, 0, m_nbSlices * sizeof( unsigned int ) );

	for( int n = 0; n < m_nbPoints; n++ ){
		unsigned int currentT = _ts[n];
		m_sizePoints[currentT]++;
	}
	unsigned int * tmpCount = new unsigned int[m_nbSlices];
	memset( tmpCount, 0, m_nbSlices * sizeof( unsigned int ) );
	int cpt = 0;
	for( int n = 0; n < m_nbSlices; n++ ){
		m_firstsPoint[n] = cpt;
		cpt += m_sizePoints[n];
	}
	m_w = m_h = 0;
	for( int n = 0; n < m_nbPoints; n++ ){
		unsigned int currentT = _ts[n];
		m_points[m_firstsPoint[currentT] + tmpCount[currentT]].set( _xs[n], _ys[n], 0. );
		tmpCount[currentT]++;
		m_intensities[n] = _photons[n];
		if( m_intensities[n] < intensityMin )
			intensityMin = m_intensities[n];
		if( m_intensities[n] > intensityMax )
			intensityMax = m_intensities[n];
		if( _xs[n] > m_w )
			m_w = _xs[n];
		if( _ys[n] > m_h )
			m_h = _ys[n];
	}
	m_stats = new ArrayStatistics[1];
	m_stats[0] = GeneralTools::generateArrayStatistics( m_intensities, m_nbPoints );

	m_nbHisto = 1;
	m_histograms = new Histogram *[m_nbHisto];
	m_histograms[0] = NULL;
	computeHistograms();
	m_palette = Palette::getStaticLut( "AllGreen" );
	m_colors = new Color4D[m_nbPoints];
	m_selection = new bool[m_nbPoints];
	forceRegenerateSelection();
}

DetectionSet::~DetectionSet()
{
	if( m_points != NULL )
		delete [] m_points;
	if( m_firstsPoint != NULL )
		delete [] m_firstsPoint;
	if( m_sizePoints != NULL )
		delete [] m_sizePoints;
	if( m_colors != NULL )
		delete [] m_colors;
	if( m_selection != NULL )
		delete [] m_selection;
	if( m_intensities != NULL )
		delete [] m_intensities;
	if( m_stats != NULL )
		delete [] m_stats;
	if( m_displayPoints != NULL )
		delete [] m_displayPoints;
}
/*
void DetectionSet::createTesselerFile(const char * filename)
{
	m_dir = std::string( filename );
	m_name = std::string( filename );
	int index = m_dir.find_last_of( "/" );
	m_dir.erase( index, m_dir.size()-index );
	m_dir.append( "/" );
	m_name.erase( 0, index + 1 );

	intensityMin = FLT_MAX;
	intensityMax = FLT_MIN;
	m_nbPoints = 0;

	std::vector < std::string > headers, values;
	std::ifstream fs( filename );

	std::string s;
	std::getline( fs,s );
	headers = split(s, ' ', headers);
	if (headers.size() != 2){
		std::cout << filename << " is not a correct localization file" << std::endl;
		return;
	}
	//std::istringstream is( s );
	//is >> m_nbSlices >> m_nbPoints;
	m_nbSlices = atoi(headers[0].c_str());
	m_nbPoints = atoi(headers[1].c_str());
	m_firstsPoint = new unsigned int[m_nbSlices];
	m_sizePoints = new unsigned int[m_nbSlices];
	m_intensities = new double[m_nbPoints];
	memset(m_firstsPoint, 0, m_nbSlices * sizeof(unsigned int));
	memset( m_sizePoints, 0, m_nbSlices * sizeof( unsigned int ) );
	m_points = new DetectionPoint[m_nbPoints];
	std::vector < double > sigmas;
	double x, y, intensity, sigma;
	int t;

	unsigned int cpt = 0;
	GeneralTools::m_imw->m_progress->setMaximum( m_nbPoints );
	GeneralTools::m_imw->m_progress->setValue( cpt++ );

	unsigned int cptt = 0;
	for( int n = 0; n < m_nbPoints; n++, cptt++ ){
		GeneralTools::m_imw->m_progress->setValue( cpt++ );
		std::getline(fs, s);

		//std::cout << s.c_str() << std::endl;
		if (s.empty()) continue;

		std::vector < std::string > values;
		values = split(s, ' ', values);
		x = ::atof(values[0].c_str());
		y = ::atof(values[1].c_str());
		intensity = ::atof(values[2].c_str());
		t = ::atoi(values[3].c_str());
		if (values.size() == 5)
			sigmas.push_back(::atof(values[4].c_str()));

		//fs >> x >> y >> intensity >> t;
		m_points[n].set( x, y, 0 );
		m_intensities[n] = intensity;
		m_sizePoints[t]++;
		if( x > m_w )
			m_w = x;
		if( y > m_h )
			m_h = y;
	}
	for( int n = 1; n < m_nbSlices; n++ )
		m_firstsPoint[n] = m_firstsPoint[n-1] + m_sizePoints[n-1];
	if (!sigmas.empty()){
		m_sigmas = new double[m_nbPoints];
		for (int n = 0; n < m_nbPoints; n++)
			m_sigmas[n] = sigmas[n];
	}

	m_stats = new ArrayStatistics[1];
	m_stats[0] = GeneralTools::generateArrayStatistics( m_intensities, m_nbPoints );

	m_nbHisto = 1;
	m_histograms = new Histogram *[m_nbHisto];
	m_histograms[0] = NULL;
	computeHistograms();
	m_palette = Palette::getStaticLut( "InvFire" );
	if( m_colors != NULL )
		delete [] m_colors;
	m_colors = new Color4D[m_nbPoints];
	m_selection = new bool[m_nbPoints];
	regenerateIntensityColorVector();

}*/

/*void DetectionSet::createCSVFile(const char * filename)
{
	m_dir = std::string(filename);
	m_name = std::string(filename);
	int index = m_dir.find_last_of("/");
	m_dir.erase(index, m_dir.size() - index);
	m_dir.append("/");
	m_name.erase(0, index + 1);

	intensityMin = FLT_MAX;
	intensityMax = FLT_MIN;
	m_nbPoints = 0;

	std::vector < std::string > headers;
	std::string s;
	std::ifstream fs(filename);
	std::getline(fs, s);
	headers = split(s, ',', headers);
	int indexXs, indexYs, indexFrames, indexIntensities, indexSigmas;
	indexXs = indexYs = indexFrames = indexIntensities = indexSigmas  = - 1;
	for (unsigned int n = 0; n < headers.size(); n++){
		std::string current = headers.at(n);
		current.erase(std::remove(current.begin(), current.end(), '"'), current.end());
		if (current.substr(0, std::strlen("x")) == "x")
			indexXs = n;
		else if (current.substr(0, std::strlen("y")) == "y")
			indexYs = n;
		else if (current.substr(0, std::strlen("intensity")) == "intensity")
			indexIntensities = n;
		else if (current.substr(0, std::strlen("frame")) == "frame")
			indexFrames = n;
		else if (current.substr(0, std::strlen("sigma")) == "sigma")
			indexSigmas = n;
	}
	if (indexXs == -1 || indexYs == -1 || indexFrames == -1 || indexIntensities == -1){
		std::cout << "Unable to open csv file, missing " << std::endl;
		if (indexXs == -1) std::cout << "x ";
		if (indexYs == -1) std::cout << "y ";
		if (indexFrames == -1) std::cout << "frame ";
		if (indexIntensities == -1) std::cout << "intensity ";
		std::cout << "parameter(s)" << std::endl;
		fs.close();
		return;
	}

	std::vector < DetectionPoint > points;
	std::vector < double > intensities, sigmas;
	std::vector < unsigned int > times;
	double x, y, ii;
	int currentFrame;

	while (std::getline(fs, s)){
		std::vector < std::string > values;
		values = split(s, ',', values);
		currentFrame = ::atoi(values[indexFrames].c_str()) - 1;
		x = ::atof(values[indexXs].c_str());
		y = ::atof(values[indexYs].c_str());
		ii = ::atof(values[indexIntensities].c_str());
		if (ii > intensityMax)
			intensityMax = ii;
		if (ii < intensityMin)
			intensityMin = ii;
		points.push_back(DetectionPoint(x, y, 0.));
		intensities.push_back(ii);
		times.push_back(currentFrame);
		if (x > m_w)
			m_w = x;
		if (y > m_h)
			m_h = y;
		if (indexSigmas != -1)
			sigmas.push_back(::atof(values[indexSigmas].c_str()));
	}

	m_nbPoints = points.size();
	m_nbSlices = times.back() + 1;

	m_firstsPoint = new unsigned int[m_nbSlices];
	m_sizePoints = new unsigned int[m_nbSlices];
	m_intensities = new double[m_nbPoints];
	if (indexSigmas != -1)
		m_sigmas = new double[m_nbPoints];
	else
		m_sigmas = NULL;
	memset(m_firstsPoint, 0, m_nbSlices * sizeof(bool));
	memset(m_sizePoints, 0, m_nbSlices * sizeof(unsigned int));
	m_points = new DetectionPoint[m_nbPoints];


	for (int n = 0; n < m_nbPoints; n++){
		m_points[n].set(points[n].x(), points[n].y(), 0);
		m_intensities[n] = intensities[n];
		m_sizePoints[times[n]]++;
		if (m_sigmas != NULL) m_sigmas[n] = sigmas[n];
	}
	for (int n = 1; n < m_nbSlices; n++)
		m_firstsPoint[n] = m_firstsPoint[n - 1] + m_sizePoints[n - 1];

	m_stats = new ArrayStatistics[1];
	m_stats[0] = GeneralTools::generateArrayStatistics(m_intensities, m_nbPoints);

	m_nbHisto = 1;
	m_histograms = new Histogram *[m_nbHisto];
	m_histograms[0] = NULL;
	computeHistograms();
	m_palette = Palette::getStaticLut("InvFire");
	if (m_colors != NULL)
		delete[] m_colors;
	m_colors = new Color4D[m_nbPoints];
	m_selection = new bool[m_nbPoints];
	regenerateIntensityColorVector();
}*/
/*
void DetectionSet::createCSVFile(const char * filename)
{
	m_dir = std::string(filename);
	m_name = std::string(filename);
	int index = m_dir.find_last_of("/");
	m_dir.erase(index, m_dir.size() - index);
	m_dir.append("/");
	m_name.erase(0, index + 1);

	intensityMin = FLT_MAX;
	intensityMax = FLT_MIN;
	m_nbPoints = 0;

	std::ifstream fs(filename);
	std::stringstream buffer;
	buffer << fs.rdbuf();
	fs.close();

	QString fileText(buffer.str().c_str());
	QStringList lines = fileText.split("\n");

	QStringList headers = lines.at(0).split(",");
	int indexXs, indexYs, indexFrames, indexIntensities;
	indexXs = indexYs = indexFrames = indexIntensities = -1;
	for (unsigned int n = 0; n < headers.size(); n++){
		QString current = headers.at(n);
		current.replace("\"", "");
		if (current.startsWith("x"))
			indexXs = n;
		if (current.startsWith("y"))
			indexYs = n;
		if (current.startsWith("intensity"))
			indexIntensities = n;
		if (current.startsWith("frame"))
			indexFrames = n;
	}
	if (indexXs == -1 || indexYs == -1 || indexFrames == -1 || indexIntensities == -1){
		std::cout << "Trying to open an uncorrect csv file" << std::endl;
		return;
	}
	m_nbPoints = lines.size() - 1;
	m_nbSlices = lines.back().split(",").at(indexFrames).toUInt();

	m_firstsPoint = new unsigned int[m_nbSlices];
	m_sizePoints = new unsigned int[m_nbSlices];
	m_intensities = new double[m_nbPoints];
	memset(m_firstsPoint, 0, m_nbSlices * sizeof(bool));
	memset(m_sizePoints, 0, m_nbSlices * sizeof(unsigned int));
	m_points = new DetectionPoint[m_nbPoints];
	double x, y, intensity;
	int t;

	unsigned int cpt = 0;
	GeneralTools::m_imw->m_progress->setMaximum(m_nbPoints);
	GeneralTools::m_imw->m_progress->setValue(cpt++);

	for (int n = 0; n < m_nbPoints; n++){
		GeneralTools::m_imw->m_progress->setValue(cpt++);
		QStringList line = lines.at(n + 1).split(",");
		m_points[n].set(line.at(indexXs).toDouble(), line.at(indexYs).toDouble(), 0);
		m_intensities[n] = line.at(indexIntensities).toDouble();
		unsigned int t = (unsigned int)line.at(indexFrames).toDouble();
		m_sizePoints[t-1]++;
		if (x > m_w)
			m_w = x;
		if (y > m_h)
			m_h = y;
	}
	for (int n = 1; n < m_nbSlices; n++)
		m_firstsPoint[n] = m_firstsPoint[n - 1] + m_sizePoints[n - 1];

	m_stats = new ArrayStatistics[1];
	m_stats[0] = GeneralTools::generateArrayStatistics(m_intensities, m_nbPoints);

	m_nbHisto = 1;
	m_histograms = new Histogram *[m_nbHisto];
	m_histograms[0] = NULL;
	computeHistograms();
	m_palette = Palette::getStaticLut("InvFire");
	if (m_colors != NULL)
		delete[] m_colors;
	m_colors = new Color4D[m_nbPoints];
	m_selection = new bool[m_nbPoints];
	regenerateIntensityColorVector();
}
*/
const std::string & DetectionSet::getDir() const
{
	return m_dir;
}

const std::string & DetectionSet::getName() const
{
	return m_name;
}

std::string DetectionSet::type() const
{
	return std::string( "txt" );
}

void DetectionSet::save()
{
	/*const QString initialPath = dir + name;
    const QString fileName = QFileDialog::getSaveFileName(0, QString("Save As"),
                                                          initialPath);
	if (fileName.isEmpty())
		return;

	ofstream fs(fileName.toAscii().data());
	for(unsigned int i = 0; i < this->size(); i++){
		Vec3md * v = (*this)[i];
		fs << v->z() << " " << v->x() << " " << v->y() << endl;
	}
	fs.close();*/
}

DetectionSet * DetectionSet::copy( DetectionSet * orig )
{
	if(orig == NULL)
		return new DetectionSet();
	DetectionSet * set = NULL;
	DetectionSet * input = dynamic_cast< DetectionSet * >(orig);
	if(input)
		set = new DetectionSet( *input );
	return set;
}

void DetectionSet::setDir( const std::string & _dir )
{
	m_dir = _dir;
}

void DetectionSet::setName( const std::string & _name )
{
	m_name = _name;
}

int DetectionSet::nbPoints() const
{
	return m_nbPoints;
}

int DetectionSet::nbSlices() const
{
	return m_nbSlices;
}

DetectionPoint * DetectionSet::getPoints()
{
	return m_points;
}

unsigned int * DetectionSet::getFirstPoint()
{
	return m_firstsPoint;
}

unsigned int * DetectionSet::getSizePoints(){
	return m_sizePoints;
}

int DetectionSet::size() const
{
	return m_nbPoints;
}

DetectionPoint & DetectionSet::operator[]( int _idx )
{
	return m_points[_idx];
}

const DetectionPoint & DetectionSet::operator[]( int _idx ) const
{
	return m_points[_idx];
}

bool DetectionSet::isDataSelected( const int )
{
	return true;
}

void DetectionSet::regenerateIntensityColorVector()
{
	Color4D * ptrC;
	ptrC = m_colors;
	long current = 0;
	double minI = m_histograms[0]->getMinH(), inter = m_histograms[0]->getMaxH() - minI;
	double sizeImage = 500.;
	bool logHist = m_histograms[0]->isLog();
	for(unsigned int i = 0; i < m_nbPoints; i++){
		double val = ( logHist ) ? MiscFunction::log10Custom( m_intensities[i] ) : m_intensities[i];
		val = ( val - minI ) / inter;
		QColor color_tmp = m_palette->getColor( val );
		if( m_selection[i] )
			(*ptrC++).set( color_tmp.redF(), color_tmp.greenF(), color_tmp.blueF(), color_tmp.alphaF() );
		else
			(*ptrC++).set( color_tmp.redF(), color_tmp.greenF(), color_tmp.blueF(), 0.f );
	}
}

void DetectionSet::getHistogramParameters( double & _minH, double & _maxH, double & _stepX, double & _maxY, const int _typeHistogram, const bool _isLog )
{
	switch ( _typeHistogram )
	{
	case IntensityHistogram:
		{
			m_histograms[0]->setParameters( _minH, _maxH, _stepX, _maxY );
			break;
		}
	default:
		{
			_minH = _maxH = _stepX = _maxY = 0.;
			break;
		}
	}
}

double * DetectionSet::getHistogram( const int _typeHistogram, const bool _isLog ) const
{
	switch ( _typeHistogram )
	{
	case IntensityHistogram:
		{
			return m_histograms[0]->getHistogram();
			break;
		}
	}
	return NULL;
}
void DetectionSet::determineSelection( const bool resetSelectionByUser )
{
	m_nbSelection = 0;
	if( resetSelectionByUser )
		for( int i = 0; i < m_nbHisto; i++ )
			m_histograms[i]->eraseBounds();
	uint current = 0;
	resetDataSelection();
	for(unsigned int i = 0; i < m_nbPoints; i++)
	{
			double inten = ( m_histograms[0]->isLog() ) ? MiscFunction::log10Custom( m_intensities[i] ) : m_intensities[i];
			m_selection[i] = ( m_histograms[0]->getMin() <= inten && inten <= m_histograms[0]->getMax() );
			if( m_selection[i] )
				m_nbSelection++;
	}
}

void DetectionSet::resetDataSelection()
{
	for(uint i = 0; i < m_nbPoints; i++)
		m_selection[i] = false;
}

void DetectionSet::forceRegenerateSelection()
{
	determineSelection();
	regenerateIntensityColorVector();
}

void DetectionSet::draw() const
{
	if( !m_selected || m_displayPoints == NULL ) return;
	glPushMatrix();
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_COLOR_ARRAY );
	glVertexPointer( 3, GL_DOUBLE, 0, m_displayPoints );
	glColorPointer( 4, GL_FLOAT, 0, m_colors );
	glDrawArrays( GL_POINTS, 0, m_nbPoints );
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
	glDisable(GL_BLEND);
	glPopMatrix();
}

void DetectionSet::createDisplayPoints( const double _w, const double _h )
{
	m_displayPoints = new DetectionPoint[m_nbPoints];
	for( int n = 0; n < m_nbPoints; n++ )
		m_displayPoints[n].set( m_points[n].x() / _w, m_points[n].y() / _h, m_points[n].z() );
}

bool DetectionSet::isCleanable() const
{
	return m_nbSlices > 2;
}

bool DetectionSet::createFile(const char * _filename)
{
	m_dir = std::string(_filename);
	m_name = std::string(_filename);
	int index = m_dir.find_last_of("/");
	m_dir.erase(index, m_dir.size() - index);
	m_dir.append("/");
	m_name.erase(0, index + 1);

	intensityMin = FLT_MAX;
	intensityMax = FLT_MIN;
	m_nbPoints = 0;

	std::vector < std::string > headers, values;
	std::ifstream fs(_filename);

	std::string s;
	std::getline(fs, s);
	char * separator = NULL;
	std::size_t found = s.find('\t');
	if (found != std::string::npos)
		separator = new char('\t');
	else{
		found = s.find(',');
		if (found != std::string::npos)
			separator = new char(',');
		else{
			found = s.find(' ');
			if (found != std::string::npos)
				separator = new char(' ');
		}
	}
	if (separator == NULL){
		std::cout << "Impossible to open " << _filename << std::endl;
		return false;
	}
	headers = split(s, *separator, headers);
	if (headers.size() == 2)
		createTesselerFile(fs, *separator, headers);
	else if (strncmp(s.c_str(), "Total", strlen("Total")) == 0)
		createSebastienFile(fs, *separator, headers);
	else
		createOtherFileFormat(fs, *separator, headers);

	if (m_nbPoints == 0){
		delete separator;
		return false;
	}

	m_stats = new ArrayStatistics[1];
	m_stats[0] = GeneralTools::generateArrayStatistics(m_intensities, m_nbPoints);

	m_nbHisto = 1;
	m_histograms = new Histogram *[m_nbHisto];
	m_histograms[0] = NULL;
	computeHistograms();
	m_palette = Palette::getStaticLut("InvFire");
	if (m_colors != NULL)
		delete[] m_colors;
	m_colors = new Color4D[m_nbPoints];
	m_selection = new bool[m_nbPoints];
	regenerateIntensityColorVector();

	delete separator;
}

void DetectionSet::createSebastienFile(std::ifstream & _fs, const char _separator, std::vector < std::string > & _headers)
{
	intensityMin = FLT_MAX;
	intensityMax = FLT_MIN;
	m_w = m_h = 0.;

	std::vector < DetectionPoint > points;
	std::vector < double > intensities, sigmas;
	std::vector < unsigned short > times;
	int nbSlices = 0, nbPoints = 0;

	MyTimer timer;

	_fs.clear();
	_fs.seekg(0, std::ios::beg);
	int nbLines = std::count(std::istreambuf_iterator<char>(_fs), std::istreambuf_iterator<char>(), '\n');
	_fs.clear();
	_fs.seekg(0, std::ios::beg);
	unsigned int currentLine = 0, nbForUpdate = nbLines / 100.;
	if (nbForUpdate == 0) nbForUpdate = 1;

	std::cout << "# of lines:" << nbLines << ", time for computing number of lines " << timer.getTimeElapsed().toAscii().data() << std::endl;
	printf("Reading file: %.2f %%", (double)currentLine / nbLines * 100.);

	std::string s;
	std::getline(_fs, s);
	std::istringstream is(s);
	std::string tmp;
	for (int i = 0; i < 4; i++){
		is >> tmp;
	}
	is >> nbPoints;
	std::getline(_fs, s);

	double x, y, intensity = 0., sigma = 0.;
	int t;
	//Sigmas and intensities are generated as a gaussian distribution of mean 25 and deviation 10
	std::random_device rd;
	std::mt19937 gen(rd());
	std::normal_distribution <> dsigma(25, 10), dintensity(1500, 400);

	double xmax = DBL_MIN, ymax = DBL_MIN;

	/*for (int n = 0; n < nbPoints; n++){
		std::getline(_fs, s);*/
	while (std::getline(_fs, s)){
		//std::cout << s.c_str() << std::endl;
		std::istringstream is2(s);
		is2 >> t >> x >> y;
		if (x <= 0. || y <= 0.) continue;
		points.push_back(DetectionPoint(x, y, 0));
		times.push_back((unsigned short)t);
		double val = dintensity(gen);
		while (val < 0.) val = dintensity(gen);
		intensities.push_back(val);
		val = dsigma(gen);
		while (val < 0.) val = dsigma(gen);
		sigmas.push_back(val);
		if (x > xmax)
			xmax = x;
		if (y > ymax)
			ymax = y;
		//if (currentLine++ % nbForUpdate == 0) printf("\rReading file: %.2f %%", ((double)currentLine / nbLines * 100.));
		if (x > m_w)
			m_w = x;
		if (y > m_h)
			m_h = y;
	}
	printf("\n");

	m_nbSlices = times.at(times.size() - 1);
	m_nbPoints = points.size();
	m_firstsPoint = new unsigned int[m_nbSlices];
	m_sizePoints = new unsigned int[m_nbSlices];
	m_intensities = new double[m_nbPoints];
	m_sigmas = new double[m_nbPoints];
	m_points = new DetectionPoint[m_nbPoints];

	std::copy(points.begin(), points.end(), m_points);
	std::copy(intensities.begin(), intensities.end(), m_intensities);
	std::copy(sigmas.begin(), sigmas.end(), m_sigmas);

	memset(m_firstsPoint, 0, m_nbSlices * sizeof(unsigned int));
	memset(m_sizePoints, 0, m_nbSlices * sizeof(unsigned int));
	for (unsigned int n = 0; n < m_nbPoints; n++){
		m_sizePoints[times.at(n)]++;
		if (m_intensities[n] > intensityMax)
			intensityMax = m_intensities[n];
		if (m_intensities[n] < intensityMin)
			intensityMin = m_intensities[n];
	}
	for (int n = 1; n < m_nbSlices; n++)
		m_firstsPoint[n] = m_firstsPoint[n - 1] + m_sizePoints[n - 1];

	//m_w = ceil(m_w);
	//m_h = ceil(m_h);
}

void DetectionSet::createTesselerFile(std::ifstream & _fs, const char _separator, std::vector < std::string > & _headers)
{
	intensityMin = FLT_MAX;
	intensityMax = FLT_MIN;
	m_nbPoints = 0;

	std::vector < std::string > values;
	std::string s;

	m_nbSlices = atoi(_headers[0].c_str());
	m_nbPoints = atoi(_headers[1].c_str());
	m_firstsPoint = new unsigned int[m_nbSlices];
	m_sizePoints = new unsigned int[m_nbSlices];
	m_intensities = new double[m_nbPoints];
	memset(m_firstsPoint, 0, m_nbSlices * sizeof(unsigned int));
	memset(m_sizePoints, 0, m_nbSlices * sizeof(unsigned int));
	m_points = new DetectionPoint[m_nbPoints];
	std::vector < double > sigmas;
	double x, y, intensity, sigma;
	int t;

	unsigned int cpt = 0;
	GeneralTools::m_imw->m_progress->setMaximum(m_nbPoints);
	GeneralTools::m_imw->m_progress->setValue(cpt++);

	unsigned int cptt = 0;
	for (int n = 0; n < m_nbPoints; n++, cptt++){
		GeneralTools::m_imw->m_progress->setValue(cpt++);
		std::getline(_fs, s);

		//std::cout << s.c_str() << std::endl;
		if (s.empty()) continue;

		std::vector < std::string > values;
		values = split(s, _separator, values);
		x = ::atof(values[0].c_str());
		y = ::atof(values[1].c_str());
		intensity = ::atof(values[2].c_str());
		t = ::atoi(values[3].c_str());
		if (values.size() == 5)
			sigmas.push_back(::atof(values[4].c_str()));

		//fs >> x >> y >> intensity >> t;
		m_points[n].set(x, y, 0);
		m_intensities[n] = intensity;
		m_sizePoints[t]++;
		if (x > m_w)
			m_w = x;
		if (y > m_h)
			m_h = y;
	}
	for (int n = 1; n < m_nbSlices; n++)
		m_firstsPoint[n] = m_firstsPoint[n - 1] + m_sizePoints[n - 1];
	if (!sigmas.empty()){
		m_sigmas = new double[m_nbPoints];
		for (int n = 0; n < m_nbPoints; n++)
			m_sigmas[n] = sigmas[n];
	}
	//m_w = ceil(m_w);
	//m_h = ceil(m_h);
}

void DetectionSet::createOtherFileFormat(std::ifstream & _fs, const char _separator, std::vector < std::string > & _headers)
{
	intensityMin = FLT_MAX;
	intensityMax = FLT_MIN;
	m_nbPoints = 0;

	std::vector < std::string > values;
	std::string s;

	std::string xs1("x"), xs2("Position X"), ys1("y"), ys2("Position Y"), intensities1("intensity"), intensities2("Number Photons"), frames1("frame"), frames2("First Frame"), sigmas1("sigma"), sigmas2("Precision");

	int indexXs, indexYs, indexFrames, indexIntensities, indexSigmas;
	indexXs = indexYs = indexFrames = indexIntensities = indexSigmas = -1;
	for (unsigned int n = 0; n < _headers.size(); n++){
		std::string current = _headers.at(n);
		current.erase(std::remove(current.begin(), current.end(), '"'), current.end());
		if (current.substr(0, std::strlen(xs1.c_str())) == xs1.c_str() || current.substr(0, std::strlen(xs2.c_str())) == xs2.c_str())
			indexXs = n;
		else if (current.substr(0, std::strlen(ys1.c_str())) == ys1.c_str() || current.substr(0, std::strlen(ys2.c_str())) == ys2.c_str())
			indexYs = n;
		else if (current.substr(0, std::strlen(intensities1.c_str())) == intensities1.c_str() || current.substr(0, std::strlen(intensities2.c_str())) == intensities2.c_str())
			indexIntensities = n;
		else if (current.substr(0, std::strlen(frames1.c_str())) == frames1.c_str() || current.substr(0, std::strlen(frames2.c_str())) == frames2.c_str())
			indexFrames = n;
		else if (current.substr(0, std::strlen(sigmas1.c_str())) == sigmas1.c_str() || current.substr(0, std::strlen(sigmas2.c_str())) == sigmas2.c_str())
			indexSigmas = n;
	}
	if (indexXs == -1 || indexYs == -1 || indexFrames == -1 || indexIntensities == -1){
		std::cout << "Unable to open file, missing " << std::endl;
		if (indexXs == -1) std::cout << "x ";
		if (indexYs == -1) std::cout << "y ";
		if (indexFrames == -1) std::cout << "frame ";
		if (indexIntensities == -1) std::cout << "intensity ";
		std::cout << "parameter(s)" << std::endl;
		_fs.close();
		return;
	}

	std::vector < DetectionPoint > points;
	std::vector < double > intensities, sigmas;
	std::vector < unsigned int > times;
	double x, y, ii;
	int currentFrame;

	bool out = false;
	while (std::getline(_fs, s) && !out){
		/*size_t tmp = std::count(s.begin(), s.end(), ' ');
		out = tmp == s.size();
		int merde = s.size();
		std::cout << tmp << " - " << merde << " -" << s.c_str() << "-" << std::endl;*/
		out = s.size() == 2;
		if (!out){
			std::vector < std::string > values;
			values = split(s, _separator, values);
			currentFrame = ::atoi(values[indexFrames].c_str()) - 1;
			x = ::atof(values[indexXs].c_str());
			y = ::atof(values[indexYs].c_str());
			ii = ::atof(values[indexIntensities].c_str());
			if (ii > intensityMax)
				intensityMax = ii;
			if (ii < intensityMin)
				intensityMin = ii;
			points.push_back(DetectionPoint(x, y, 0.));
			intensities.push_back(ii);
			times.push_back(currentFrame);
			if (x > m_w)
				m_w = x;
			if (y > m_h)
				m_h = y;
			if (indexSigmas != -1)
				sigmas.push_back(::atof(values[indexSigmas].c_str()));
		}
	}

	m_nbPoints = points.size();
	m_nbSlices = times.back() + 1;

	m_firstsPoint = new unsigned int[m_nbSlices];
	m_sizePoints = new unsigned int[m_nbSlices];
	m_intensities = new double[m_nbPoints];
	if (indexSigmas != -1)
		m_sigmas = new double[m_nbPoints];
	else
		m_sigmas = NULL;
	memset(m_firstsPoint, 0, m_nbSlices * sizeof(bool));
	memset(m_sizePoints, 0, m_nbSlices * sizeof(unsigned int));
	m_points = new DetectionPoint[m_nbPoints];


	for (int n = 0; n < m_nbPoints; n++){
		m_points[n].set(points[n].x(), points[n].y(), 0);
		m_intensities[n] = intensities[n];
		m_sizePoints[times[n]]++;
		if (m_sigmas != NULL) m_sigmas[n] = sigmas[n];
	}
	for (int n = 1; n < m_nbSlices; n++)
		m_firstsPoint[n] = m_firstsPoint[n - 1] + m_sizePoints[n - 1];
}

void DetectionSet::colorLocsOfObject(unsigned int * _indexes, const int _nbLocs, const Color4D & _color)
{
	for (unsigned int n = 0; n < _nbLocs; n++){
		const unsigned int current = _indexes[n];
		m_colors[current].set(_color.x(), _color.y(), _color.z(), _color.w());
	}
}

void DetectionSet::setColors(Color4D * _colors)
{ 
	memcpy(m_colors, _colors, m_nbPoints * sizeof(Color4D));
}
