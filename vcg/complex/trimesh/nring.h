#ifndef RINGWALKER_H
#define RINGWALKER_H

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

#include <vcg/simplex/face/jumping_pos.h>
#include <vcg/complex/trimesh/allocate.h>
#include <vcg/complex/trimesh/update/flag.h>
#include <vector>
#include <set>

using namespace std;

namespace vcg
{
namespace tri 
{
		
	/** \addtogroup trimesh */
	/*@{*/
	/*@{*/
	/** Class Mesh.
	 This is class for extracting n-ring of vertexes or faces, starting from a vertex of a mesh.
	 */
template <class MeshType>
class Nring
{
public:

    typedef typename MeshType::FaceType   FaceType;
    typedef typename MeshType::VertexType VertexType;
    typedef typename MeshType::ScalarType ScalarType;
    typedef typename MeshType::FaceIterator FaceIterator;
    typedef typename MeshType::VertexIterator VertexIterator;
    typedef typename MeshType::CoordType CoordType;


    vector<VertexType*> allV;
    vector<FaceType*> allF;

    vector<VertexType*> lastV;
    vector<FaceType*> lastF;

    MeshType* m;

    RingWalker(VertexType* v, MeshType* m) : m(m)
    {
        assert((v - &*m->vert.begin()) < m->vert.size());
        insertAndFlag(v);

    }
	
    ~RingWalker()
    {
		clear();
    }

    void insertAndFlag1Ring(VertexType* v)
    {
        insertAndFlag(v);

        typename vcg::face::Pos<FaceType> p(v->VFp(),v);
        assert(p.V() == v);

        int count = 0;
        vcg::face::Pos<FaceType> ori = p;
        do
        {
            insertAndFlag(p.F());
            p.FlipF();
            p.FlipE();
            assert(count++ < 100);
        } while (ori != p);

    }

    void insertAndFlag(FaceType* f)
    {
        if (!f->IsV())
        {
            allF.push_back(f);
            lastF.push_back(f);
            f->SetV();
            insertAndFlag(f->V(0));
            insertAndFlag(f->V(1));
            insertAndFlag(f->V(2));
        }
    }

    void insertAndFlag(VertexType* v)
    {
        if (!v->IsV())
        {
            allV.push_back(v);
            lastV.push_back(v);
            v->SetV();
        }
    }


    static void clearFlags(MeshType* m)
    {
        vcg::tri::UpdateFlags<MeshType>::VertexClearV(*m);
        vcg::tri::UpdateFlags<MeshType>::FaceClearV(*m);
    }

    void clear()
    {
        for(int i=0; i< allV.size(); ++i)
            allV[i]->ClearV();
        for(int i=0; i< allF.size(); ++i)
            allF[i]->ClearV();

        allV.clear();
        allF.clear();
    }

    void expand()
    {
        vector<VertexType*> lastVtemp = lastV;

        lastV.clear();
        lastF.clear();

        for(typename vector<VertexType*>::iterator it = lastVtemp.begin(); it != lastVtemp.end(); ++it)
        {
            insertAndFlag1Ring(*it);
        }
    }

    void expand(int k)
    {
        for(int i=0;i<k;++i)
            expand();
    }
};

}} // end namespace NAMESPACE
#endif // RINGWALKER_H
