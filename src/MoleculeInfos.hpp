/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      MoleculeInfos.hpp
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

#ifndef MoleculeInfos_h__
#define MoleculeInfos_h__

#include "ObjectInterface.hpp"

class MoleculeInfos{
public:
	enum Type {Area = 2, MeanDistance = 1, LocalDensity = 0, Mean = 0, Median = 1, StdDev = 2, Delta = 3};

	MoleculeInfos();
	MoleculeInfos( const MoleculeInfos & );

	void setNeighborsInfos( EdgeCirc *, const int );
	VertHandle getNeighbor( const int ) const;

	inline void setData( const int _idx, const float _val ){m_data[_idx] = _val;}
	inline double getData( const int _idx ) const {return m_data[_idx];}
	inline void setDataLog( const int _idx, const float _val ){m_dataLog[_idx] = _val;}
	inline double getDataLog( const int _idx ) const {return m_dataLog[_idx];}
	inline void setMolecule( VertHandle _mol) {m_molecule = _mol;}
	inline VertHandle getMolecule() const {return m_molecule;}
	inline void setEdges( EdgeCirc * _neighs ){m_edges = _neighs;}
	inline void setNbEdges( const int _nb ){m_nbEdges = _nb;}
	inline EdgeCirc * getEdges() const {return m_edges;}
	inline EdgeCirc getEdge( const int _idx ) const {return m_edges[_idx];}
	inline const int nbEdges() const {return m_nbEdges;}

public:
	static unsigned short NB_DATATYPE;

protected:
	double m_data[3];
	double m_dataLog[3];
	VertHandle m_molecule;
	EdgeCirc * m_edges;
	int m_nbEdges;
};

#endif // MoleculeInfos_h__