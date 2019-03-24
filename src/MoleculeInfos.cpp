/*
 * Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
 *
 * File:      MoleculeInfos.cpp
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

#include "MoleculeInfos.hpp"

MoleculeInfos::MoleculeInfos()
{

}

MoleculeInfos::MoleculeInfos( const MoleculeInfos & _o )
{
	m_molecule = _o.m_molecule;
	for( int n = 0; n < MoleculeInfos::NB_DATATYPE; n++ )
		m_data[n] = _o.m_data[n];
	for( int n = 0; n < MoleculeInfos::NB_DATATYPE; n++ )
		m_dataLog[n] = _o.m_dataLog[n];
}

void MoleculeInfos::setNeighborsInfos( EdgeCirc * _edges, const int _nbEdges )
{
	setEdges( _edges );
	setNbEdges( _nbEdges );
}

VertHandle MoleculeInfos::getNeighbor( const int _idx ) const
{
	return m_edges[_idx]->first->vertex( m_edges[_idx]->first->ccw( m_edges[_idx]->second ) );
}