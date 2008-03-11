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

$Log: not supported by cvs2svn $
Revision 1.13  2008/02/05 20:42:43  cignoni
Other small typos

Revision 1.12  2008/02/04 21:26:49  ganovelli
added ImportLocal which imports all local attributes into vertexplus and faceplus.
A local attribute is everything (N(), C(), Q()....) except pointers to other simplices
(i.e. FFAdj, VFAdj, VertexRef) which are set to NULL.
Added some function for const attributes

Revision 1.11  2007/12/11 18:25:31  cignoni
added missing include limits

Revision 1.10  2007/12/11 11:36:03  cignoni
Added the CompactVertexVector garbage collecting function.

Revision 1.9  2006/12/11 23:42:00  ganovelli
bug Index()() instead of Index()

Revision 1.8  2006/12/04 11:17:42  ganovelli
added forward declaration of TriMesh

Revision 1.7  2006/11/07 17:22:52  cignoni
many gcc compiling issues

Revision 1.6  2006/11/07 15:13:57  zifnab1974
Necessary changes for compilation with gcc 3.4.6. Especially the hash function is a problem

Revision 1.5  2006/11/07 11:29:24  cignoni
Corrected some errors in the reflections Has*** functions

Revision 1.4  2006/10/31 16:02:59  ganovelli
vesione 2005 compliant

Revision 1.3  2006/02/28 11:59:55  ponchio
g++ compliance:

begin() -> (*this).begin() and for end(), size(), Base(), Index()

Revision 1.2  2005/12/12 11:17:32  cignoni
Corrected update function, now only the needed simplexes should be updated.

Revision 1.1  2005/10/14 15:07:59  cignoni
First Really Working version


****************************************************************************/

/* 
Note
OCF = Optional Component Fast (hopefully)
compare with OCC(Optional Component Compact)

Mainly the trick here is to store a base pointer in each simplex...

****************************************************************************/
#ifndef __VCG_VERTEX_PLUS_COMPONENT_OCF
#define __VCG_VERTEX_PLUS_COMPONENT_OCF

#include <vcg/simplex/vertexplus/component.h>
#include <vector>
#include <limits>

namespace vcg {
  namespace vert {
/*
All the Components that can be added to a vertex should be defined in the namespace vert:

*/
template <class VALUE_TYPE>
class vector_ocf: public std::vector<VALUE_TYPE> {
  typedef std::vector<VALUE_TYPE> BaseType; 
	typedef typename vector_ocf<VALUE_TYPE>::iterator ThisTypeIterator;
  
public:
	vector_ocf():std::vector<VALUE_TYPE>(){
  QualityEnabled=false;
  ColorEnabled=false;
  NormalEnabled=false;
  VFAdjacencyEnabled=false;
  }
  
  // override di tutte le funzioni che possono spostare 
	// l'allocazione in memoria del container
	void push_back(const VALUE_TYPE & v)
  {
    BaseType::push_back(v);
	BaseType::back()._ovp = this;
  }
	void pop_back();
  void resize(const unsigned int & _size) 
  {
		int oldsize = BaseType::size();
			BaseType::resize(_size);
		if(oldsize<_size){
			ThisTypeIterator firstnew = BaseType::begin();
			advance(firstnew,oldsize);
			_updateOVP(firstnew,(*this).end());
		}  
		if(ColorEnabled) CV.resize(_size);
    if(NormalEnabled) NV.resize(_size);
   }
  
	void reserve(const unsigned int & _size)
  {
    BaseType::reserve(_size);
    if (ColorEnabled) CV.reserve(_size);
    if (NormalEnabled) NV.reserve(_size);
  }

	void _updateOVP(ThisTypeIterator lbegin, ThisTypeIterator lend)
	{
    ThisTypeIterator vi;
    for(vi=lbegin;vi!=lend;++vi)
        (*vi)._ovp=this;
	}
 
// this function is called by the specialized Reorder function, that is called whenever someone call the allocator::CompactVertVector
void ReorderVert(std::vector<size_t> &newVertIndex )
{
	size_t pos=0;
	size_t i=0;
	if(ColorEnabled) assert( CV.size() == newVertIndex.size() );
	if(NormalEnabled) assert( NV.size() == newVertIndex.size() );
		
	for(i=0;i<newVertIndex.size();++i)
		{
			if(newVertIndex[i] != std::numeric_limits<size_t>::max() )
				{
					assert(newVertIndex[i] <= i);
					if(ColorEnabled)  CV[newVertIndex[i]] = CV[i];
					if(NormalEnabled) NV[newVertIndex[i]] = NV[i];;
				}
		}
}



////////////////////////////////////////
// Enabling Eunctions

bool IsQualityEnabled() const {return QualityEnabled;}
void EnableQuality() {
  assert(VALUE_TYPE::HasQualityOcf());
  QualityEnabled=true;
  QV.resize((*this).size());
}

void DisableQuality() {
  assert(VALUE_TYPE::HasQualityOcf());
  QualityEnabled=false;
  QV.clear();
}

bool IsColorEnabled() const {return ColorEnabled;}
void EnableColor() {
  assert(VALUE_TYPE::HasColorOcf());
  ColorEnabled=true;
  CV.resize((*this).size());
}

void DisableColor() {
  assert(VALUE_TYPE::HasColorOcf());
  ColorEnabled=false;
  CV.clear();
}

bool IsNormalEnabled() const {return NormalEnabled;}
void EnableNormal() {
  assert(VALUE_TYPE::HasNormalOcf());
  NormalEnabled=true;
  NV.resize((*this).size());
}

void DisableNormal() {
  assert(VALUE_TYPE::HasNormalOcf());
  NormalEnabled=false;
  NV.clear();
}
  
void EnableVFAdjacency() {
  assert(VALUE_TYPE::HasVFAdjacencyOcf());
  VFAdjacencyEnabled=true;
  AV.resize((*this).size());
}

void DisableVFAdjacency() {
  assert(VALUE_TYPE::HasVFAdjacencyOcf());
  VFAdjacencyEnabled=false;
  AV.clear();
}




struct VFAdjType {
  typename VALUE_TYPE::FacePointer _fp ;    
  int _zp ;    
  };
  
public:
  std::vector<typename VALUE_TYPE::QualityType> QV;
  std::vector<typename VALUE_TYPE::ColorType> CV;
  std::vector<typename VALUE_TYPE::NormalType> NV;
  std::vector<struct VFAdjType> AV;

  bool QualityEnabled;
  bool ColorEnabled;
  bool NormalEnabled;
  bool VFAdjacencyEnabled;
};


//template<>	void EnableAttribute<typename VALUE_TYPE::NormalType>(){	NormalEnabled=true;}

/*------------------------- COORD -----------------------------------------*/ 
/*----------------------------- VFADJ ------------------------------*/ 


template <class T> class VFAdjOcf: public T {
public:
  typename T::FacePointer &VFp() {
    assert((*this).Base().VFAdjacencyEnabled); 
    return (*this).Base().AV[(*this).Index()()]._fp; 
  }

  typename T::FacePointer cVFp() const {
    if(! (*this).Base().VFAdjacencyEnabled ) return 0; 
    else return (*this).Base().AV[(*this).Index()()]._fp; 
  }

  int &VFi() {
    assert((*this).Base().VFAdjacencyEnabled); 
    return (*this).Base().AV[(*this).Index()()]._zp; 
  }
	template <class LeftV>
	void ImportLocal(const LeftV & leftV){VFp() = NULL; VFi() = -1; T::ImporLocal(leftV);}

  static bool HasVFAdjacency()   {   return true; }
  static bool HasVFAdjacencyOcf()   { return true; }

private:
};

/*------------------------- Normal -----------------------------------------*/ 

template <class A, class T> class NormalOcf: public T {
public:
  typedef A NormalType;
  static bool HasNormal()   { return true; }
  static bool HasNormalOcf()   { return true; }

  NormalType &N() { 
    // you cannot use Normals before enabling them with: yourmesh.vert.EnableNormal()
    assert((*this).Base().NormalEnabled); 
    return (*this).Base().NV[(*this).Index()];  }
  const NormalType &N() const { 
    // you cannot use Normals before enabling them with: yourmesh.vert.EnableNormal()
    assert((*this).Base().NormalEnabled); 
    return (*this).Base().NV[(*this).Index()];  }

	template <class LeftV>
	void ImportLocal(const LeftV & leftV){ N() = leftV.cN(); T::ImporLocal(leftV);}
};

template <class T> class Normal3sOcf: public NormalOcf<vcg::Point3s, T> {};
template <class T> class Normal3fOcf: public NormalOcf<vcg::Point3f, T> {};
template <class T> class Normal3dOcf: public NormalOcf<vcg::Point3d, T> {};

///*-------------------------- COLOR ----------------------------------*/ 

template <class A, class T> class ColorOcf: public T {
public:
  typedef A ColorType;
  ColorType &C() { assert((*this).Base().NormalEnabled); return (*this).Base().CV[(*this).Index()()]; }
  const ColorType &cC() const { assert((*this).Base().NormalEnabled); return (*this).Base().CV[(*this).Index()()]; }
	template <class LeftV>
	void ImportLocal(const LeftV & leftV){ C() = leftV.cC(); T::ImporLocal(leftV);}
  static bool HasColor()   { return true; }
  static bool HasColorOcf()   { return true; }
};

template <class T> class Color4bOcf: public ColorOcf<vcg::Color4b, T> {};

///*-------------------------- QUALITY  ----------------------------------*/ 

template <class A, class T> class QualityOcf: public T {
public:
  typedef A QualityType;
  QualityType &Q() { assert((*this).Base().QualityEnabled); return (*this).Base().QV[(*this).Index()()]; }
	template <class LeftV>
	void ImportLocal(const LeftV & leftV){ Q() = leftV.cQ(); T::ImporLocal(leftV);}
  static bool HasQuality()   { return true; }
  static bool HasQualityOcf()   { return true; }
};

template <class T> class QualityfOcf: public QualityOcf<float, T> {};


///*-------------------------- InfoOpt  ----------------------------------*/ 

template < class T> class InfoOcf: public T {
public:
  vector_ocf<typename T::VertType> &Base() const { return *_ovp;}

  inline int Index() const {
    typename  T::VertType const *tp=static_cast<typename T::VertType const*>(this); 
    int tt2=tp- &*(_ovp->begin());
    return tt2;
  } 
public:
  vector_ocf<typename T::VertType> *_ovp;
};


} // end namespace vert

template < class, class > class TriMesh;

namespace tri
{
	template < class VertexType, class FaceContainerType >
    bool HasPerVertexQuality (const TriMesh < vert::vector_ocf< VertexType > , FaceContainerType > & m) 
	{
	  if(VertexType::HasQualityOcf()) return m.vert.IsQualityEnabled();
	  else return VertexType::HasQuality();
	}

	template < class VertexType >
	void ReorderVert( std::vector<size_t>  &newVertIndex, vert::vector_ocf< VertexType > &vertVec)
		{
			vertVec.ReorderVert(newVertIndex);
		}
}
}// end namespace vcg
#endif
