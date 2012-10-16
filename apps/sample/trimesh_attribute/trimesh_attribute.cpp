/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2004-2012                                           \/)\/    *
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
/*! \file trimesh_attribute.cpp
\ingroup code_sample

\brief the minimal example of using the attributes

Attributes are a simple mechanism to associate user-defined 'attributes' to the simplicies and to the mesh.
\ref attributes for more Details

*/

#include<vcg/complex/complex.h>
class MyEdge;
class MyFace;
class MyVertex;
struct MyUsedTypes : public vcg::UsedTypes<	vcg::Use<MyVertex>		::AsVertexType,
                                            vcg::Use<MyFace>			::AsFaceType>{};

class MyVertex  : public vcg::Vertex< MyUsedTypes, vcg::vertex::Coord3f,vcg::vertex::Normal3f>{};
class MyFace    : public vcg::Face< MyUsedTypes, vcg::face::VertexRef, vcg::face::Normal3f> {};

class MyMesh : public vcg::tri::TriMesh< std::vector<MyVertex>, std::vector<MyFace> > {};

int main()
{
  MyMesh m;
  // add a per-vertex attribute with type float named "Irradiance"
  MyMesh::PerVertexAttributeHandle<float> named_hv = vcg::tri::Allocator<MyMesh>::AddPerVertexAttribute<float> (m,std::string("Irradiance"));

  // add a per-vertex attribute with type float named "Radiosity"   
  vcg::tri::Allocator<MyMesh>::AddPerVertexAttribute<float> (m,std::string("Radiosity"));
 
  // add a per-vertex attribute with type bool and no name specified
  MyMesh::PerVertexAttributeHandle<bool> anon_hv = vcg::tri::Allocator<MyMesh>::AddPerVertexAttribute<bool> (m);
  
  // add a per-face attribute with type bool and no name specified
  MyMesh::PerFaceAttributeHandle<bool> anon_hf = vcg::tri::Allocator<MyMesh>::AddPerFaceAttribute<bool> (m);

  MyMesh::VertexIterator vi; int i = 0;
  for(vi   = m.vert.begin(); vi != m.vert.end(); ++vi,++i){
   named_hv[vi]  = 1.0f;  // [] operator takes a iterator
   named_hv[*vi] = 1.0f;  //                or a MyMesh::VertexType object
   named_hv[&*vi]= 1.0f;  //                or a pointer to it
   named_hv[i]   = 1.0f;  //                or an integer index
  }

  // query if an attribute is present or not
  bool hasRadiosity = vcg::tri::HasPerVertexAttribute(m,"Radiosity");

  // obtain another handle of a previously attribute
  MyMesh::PerVertexAttributeHandle<float> ret_hv = vcg::tri::Allocator<MyMesh>::GetPerVertexAttribute<float>(m,"Radiosity");

  // you can also have PerMesh attributes
  MyMesh::PerMeshAttributeHandle<int> hm = vcg::tri::Allocator<MyMesh>::AddPerMeshAttribute<int> (m,std::string("ADummyIntegerAttribute"));

  // PerMesh attributes are accessed directly using the handle itself
  hm() = 10;

  // delete an attribute by name
  vcg::tri::Allocator<MyMesh>::DeletePerVertexAttribute(m,"Radiosity");

  // delete an attribute by handle
  vcg::tri::Allocator<MyMesh>::DeletePerVertexAttribute(m,anon_hv);

  bool res;
  res = vcg::tri::Allocator<MyMesh>::IsValidHandle(m,named_hv);         printf("Is Valid: %s\n",res?"Yes":"No");
  res = vcg::tri::Allocator<MyMesh>::IsValidHandle(m,anon_hf); printf("Is Valid: %s\n",res?"Yes":"No");
  res = vcg::tri::Allocator<MyMesh>::IsValidHandle(m,hm); printf("Is Valid: %s\n",res?"Yes":"No");
  vcg::tri::Allocator<MyMesh>::DeletePerVertexAttribute(m,ret_hv);
  vcg::tri::Allocator<MyMesh>::DeletePerFaceAttribute(m,anon_hf);
  res = vcg::tri::Allocator<MyMesh>::IsValidHandle(m,named_hv);         printf("Is Valid: %s\n",res?"Yes":"No");
  res = vcg::tri::Allocator<MyMesh>::IsValidHandle(m,anon_hf); printf("Is Valid: %s\n",res?"Yes":"No");
  res = vcg::tri::Allocator<MyMesh>::IsValidHandle(m,hm); printf("Is Valid: %s\n",res?"Yes":"No");
}
