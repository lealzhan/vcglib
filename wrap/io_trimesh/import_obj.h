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
Revision 1.19  2008/01/17 18:02:02  cignoni
added missing cast for normal assignments

Revision 1.18  2007/12/13 17:57:33  cignoni
removed harmless gcc warnings

Revision 1.17  2007/10/17 09:49:50  cignoni
correct management of point only files

Revision 1.16  2007/07/20 14:49:46  cignoni
Added in load mask the face color bit when there is a generic material used

Revision 1.15  2007/07/05 14:47:04  cignoni
Added face coloring when there is a texture (and therefore a material)

Revision 1.14  2007/04/18 13:33:11  cignoni
resolved issue related to the parsing of CR LF under *nixes

Revision 1.13  2007/04/18 07:01:26  cignoni
Added managment of map_Ka textures (and not only map_Kd)

Revision 1.12  2006/12/21 00:36:17  cignoni
Removed a bug in the managment of non triangular faces

Revision 1.11  2006/12/12 02:47:12  cignoni
Removed use of tellg that is broken in current version of mingw

Revision 1.10  2006/11/21 10:56:41  cignoni
ReWrote loadMask. Now shorter and faster.

Revision 1.9  2006/10/09 19:58:08  cignoni
Added casts to remove warnings

Revision 1.8  2006/07/09 05:41:17  cignoni
Major rewrite. Now shorter and more robust.

Revision 1.7  2006/06/21 04:26:26  cignoni
added initial test on end of file in the tokenize

Revision 1.6  2006/05/21 07:01:04  cignoni
Added mask clamping to the effective capabilities of the mesh

Revision 1.5  2006/04/11 09:48:00  zifnab1974
changes needed for compilation on linux 64b with gcc 3.4.5

Revision 1.4  2006/03/29 09:27:07  cignoni
Added managemnt of non critical errors

Revision 1.3  2006/03/29 08:51:16  corsini
reset to zero warnings

Revision 1.2  2006/03/27 07:18:22  cignoni
added missing std::

Revision 1.1  2006/03/07 13:19:29  cignoni
First Release with OBJ import support

Initial Working version coded by Buzzelli.
****************************************************************************/

#ifndef __VCGLIB_IMPORT_OBJ
#define __VCGLIB_IMPORT_OBJ

#include <wrap/callback.h>
#include <vcg/complex/trimesh/allocate.h>
#include <wrap/io_trimesh/io_mask.h>
#include <wrap/io_trimesh/io_material.h>

#include <fstream>
#include <string>
#include <vector>


namespace vcg {
namespace tri {
namespace io {

/** 
This class encapsulate a filter for importing obj (Alias Wavefront) meshes.
Warning: this code assume little endian (PC) architecture!!!
*/
template <class OpenMeshType>
class ImporterOBJ
{
public:

typedef typename OpenMeshType::VertexPointer VertexPointer;
typedef typename OpenMeshType::ScalarType ScalarType;
typedef typename OpenMeshType::VertexType VertexType;
typedef typename OpenMeshType::FaceType FaceType;
typedef typename OpenMeshType::VertexIterator VertexIterator;
typedef typename OpenMeshType::FaceIterator FaceIterator;
typedef typename OpenMeshType::CoordType CoordType;

class Info
{
public:

  Info()
  {
    mask	= 0;
    cb		= 0;
    numTexCoords=0;
  }

	/// It returns a bit mask describing the field preesnt in the ply file
  int mask;  

  /// a Simple callback that can be used for long obj parsing. 
  // it returns the current position, and formats a string with a description of what th efunction is doing (loading vertexes, faces...)
  CallBackPos *cb;

  /// number of vertices
  int numVertices;
 	/// number of faces (the number of triangles could be 
  /// larger in presence of polygonal faces
	int numFaces;
 	/// number of texture coords indexes
	int numTexCoords;

}; // end class


//struct OBJFacet
//{
//  CoordType n;
//	CoordType t;
//  CoordType v[3];
//
//	short attr;  // material index
//};
struct ObjIndexedFace
{ 
	void set(const int & num){v.resize(num);n.resize(num); t.resize(num);}
	std::vector<int> v;
	std::vector<int> n;
	std::vector<int> t;
	int tInd;
	bool  edge[3];// useless if the face is a polygon, no need to have variable length array
	Color4b c;
};

struct ObjTexCoord
{
	float u;
	float v;
};

enum OBJError {
		// Successfull opening
	E_NOERROR													= 0x000,	//	0  (position of correspondig string in the array)
	
	// Non Critical Errors (only odd numbers)
	E_NON_CRITICAL_ERROR							= 0x001,
	E_MATERIAL_FILE_NOT_FOUND					= 0x003,	//  1
	E_MATERIAL_NOT_FOUND							= 0x005,	//  2
	E_TEXTURE_NOT_FOUND								= 0x007,	//  3
	E_VERTICES_WITH_SAME_IDX_IN_FACE	= 0x009,  //	4

	// Critical Opening Errors (only even numbers)
	E_CANTOPEN										=	0x00A,	//  5
	E_UNESPECTEDEOF								= 0x00C,	//  6
	E_ABORTED											= 0x00E,	//  7
	E_NO_VERTEX										= 0x010,	//  8
	E_NO_FACE											= 0x012,	//  9
	E_BAD_VERTEX_STATEMENT				= 0x014,	// 10
	E_BAD_VERT_TEX_STATEMENT			= 0x016,	// 11
	E_BAD_VERT_NORMAL_STATEMENT	  = 0x018,	// 12
	E_LESS_THAN_3VERTINFACE				= 0x01A,	// 13
	E_BAD_VERT_INDEX							= 0x01C,	// 14
	E_BAD_VERT_TEX_INDEX 					= 0x01E,	// 15
	E_BAD_VERT_NORMAL_INDEX 			= 0x020		// 16
};

// to check if a given error is critical or not.
static bool ErrorCritical(int err)
{ 
  if(err<0x00A && err>=0) return false;
  return true;
}

static const char* ErrorMsg(int error)
{
  static const char* obj_error_msg[] =
  {
		"No errors",																									//  0

		"Material library file wrong or not found, a default white material is used",						//  1
		"Some materials definitions were not found, a default white material is used where no material was available",  // 2
		"Texture file not found",																			//  3
		"Identical index vertices found in the same face",						//	4

		"Can't open file",																						//  5
		"Premature End of file",																			//  6
		"File opening aborted",																				//  7
		"No vertex field found",																			//  8
		"No face field found",																				//  9
		"Vertex statement with less than 3 coords",										// 10
		"Texture coords statement with less than 2 coords",						// 11
		"Vertex normal statement with less than 3 coords",						// 12  
		"Face with less than 3 vertices",															// 13
		"Bad vertex index in face",																		// 14
		"Bad texture coords index in face",														// 15
		"Bad vertex normal index in face"															// 16
	};

	// due to approximation, following line works well for either even (critical err codes)
	// or odd (non critical ones) numbers
	error = (int) error/2;

  if(error>15 || error<0) return "Unknown error";
  else return obj_error_msg[error];
};

// Helper functions that checks the range of indexes 
// putting them in the correct range if less than zero (as in the obj style)

static bool GoodObjIndex(int &index, const int maxVal)
{
  if (index > maxVal)	return false;
  if (index < 0)
	{
		index += maxVal+1;
		if (index<0 || index > maxVal)	return false;
	}
  return true;
}

static int Open(OpenMeshType &mesh, const char *filename, int &loadmask, CallBackPos *cb=0)
{
  Info oi;
  oi.mask=-1;
  oi.cb=cb;
  int ret=Open(mesh,filename,oi);
  loadmask=oi.mask;
  return ret;
}

/*!
* Opens an object file (in ascii format) and populates the mesh passed as first
* accordingly to read data
* \param m The mesh model to be populated with data stored into the file
* \param filename The name of the file to be opened
* \param oi A structure containing infos about the object to be opened
*/
static int Open( OpenMeshType &m, const char * filename, Info &oi)
{
	int result = E_NOERROR;

	m.Clear();
		CallBackPos *cb = oi.cb;

	// if LoadMask has not been called yet, we call it here
	if (oi.mask == -1)
		LoadMask(filename, oi);
  
  Mask::ClampMask<OpenMeshType>(m,oi.mask);

	if (oi.numVertices == 0)
		return E_NO_VERTEX;

	// Commented out this test. You should be allowed to load point clouds.
	//if (oi.numFaces == 0)
	//	return E_NO_FACE;

	std::ifstream stream(filename);
	if (stream.fail())
		return E_CANTOPEN;

	std::vector<Material>	materials;  // materials vector
	std::vector<ObjTexCoord>	texCoords;  // texture coordinates
	std::vector<CoordType>  normals;		// vertex normals
  std::vector<ObjIndexedFace> indexedFaces;
	std::vector< std::string > tokens;
	std::string	header;

	short currentMaterialIdx = 0;			// index of current material into materials vector
  Color4b currentColor=Color4b::LightGray;	// we declare this outside code block since other 
				     										// triangles of this face will share the same color
				
	Material defaultMaterial;					// default material: white
	materials.push_back(defaultMaterial);

	int numVertices  = 0;  // stores the number of vertices been read till now
	int numTriangles = 0;  // stores the number of faces been read till now
	int numTexCoords = 0;  // stores the number of texture coordinates been read till now
	int numVNormals	 = 0;  // stores the number of vertex normals been read till now

	int numVerticesPlusFaces = oi.numVertices + oi.numFaces;
  int extraTriangles=0;
	// vertices and faces allocatetion
	VertexIterator vi = Allocator<OpenMeshType>::AddVertices(m,oi.numVertices);
	//FaceIterator   fi = Allocator<OpenMeshType>::AddFaces(m,oi.numFaces);
  
  ObjIndexedFace	ff; 

	while (!stream.eof())
	{
		tokens.clear();
		TokenizeNextLine(stream, tokens);
		
		unsigned int numTokens = static_cast<unsigned int>(tokens.size());
		if (numTokens > 0)
		{
			header.clear();
			header = tokens[0];

			if (header.compare("v")==0)	// vertex
			{
				if (numTokens < 4) return E_BAD_VERTEX_STATEMENT;

				(*vi).P()[0] = (ScalarType) atof(tokens[1].c_str());
				(*vi).P()[1] = (ScalarType) atof(tokens[2].c_str());
				(*vi).P()[2] = (ScalarType) atof(tokens[3].c_str());
				++numVertices;
				
				// assigning vertex color
				// ----------------------
				if( oi.mask & vcg::tri::io::Mask::IOM_VERTCOLOR)
					(*vi).C() = currentColor;

				++vi;  // move to next vertex iterator

				// callback invocation, abort loading process if the call returns false
				if ((cb !=NULL) && (((numTriangles + numVertices)%100)==0) && !(*cb)((100*(numTriangles + numVertices))/numVerticesPlusFaces, "Vertex Loading"))
					return E_ABORTED;
			}
			else if (header.compare("vt")==0)	// vertex texture coords
			{
				if (numTokens < 3) return E_BAD_VERT_TEX_STATEMENT;

				ObjTexCoord t;
				t.u = static_cast<float>(atof(tokens[1].c_str()));
				t.v = static_cast<float>(atof(tokens[2].c_str()));
				texCoords.push_back(t);
				
				numTexCoords++;
			}
			else if (header.compare("vn")==0)  // vertex normal
			{
				if (numTokens != 4) return E_BAD_VERT_NORMAL_STATEMENT;

				CoordType n;
				n[0] = (ScalarType) atof(tokens[1].c_str());
				n[1] = (ScalarType) atof(tokens[2].c_str());
				n[2] = (ScalarType) atof(tokens[3].c_str());	
				normals.push_back(n);

				numVNormals++;
			}
			else if (header.compare("f")==0)  // face
			{
				if (numTokens < 4) return E_LESS_THAN_3VERTINFACE;
				int vertexesPerFace = static_cast<int>(tokens.size()-1);

				if( (vertexesPerFace>3) && OpenMeshType::FaceType::HasPolyInfo() ){
			//_BEGIN___ if  you are loading a GENERIC POLYGON mesh 
		  
					ff.set(vertexesPerFace);

					std::string vertex;
					std::string texcoord;
					std::string normal;
					for(int i=0;i<vertexesPerFace;++i) // remember index starts from 1 instead of 0
					  SplitToken(tokens[i+1], ff.v[i], ff.n[i], ff.t[i], oi.mask);

		 if ( oi.mask & vcg::tri::io::Mask::IOM_WEDGTEXCOORD )
				{
					// verifying validity of texture coords indices
		          for(int i=0;i<vertexesPerFace;i++)
				  if(!GoodObjIndex(ff.t[i],oi.numTexCoords)) 
				   return E_BAD_VERT_TEX_INDEX;

					ff.tInd=materials[currentMaterialIdx].index;
				}
				
				// verifying validity of vertex indices
				std::vector<int> tmp = ff.v;
				std::sort(tmp.begin(),tmp.end());
				std::unique(tmp.begin(),tmp.end());
				if(tmp.size() != ff.v.size())
					result = E_VERTICES_WITH_SAME_IDX_IN_FACE;

        for(int i=0;i<vertexesPerFace;i++)
            if(!GoodObjIndex(ff.v[i],numVertices))
              return E_BAD_VERT_INDEX;
					
				// assigning face normal
				// ---------------------
				if ( oi.mask & vcg::tri::io::Mask::IOM_WEDGNORMAL )
				{
					// verifying validity of vertex normal indices
					// -------------------------------------------
          for(int i=0;i<vertexesPerFace;i++)
            if(!GoodObjIndex(ff.n[i],numVNormals)) return E_BAD_VERT_NORMAL_INDEX;
				}

				// assigning face color
				// --------------------
				if( oi.mask & vcg::tri::io::Mask::IOM_FACECOLOR)		
					 ff.c = currentColor;
				
				++numTriangles;
		        indexedFaces.push_back(ff);
   
				
				// callback invocation, abort loading process if the call returns false
				if ((cb !=NULL)&& (((numTriangles + numVertices)%100)==0) )
					{
					  if (!(*cb)( (100*(numTriangles +numVertices))/ numVerticesPlusFaces, "Face Loading"))
					  return E_ABORTED;
					}
			//_END  ___ if  you are loading a GENERIC POLYGON mesh 
				}else
				{
			//_BEGIN___ if  you are loading a  TRIMESH mesh 
				ff.set(3);	
			  std::string vertex;
			  std::string texcoord;
				std::string normal;
        for(int i=0;i<3;++i) // remember index starts from 1 instead of 0
           SplitToken(tokens[i+1], ff.v[i], ff.n[i], ff.t[i], oi.mask);

        if ( oi.mask & vcg::tri::io::Mask::IOM_WEDGTEXCOORD )
				{
					// verifying validity of texture coords indices
          for(int i=0;i<3;i++)
            if(!GoodObjIndex(ff.t[i],oi.numTexCoords)) 
              return E_BAD_VERT_TEX_INDEX;

					ff.tInd=materials[currentMaterialIdx].index;
				}
				
				// verifying validity of vertex indices
				if ((ff.v[0] == ff.v[1]) || (ff.v[0] == ff.v[2]) || (ff.v[1] == ff.v[2]))
					       result = E_VERTICES_WITH_SAME_IDX_IN_FACE;

        for(int i=0;i<3;i++)
            if(!GoodObjIndex(ff.v[i],numVertices))
              return E_BAD_VERT_INDEX;
					
				// assigning face normal
				// ---------------------
				if ( oi.mask & vcg::tri::io::Mask::IOM_WEDGNORMAL )
				{
					// verifying validity of vertex normal indices
					// -------------------------------------------
          for(int i=0;i<3;i++)
            if(!GoodObjIndex(ff.n[i],numVNormals)) return E_BAD_VERT_NORMAL_INDEX;
				}

				// assigning face color
				// --------------------
				if( oi.mask & vcg::tri::io::Mask::IOM_FACECOLOR)		
          ff.c = currentColor;
				
				// by default there are no internal edge
				ff.edge[0]=ff.edge[1]=ff.edge[2]=false;
				if(vertexesPerFace>3) ff.edge[2]=true;
				++numTriangles;
        indexedFaces.push_back(ff);
        	/*
					// A face polygon composed of more than three vertices is triangulated
					// according to the following schema:
					//                     v5
					//                    /  \   
					//                   /    \    
					//                  /      \   
					//                 v1------v4 
					//                 |\      /
					//                 | \    /
					//                 |  \  /
					//                v2---v3
					//
					// As shown above, the 5 vertices polygon (v1,v2,v3,v4,v5)
					// has been split into the triangles (v1,v2,v3), (v1,v3,v4) e (v1,v4,v5).
					// This way vertex v1 becomes the common vertex of all newly generated
					// triangles, and this may lead to the creation of very thin triangles.
					*/

				int iVertex = 3;
				while (iVertex < vertexesPerFace)  // add other triangles
				{
					ObjIndexedFace ffNew=ff;
          			int v4_index;
					int vt4_index;
					int vn4_index;

					SplitToken(tokens[++iVertex], v4_index, vn4_index, vt4_index, oi.mask);
					if(!GoodObjIndex(v4_index, numVertices))
						return E_BAD_VERT_INDEX;
					
					// assigning wedge texture coordinates
					// -----------------------------------
					if( oi.mask & vcg::tri::io::Mask::IOM_WEDGTEXCOORD )
					{
						// verifying validity of texture coords index
						// ------------------------------------------
            if(!GoodObjIndex(vt4_index,oi.numTexCoords))
              return E_BAD_VERT_TEX_INDEX;
						
				    if ( oi.mask & vcg::tri::io::Mask::IOM_WEDGNORMAL )
                if(!GoodObjIndex(vn4_index,numVNormals))
                  return E_BAD_VERT_NORMAL_INDEX;
            
         		ffNew.t[1]=ff.t[2];
            ffNew.t[2]=vt4_index;            
					}
					
					if ((ff.v[0] == v4_index) || (ff.v[2] == v4_index)) result = E_VERTICES_WITH_SAME_IDX_IN_FACE;
					ffNew.v[1]=ff.v[2];
          ffNew.v[2]=v4_index;            

					// assigning face normal
					// ---------------------
					if ( oi.mask & vcg::tri::io::Mask::IOM_WEDGNORMAL )
					{									  
            ffNew.n[1]=ff.n[2];
            ffNew.n[2]=vn4_index;            
					}
					// Setting internal edges: edge 1 (the opposite to vertex 0) is always an external edge. 
          ffNew.edge[0]=true;
					ffNew.edge[1]=false;
					if(iVertex < vertexesPerFace) ffNew.edge[2]=true;
																	else	ffNew.edge[2]=false;
					++numTriangles;
          ++extraTriangles;
          indexedFaces.push_back(ffNew);
					ff.v[2] = v4_index;
				}
				
				// callback invocation, abort loading process if the call returns false
				if ((cb !=NULL)&& (((numTriangles + numVertices)%100)==0) )
					{
					  if (!(*cb)( (100*(numTriangles +numVertices))/ numVerticesPlusFaces, "Face Loading"))
					  return E_ABORTED;
					}
			//_END___ if  you are loading a  TRIMESH mesh 

				}
			}
			else if (header.compare("mtllib")==0)	// material library
			{
				// obtain the name of the file containing materials library
				std::string materialFileName = tokens[1];
				if (!LoadMaterials( materialFileName.c_str(), materials, m.textures))
					result = E_MATERIAL_FILE_NOT_FOUND;
			}
			else if (header.compare("usemtl")==0)	// material usage
			{
				std::string materialName = tokens[1];
				bool found = false;
				unsigned i = 0;
				while (!found && (i < materials.size()))
				{
					std::string currentMaterialName = materials[i].materialName;
					if (currentMaterialName == materialName)
					{
						currentMaterialIdx = i;
					  Material &material = materials[currentMaterialIdx];
					  Point3f diffuseColor = material.Kd;
					  unsigned char r			= (unsigned char) (diffuseColor[0] * 255.0);
					  unsigned char g			= (unsigned char) (diffuseColor[1] * 255.0);
					  unsigned char b			= (unsigned char) (diffuseColor[2] * 255.0);
					  unsigned char alpha = (unsigned char) (material.Tr  * 255.0);
					  currentColor= Color4b(r, g, b, alpha);
						found = true;
					}
					++i;
				}

				if (!found)
				{
					currentMaterialIdx = 0;
					result = E_MATERIAL_NOT_FOUND;
				}
			}
			// we simply ignore other situations
		} // end for each line...
	} // end while stream not eof
	assert((numTriangles +numVertices) == numVerticesPlusFaces+extraTriangles);

	FaceIterator   fi = Allocator<OpenMeshType>::AddFaces(m,numTriangles);
  //-------------------------------------------------------------------------------
	// Now the final pass to convert indexes into pointers for face to vert/norm/tex references
  for(int i=0;i<numTriangles;++i)
  {
    assert(m.face.size()==size_t(m.fn));
	m.face[i].Alloc(indexedFaces[i].v.size()); // it does not do anything if it is a trimesh
    for(int j=0;j<indexedFaces[i].v.size();++j)
    {
      m.face[i].V(j)=&(m.vert[indexedFaces[i].v[j]]);
      if ( oi.mask & vcg::tri::io::Mask::IOM_WEDGTEXCOORD ) {
          ObjTexCoord t = texCoords[indexedFaces[i].t[j]];
			    m.face[i].WT(j).u() = t.u;
			    m.face[i].WT(j).v() = t.v;
			    m.face[i].WT(j).n() = indexedFaces[i].tInd;
      }
		  if ( oi.mask & vcg::tri::io::Mask::IOM_WEDGNORMAL )
        m.face[i].WN(j).Import(normals[indexedFaces[i].n[j]]);		
			// set border flags according to internals faces
			if(indexedFaces[i].edge[j]) m.face[i].SetF(j);
						else m.face[i].ClearF(j);
    }
		if ( oi.mask & vcg::tri::io::Mask::IOM_FACECOLOR )
        m.face[i].C()=indexedFaces[i].c;
		if ( oi.mask & vcg::tri::io::Mask::IOM_WEDGNORMAL )
    					// face normal is computed as an average of wedge normals
	    m.face[i].N().Import(m.face[i].WN(0)+m.face[i].WN(1)+m.face[i].WN(2));
    				else	// computing face normal from position of face vertices
	  	face::ComputeNormalizedNormal(m.face[i]);
						
  }

  return result;
} // end of Open


	/*!
	* Read the next valid line and parses it into "tokens", allowing
	*	the tokens to be read one at a time.
	* \param stream	The object providing the input stream
	*	\param tokens	The "tokens" in the next line
	*/
	inline static const void TokenizeNextLine(std::ifstream &stream, std::vector< std::string > &tokens)
	{
		if(stream.eof()) return;
		std::string line;
		do
			std::getline(stream, line);
		while ((line[0] == '#' || line.length()==0) && !stream.eof());  // skip comments and empty lines
		
		if ((line[0] == '#') || (line.length() == 0))  // can be true only on last line of file
			return;

		size_t from		= 0; 
		size_t to			= 0;
		size_t length = line.size();
		tokens.clear();
		do
		{
			while (from!=length && (line[from]==' ' || line[from]=='\t' || line[from]=='\r') )
				from++;
      if(from!=length)
      {
				to = from+1;
				while (to!=length && line[to]!=' ' && line[to] != '\t' && line[to]!='\r')
					to++;
				tokens.push_back(line.substr(from, to-from).c_str());
				from = to;
      }
		}
		while (from<length);
	} // end TokenizeNextLine

inline static const void SplitToken(std::string token, int &vId, int &nId, int &tId, int mask)
{
  		  std::string vertex;
			  std::string texcoord;
				std::string normal;
 				if( ( mask & Mask::IOM_WEDGTEXCOORD ) && (mask & Mask::IOM_WEDGNORMAL) )   SplitVVTVNToken(token, vertex, texcoord, normal);
 				if(!( mask & Mask::IOM_WEDGTEXCOORD ) && (mask & Mask::IOM_WEDGNORMAL) )   SplitVVNToken(token, vertex, normal);
 				if( ( mask & Mask::IOM_WEDGTEXCOORD ) &&!(mask & Mask::IOM_WEDGNORMAL) )   SplitVVTToken(token, vertex, texcoord);
 				if(!( mask & Mask::IOM_WEDGTEXCOORD ) &&!(mask & Mask::IOM_WEDGNORMAL) )   SplitVToken(token, vertex);

          vId	= atoi(vertex.c_str())-1;
          if(mask & Mask::IOM_WEDGTEXCOORD) tId = atoi(texcoord.c_str())-1;
					if(mask & Mask::IOM_WEDGNORMAL)   nId= atoi(normal.c_str())-1;
}

inline static const void SplitVToken(std::string token, std::string &vertex) 
{
  vertex=token; 
}

	inline static const void SplitVVTToken(std::string token, std::string &vertex, std::string &texcoord)
	{
		vertex.clear();
		texcoord.clear();

		size_t from		= 0; 
		size_t to			= 0;
		size_t length = token.size();
		
		if(from!=length)
    {
			char c = token[from];
			vertex.push_back(c);

			to = from+1;
			while (to!=length && ((c = token[to]) !='/'))
			{
				vertex.push_back(c);
				++to;
			}
			++to;
			while (to!=length && ((c = token[to]) !=' '))
			{
				texcoord.push_back(c);
				++to;
			}
		}
	}	// end of SplitVVTToken

	inline static const void SplitVVNToken(std::string token, std::string &vertex, std::string &normal)
	{
		vertex.clear();
		normal.clear();

		size_t from		= 0; 
		size_t to			= 0;
		size_t length = token.size();
		
		if(from!=length)
    {
			char c = token[from];
			vertex.push_back(c);

			to = from+1;
			while (to!=length && ((c = token[to]) !='/'))
			{
				vertex.push_back(c);
				++to;
			}
			++to;
			++to;  // should be the second '/'
			while (to!=length && ((c = token[to]) !=' '))
			{
				normal.push_back(c);
				++to;
			}
		}
	}	// end of SplitVVNToken

	inline static const void SplitVVTVNToken(std::string token, std::string &vertex, std::string &texcoord, std::string &normal)
	{
		vertex.clear();
		texcoord.clear();
		normal.clear();
		
		size_t from		= 0; 
		size_t to			= 0;
		size_t length = token.size();
		
		if(from!=length)
    {
			char c = token[from];
			vertex.push_back(c);

			to = from+1;
			while (to!=length && ((c = token[to]) !='/'))
			{
				vertex.push_back(c);
				++to;
			}
			++to;
			while (to!=length && ((c = token[to]) !='/'))
			{
				texcoord.push_back(c);
				++to;
			}
			++to;
			while (to!=length && ((c = token[to]) !=' '))
			{
				normal.push_back(c);
				++to;
			}
		}
	}	// end of SplitVVTVNToken

	/*!
	* Retrieves infos about kind of data stored into the file and fills a mask appropriately
	* \param filename The name of the file to open
	*	\param mask	A mask which will be filled according to type of data found in the object
	* \param oi A structure which will be filled with infos about the object to be opened
  	*/

static bool LoadMask(const char * filename, Info &oi)
{
 		std::ifstream stream(filename);
		if (stream.fail()) return false;

		 // obtain length of file:
    stream.seekg (0, std::ios::end);
		int length = stream.tellg();
    stream.seekg (0, std::ios::beg);
    
    if (length == 0) return false;

    bool bHasPerFaceColor			= false;

    oi.numVertices=0;
    oi.numFaces=0;
    oi.numTexCoords=0;
		int lineCount=0;
		int totRead=0;
    std::string line;
	  while (!stream.eof())
    {
      lineCount++;
			std::getline(stream, line);
      totRead+=line.size();
      if(oi.cb && (lineCount%1000)==0) 
					(*oi.cb)( (int)(100.0*(float(totRead))/float(length)), "Loading mask...");
			if(line.size()>2)
      {
        if(line[0]=='v')
        {
          if(line[1]==' ') oi.numVertices++;
          if(line[1]=='t') oi.numTexCoords++;
        } 
        else 
         if(line[0]=='f') oi.numFaces++;
				 else 
				 if(line[0]=='u' && line[1]=='s') bHasPerFaceColor = true; // there is a usematerial so add per face color
      }
		}
		oi.mask = 0;
		if (oi.numTexCoords)	
			{
				oi.mask |= vcg::tri::io::Mask::IOM_WEDGTEXCOORD;
				// Usually if you have tex coords you also have materials
				oi.mask |= vcg::tri::io::Mask::IOM_FACECOLOR; 
			}
  if(bHasPerFaceColor) 				oi.mask |= vcg::tri::io::Mask::IOM_FACECOLOR; 


  return true;
}

static bool LoadMask(const char * filename, int &mask)
{
  Info oi;
  bool ret=LoadMask(filename, oi);
  mask= oi.mask;
  return ret;
}

	static bool LoadMaterials(const char * filename, std::vector<Material> &materials, std::vector<std::string> &textures)
	{
		// assumes we are in the right directory

		std::ifstream stream(filename);
		if (stream.fail())
			return false;
    
		std::vector< std::string > tokens;
		std::string	header;
	
		materials.clear();
		Material currentMaterial;
		
		bool first = true;
		while (!stream.eof())
		{
			tokens.clear();
			TokenizeNextLine(stream, tokens);
		
			if (tokens.size() > 0)
			{
				header.clear();
				header = tokens[0];

				if (header.compare("newmtl")==0)
				{
					if (!first)
					{
						materials.push_back(currentMaterial);
						currentMaterial = Material();
					}
					else
						first = false;
					//strcpy(currentMaterial.name, tokens[1].c_str());
          if(tokens.size() < 2) 
						return false; 
					currentMaterial.materialName=tokens[1];
				}
				else if (header.compare("Ka")==0)
				{
					float r = (float) atof(tokens[1].c_str());
					float g = (float) atof(tokens[2].c_str());
					float b = (float) atof(tokens[3].c_str());

					currentMaterial.Ka = Point3f(r, g, b); 
				}
				else if (header.compare("Kd")==0)
				{
					float r = (float) atof(tokens[1].c_str());
					float g = (float) atof(tokens[2].c_str());
					float b = (float) atof(tokens[3].c_str());

          currentMaterial.Kd = Point3f(r, g, b); 
				}
				else if (header.compare("Ks")==0)
				{
					float r = (float) atof(tokens[1].c_str());
					float g = (float) atof(tokens[2].c_str());
					float b = (float) atof(tokens[3].c_str());

          currentMaterial.Ks = Point3f(r, g, b); 
				}
				else if (	(header.compare("d")==0) ||
									(header.compare("Tr")==0)	)	// alpha
				{
          currentMaterial.Tr = (float) atof(tokens[1].c_str());
				}
				else if (header.compare("Ns")==0)  // shininess        
				{
					currentMaterial.Ns = atoi(tokens[1].c_str());
				}
				else if (header.compare("illum")==0)	// specular illumination on/off
				{
					int illumination = atoi(tokens[1].c_str());
          //currentMaterial.bSpecular = (illumination == 2);
          currentMaterial.illum = illumination;
				}
				else if( (header.compare("map_Kd")==0)	|| (header.compare("map_Ka")==0) ) // texture name
				{
					std::string textureName = tokens[1];
					//strcpy(currentMaterial.textureFileName, textureName.c_str());
					 currentMaterial.map_Kd=textureName;
					
					// adding texture name into textures vector (if not already present)
					// avoid adding the same name twice
					bool found = false;
					unsigned int size = static_cast<unsigned int>(textures.size());
					unsigned j = 0;
					while (!found && (j < size))
					{
						if (textureName.compare(textures[j])==0)
						{
							currentMaterial.index = (int)j;
							found = true;
						}
						++j;
					}
					if (!found)
					{
						textures.push_back(textureName);
						currentMaterial.index = (int)size;
					}
				}
				// we simply ignore other situations
			}
		}
		materials.push_back(currentMaterial);  // add last read material

		stream.close();

		return true;
	}

}; // end class
} // end Namespace tri
} // end Namespace io
} // end Namespace vcg

#endif  // ndef __VCGLIB_IMPORT_OBJ
