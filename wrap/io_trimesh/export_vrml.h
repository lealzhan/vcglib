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



/** Function to save in Inventor format file.
	@param filename Name of the new inventor file
*/
bool Save_VRML2(const char * filename, int savemask = SM_ALL )
{
	FILE *fp;

	fp = fopen(filename,"wb");
	if(fp==0)
		return false;

		// Header
	fprintf(fp,
		"#VRML V2.0 utf8\n"
		"\n"
		"# Generated by VCGLIB, (C)Copyright 1999-2001 VCG, IEI-CNR\n"
		"\n"
		"NavigationInfo {\n"
		"	type [ \"EXAMINE\", \"ANY\" ]\n"
		"}\n"
	);

		// Trans principale
	double ss = 8.0/bbox.Diag();

	fprintf(fp,
    	"Transform {\n"
		"  scale %g %g %g\n"
		"  translation %g %g %g\n"
		"  children\n"
		"  [\n"
		,ss
		,ss
		,ss
		,  -bbox.Center()[0]*ss
		,  -bbox.Center()[1]*ss
		,-3-bbox.Center()[2]*ss	
	);

		// Start Shape
	fprintf(fp,
		"    Shape\n"
		"    {\n"
		"      geometry IndexedFaceSet\n"
		"      {\n"
		"        creaseAngle .5\n"
		"        solid FALSE\n"
		"        coord Coordinate\n"
		"        {\n"
		"          point\n"
		"          ["
	);

	
	face_iterator fi;
	vertex_iterator vi;
	map<vertex_pointer,int> index;
	int ind;
			// Vertici
	for(ind=0,vi=vert.begin(); vi!=vert.end(); ++vi,++ind)
	{
		if(ind%4==0)
			fprintf(fp,"\n            ");

		fprintf(fp,"%g %g %g, "
			,(*vi).P()[0]
			,(*vi).P()[1]
			,(*vi).P()[2]
		);
		index[&*vi] = ind;
	}

	fprintf(fp,
		"\n"
		"          ]\n"
		"        }\n"
	);

	if( ( savemask & SM_VERTCOLOR ) && (vertex_type::OBJ_TYPE & vertex_type::OBJ_TYPE_C) )
	{
		fprintf(fp,
			"        color Color\n"
			"        {\n"
			"          color\n"
			"          ["
		);

		for(ind=0,vi=vert.begin();vi!=vert.end();++vi,++ind)
		{
			
			float r = float(vi->C()[0])/255;
			float g = float(vi->C()[1])/255;
			float b = float(vi->C()[2])/255;

			if(ind%4==0)
				fprintf(fp,"\n            ");

			fprintf(fp,
				"%g %g %g,"
				,r,g,b
			);
		}

		fprintf(fp,
			"\n"
			"          ]\n"
			"        }\n"
			"        colorIndex\n"
			"        ["
		);

		for(ind=0,fi=face.begin(); fi!=face.end(); ++fi,++ind)
		{
			if(ind%4==0)
				fprintf(fp,"\n          ");
			for (int j = 0; j < 3; j++)
				fprintf(fp,"%i,",index[(*fi).V(j)]);
			fprintf(fp,"-1,");
		}

		fprintf(fp,
			"\n"
			"        ]\n"
		);
	}
	else if( ( savemask & SM_WEDGCOLOR ) && (face_type::OBJ_TYPE & face_type::OBJ_TYPE_WC) )
	{
		fprintf(fp,
			"        color Color\n"
			"        {\n"
			"          color\n"
			"          ["
		);

		for(ind=0,fi=face.begin();fi!=face.end();++fi,++ind)
		{
			for(int z=0;z<3;++z)
			{
				float r = float(fi->WC(z)[0])/255;
				float g = float(fi->WC(z)[1])/255;
				float b = float(fi->WC(z)[2])/255;

				if(ind%4==0)
					fprintf(fp,"\n            ");

				fprintf(fp,
					"%g %g %g,"
					,r,g,b
				);
			}
		}

		fprintf(fp,
			"\n"
			"          ]\n"
			"        }\n"
			"        colorIndex\n"
			"        ["
		);

		int nn = 0;
		for(ind=0,fi=face.begin(); fi!=face.end(); ++fi,++ind)
		{
			if(ind%4==0)
				fprintf(fp,"\n          ");
			for (int j = 0; j < 3; j++)
			{
				fprintf(fp,"%i,",nn);
				++nn;
			}
			fprintf(fp,"-1,");
		}

		fprintf(fp,
			"\n"
			"        ]\n"
		);
	}
	else if( ( savemask & SM_WEDGTEXCOORD ) && (face_type::OBJ_TYPE & face_type::OBJ_TYPE_WT) )
	{
		fprintf(fp,
			"\n"
			"        texCoord TextureCoordinate\n"
			"        {\n"
			"          point\n"
			"          [\n"
		);

		for(ind=0,fi=face.begin(); fi!=face.end(); ++fi,++ind)
		{
			if(ind%4==0)
				fprintf(fp,"\n          ");
			for (int j = 0; j < 3; j++)
			{
				fprintf(fp,"%g %g, "
					,fi->WT(j).u()
					,fi->WT(j).v()
				);
			}
		}

		fprintf(fp,
			"\n"
			"          ]\n"
			"        }\n"
			"        texCoordIndex\n"
			"        [\n"
		);

		int nn = 0;
		for(ind=0,fi=face.begin(); fi!=face.end(); ++fi,++ind)
		{
			if(ind%4==0)
				fprintf(fp,"\n          ");
			for (int j = 0; j < 3; j++)
			{
				fprintf(fp,"%d,",nn);
				++nn;
			}
			fprintf(fp,"-1,");
		}

		fprintf(fp,
			"\n"
			"        ]\n"
		);
	}


	fprintf(fp,
		"        coordIndex\n"
		"        ["
	);
			// Facce
	for(ind=0,fi=face.begin(); fi!=face.end(); ++fi,++ind)
	{
		if(ind%6==0)
		{
			fprintf(fp,"\n          ");
		}

		for (int j = 0; j < 3; j++)
			fprintf(fp,"%i,",index[(*fi).V(j)]);
		fprintf(fp,"-1,");
	}

	fprintf(fp,
	    "\n"
		"        ]\n"
		"      }\n"
		"      appearance Appearance\n"
		"      {\n"
		"        material Material\n"
		"        {\n"
		"	       ambientIntensity 0.2\n"
		"	       diffuseColor 0.9 0.9 0.9\n"
		"	       specularColor .1 .1 .1\n"
		"	       shininess .5\n"
		"        }\n"
	);

	if(textures.size())
	{
		fprintf(fp,	
			"        texture ImageTexture { url \"%s\" }\n"
			,textures[0]
		);
	}

	fprintf(fp,
		"      }\n"
	    "    }\n"
	    "  ]\n"
	    "}\n"
	);

	fclose(fp);

	return true;
}
