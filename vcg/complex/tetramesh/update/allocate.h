/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2004                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *   
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/
/****************************************************************************
  History

Revision 1.1  2004/19/04 13:05  pietroni
Initial commit


****************************************************************************/
#ifndef __VCG_TETRA_ALLOCATE
#define __VCG_TETRA_ALLOCATE
#include <vector>
using namespace std;
namespace vcg {
namespace tetra {
/** \addtogroup tetramesh */
/*@{*/

 /**  Class Allocate.
 This is class for Allocate new vertices or tetrahedron on the mesh.
		@param TM_TYPE (Template Parameter) Specifies the type of the tetrahedral mesh.
 */

template  < class TM_TYPE >
class Allocator
{

public:

	/// The tetramesh type
	typedef TM_TYPE TetraMeshType;

	
	/// The vertex type 
	typedef typename TM_TYPE::VertexType VertexType;
	
	/// The tetrahedron type 
	typedef typename TM_TYPE::TetraType TetraType;

	/// The type of vertex iterator
	typedef typename TM_TYPE::VertexIterator VertexIterator;

	/// The type of tetra iterator
	typedef typename TM_TYPE::TetraIterator TetraIterator;

	/// The type of constant vertex iterator
	typedef typename TM_TYPE::const_VertexIterator const_VertexIterator;

	/// The type of constant face iterator
	typedef typename TM_TYPE::const_TetraIterator const_TetraIterator;


public:
 
  /** Function to add n vertices to the mesh. The second parameter hold a vector of 
pointers to pointer to elements of the mesh that should be updated after a 
possible vector realloc. 
@param n Il numero di vertici che si vuole aggiungere alla mesh.
@param local_var Vettore di variabili locali che rappresentano puntatori a vertici. 
restituisce l'iteratore al primo elemento aggiunto.
*/

VertexIterator AddVertices(TetraMeshType &m,int n, vector<VertexType **> &local_var)
{
	VertexIterator oldbegin, newbegin;
	oldbegin = m.vert.begin();
  VertexIterator last=m.vert.end();
	if(m.vert.empty()) last=0;  // if the vector is empty we cannot find the last valid element
	else --last;
	unsigned int siz=0;
#ifdef __STL_CONFIG_H	
if(last!=0) distance(m.vert.begin(),last,siz);
#else
if(last!=0) siz=distance(m.vert.begin(),last);
#endif
	for(unsigned int i=0; i<n; ++i)
	{
		m.vert.push_back(VertexType());
		m.vert.back().ClearFlags();
	}
	m.vn+=n;
	newbegin = m.vert.begin();
	if(newbegin != oldbegin)
		{
			TetraIterator f;
			for (f=m.tetra.begin(); f!=m.tetra.end(); ++f)
				if(!(*f).IsD())
				for(unsigned int k=0; k<4; ++k)
					(*f).V(k)= (*f).V(k)-&*oldbegin+&*newbegin;
			for(unsigned int j=0; j<local_var.size(); ++j)
				if((*local_var[j]) !=0 ) *local_var[j] = *local_var[j]-&*oldbegin+&*newbegin;

			// deve restituire l'iteratore alla prima faccia aggiunta;
			// e poiche' lo spazio e' cambiato si ricalcola last da zero  
			if(last!=0) 
			{ 
				last = m.vert.begin(); 
				advance(last,siz+1);
			}
			else last=m.vert.begin(); 
		}
	else 
	{ 
		// se non e'cambiato lo spazio (vector abbastanza grande o lista)
		if(last==0) last = m.vert.begin(); // se il vettore era vuoto si restituisce begin
		           else advance(last,1); // altrimenti il primo dopo quello che era in precedenza l'ultimo valido.
	}
	return last;
}

 /** Function to add n vertices to the mesh. 
@param n Il numero di vertici che si vuole aggiungere alla mesh.
*/
VertexIterator AddVertices(TetraMeshType &m,int n)
{
	VertexIterator oldbegin, newbegin;
	oldbegin = m.vert.begin();
  VertexIterator last=m.vert.end();
	if(m.vert.empty()) last=0;  // if the vector is empty we cannot find the last valid element
	else --last;
	unsigned int siz=0;
#ifdef __STL_CONFIG_H	
if(last!=0) distance(m.vert.begin(),last,siz);
#else
if(last!=0) siz=distance(m.vert.begin(),last);
#endif
	for(unsigned int i=0; i<n; ++i)
	{
		m.vert.push_back(VertexType());
		m.vert.back().ClearFlags();
	}
	m.vn+=n;
	newbegin = m.vert.begin();
	if(newbegin != oldbegin)
		{
			TetraIterator f;
			for (f=m.tetra.begin(); f!=m.tetra.end(); ++f)
				if(!(*f).IsD())
				for(unsigned int k=0; k<4; ++k)
					(*f).V(k)= (*f).V(k)-&*oldbegin+&*newbegin;
			
			// deve restituire l'iteratore alla prima faccia aggiunta;
			// e poiche' lo spazio e' cambiato si ricalcola last da zero  
			if(last!=0) 
			{ 
				last = m.vert.begin(); 
				advance(last,siz+1);
			}
			else last=m.vert.begin(); 
		}
	else 
	{ 
		// se non e'cambiato lo spazio (vector abbastanza grande o lista)
		if(last==0) last = m.vert.begin(); // se il vettore era vuoto si restituisce begin
		           else advance(last,1); // altrimenti il primo dopo quello che era in precedenza l'ultimo valido.
	}
	return last;
}

}; // end class


/*@}*/
}	// End namespace
}	// End namespace


#endif
