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
Revision 1.29  2005/02/14 17:11:07  ponchio
aggiunta delle sphere

Revision 1.28  2005/02/08 12:43:03  ponchio
Added copyright


****************************************************************************/

#ifndef WIN32
#include <fcntl.h>
#endif

#include <map>
#include <queue>

#include <GL/glew.h>

#include <ptypes/pasync.h>

#include "nexusmt.h"

using namespace nxs;
using namespace vcg;
using namespace std;

void Stats::Start() {
  tri = extr = 0;
  watch.Start();
}

void Stats::Disk(float _disk) {
  disk.push_front(_disk);
  if(disk.size() > log_size) disk.pop_back();
}

void Stats::Error(float _error) {
  error.push_front(_error);
  if(error.size() > log_size) error.pop_back();
}

void Stats::Stop() {
  time.push_front((float)watch.Time());
  if(time.size() > log_size) time.pop_back();
  fps = (7*fps + 1/time[0])/8.0f;
}

NexusMt::NexusMt() {
  preload.mt = this;
  preload.start();
}

NexusMt::~NexusMt() {
  preload.signal();
  preload.waitfor();
}

void NexusMt::SetPreload(bool on) {
  if(on == (preload.get_running() && !preload.get_finished())) return;
  if(on) preload.start();
  else {
    preload.signal();
    //    preload.waitfor();
  }
}

bool NexusMt::Load(const string &filename) {
  if(!Nexus::Load(filename, true)) return false;
  if(!history.IsQuick() && !history.UpdatesToQuick())
    return false;

#ifndef WIN32
  //i will read data only once usually.
  //  for(unsigned int i = 0; i < files.size(); i++) {
  //  int fd = fileno(files[i]->fp);
  //  posix_fadvise(fd, 0, 0, POSIX_FADV_NOREUSE);
  // }
  
#endif
  return true;
}

bool NexusMt::InitGL(bool vbo) {
  use_vbo = vbo;

  GLenum ret = glewInit();
  if(ret != GLEW_OK) return false;
  if(vbo && !GLEW_ARB_vertex_buffer_object) {
    cerr << "No vbo available!" << endl;
    use_vbo = false;
  }
  return true;
}

void NexusMt::Render(DrawContest contest) {
  Extraction extraction;
  extraction.frustum.GetView();
  extraction.metric->GetView();
  extraction.Extract(this);
  Render(extraction, contest);
}


void NexusMt::Render(Extraction &extraction, DrawContest &contest,
		     Stats *stats) {
  static ::GLUquadricObj *  spr = gluNewQuadric();

  for(unsigned int i = 0; i < heap.size(); i++) {
    Item &item = heap[i];
    if(!extraction.errors.count(item.id)) {
      item.error = 1e20;
    } else
      item.error = extraction.errors[item.id];
  }
  make_heap(heap.begin(), heap.end());

  preload.post(extraction.selected);

  glEnableClientState(GL_VERTEX_ARRAY);
  if((signature & NXS_COLORS) && (contest.attrs & DrawContest::COLOR))
    glEnableClientState(GL_COLOR_ARRAY);
  if((signature & NXS_NORMALS_SHORT) && (contest.attrs & DrawContest::NORMAL))
    glEnableClientState(GL_NORMAL_ARRAY);

  vector<Item> skipped;
  
  for(unsigned int i = 0; i < extraction.draw_size; i++) {
    unsigned int patch = extraction.selected[i].id;
    Entry &entry = operator[](patch);
    vcg::Sphere3f &sphere = entry.sphere;

    if(stats) stats->extr += entry.nface;

    if(extraction.frustum.IsOutside(sphere.Center(), sphere.Radius())) 
      continue;

    if(stats) stats->tri += entry.nface;

    if(!entry.patch) {
      skipped.push_back(extraction.selected[i]);
      continue;
    }
    Draw(patch, contest);
  }


  preload.trigger.reset();
  preload.lock.enter();  

  //  if(skipped.size()) cerr << "Skipped: " << skipped.size() << endl;

  for(vector<Item>::iterator i = skipped.begin(); i != skipped.end(); i++) {
    GetPatch((*i).id, (*i).error);
    Draw((*i).id, contest);
  }
  Flush(false); //in case there are no skipped... :P

  if(stats) {
    stats->Error(extraction.max_error);
    stats->Disk(preload.disk);
    preload.disk = 0;
  }
  

  preload.trigger.post();
  preload.lock.leave();



  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
}

void NexusMt::Draw(unsigned int cell, DrawContest &contest) { 
  static ::GLUquadricObj *  spr = gluNewQuadric();

  Entry &entry = operator[](cell);
  Patch &patch = *(entry.patch);
  char *fstart;
  char *vstart;
  char *cstart;
  char *nstart;

  if(contest.attrs & DrawContest::SPHERES){
    vcg::Sphere3f &sphere = entry.sphere;
    glPushAttrib(GL_POLYGON_BIT);
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    glPushMatrix();
    glTranslatef(sphere.Center().X(),sphere.Center().Y(),sphere.Center().Z());
    gluSphere(spr,sphere.Radius(),15,15);
    glPopMatrix();
    glPopAttrib();
  }

  
  if(use_vbo) {
    if(!entry.vbo_element) 
      LoadVbo(entry);
    
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, entry.vbo_array);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, entry.vbo_element);
    
    fstart = NULL;
    vstart = NULL;
    cstart = (char *)(sizeof(float) * patch.cstart);
    nstart = (char *)(sizeof(float) * patch.nstart);
  } else {
    fstart = (char *)patch.FaceBegin();
    vstart = (char *)patch.VertBegin();
    cstart = (char *)patch.ColorBegin();
    nstart = (char *)patch.Norm16Begin();
  }
  
  glVertexPointer(3, GL_FLOAT, 0, vstart);
  if(contest.attrs & DrawContest::COLOR)
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, cstart);
  if(contest.attrs & DrawContest::NORMAL)
    glNormalPointer(GL_SHORT, 8, nstart);  
  

  
  switch(contest.mode) {
  case DrawContest::POINTS:
    glDrawArrays(GL_POINTS, 0, patch.nv); break;
  case DrawContest::PATCHES:
    glColor3ub((cell * 27)%225 + 30, (cell * 37)%225 + 30, (cell * 87)%225 + 30);
  case DrawContest::SMOOTH:
    if(signature & NXS_FACES)
      glDrawElements(GL_TRIANGLES, patch.nf * 3, 
		     GL_UNSIGNED_SHORT, fstart);
    else if(signature & NXS_STRIP)
      glDrawElements(GL_TRIANGLE_STRIP, patch.nf, 
		     GL_UNSIGNED_SHORT, fstart);
    break;
  case DrawContest::FLAT:
    if(use_vbo) {
      cerr << "Mode incompatible with VBO\n";
      exit(0);
    }
    if(signature & NXS_FACES) {
      glBegin(GL_TRIANGLES);
      unsigned short *f = patch.Face(0);
      for(int i = 0; i < patch.nf; i++) {

	Point3f &p0 = patch.Vert(f[0]);
	Point3f &p1 = patch.Vert(f[1]);
	Point3f &p2 = patch.Vert(f[2]);
	Point3f n = ((p1 - p0) ^ (p2 - p0));
	glNormal3f(n[0], n[1], n[2]);
	glVertex3f(p0[0], p0[1], p0[2]);
	glVertex3f(p1[0], p1[1], p1[2]);
	glVertex3f(p2[0], p2[1], p2[2]);
	f += 3;
      }
      glEnd();
    } else if(signature & NXS_STRIP) {
      cerr << "Unsupported rendering mode sorry\n";
      exit(0);
    }
    break;
  default: 
    cerr << "Unsupported rendering mode sorry\n";
    exit(0);
    break;
  }
}

Patch &NexusMt::GetPatch(unsigned int patch, float error, bool flush) { 
  Entry &entry = operator[](patch);
  if(entry.patch) return *(entry.patch);

  while(flush && ram_used > ram_max) {
    if(heap[0].error == 0) break;
    unsigned int to_flush = heap[0].id;
    pop_heap(heap.begin(), heap.end());
    heap.pop_back();
    FlushPatch(to_flush);
  }
  entry.patch = LoadPatch(patch);
  heap.push_back(Item(patch, error));
  push_heap(heap.begin(), heap.end());
  return *(entry.patch);
}

void NexusMt::Flush(bool all) {
  if(all) {
    for(unsigned int i = 0; i < heap.size(); i++) {
      unsigned int patch = heap[i].id;
      FlushPatch(patch);
    }
    heap.clear();
  } else {
    while(heap.size() && ram_used > ram_max) {    
      if(heap[0].error == 0) break;
      unsigned int to_flush = heap[0].id;
      pop_heap(heap.begin(), heap.end());
      heap.pop_back();
      FlushPatch(to_flush);
    }
  }
}

bool NexusMt::CanAdd(Item &item) {
  if(!heap.size()) return true;
  Entry &entry = operator[](item.id);
  if(ram_used + entry.ram_size < ram_max)
    return true;
  return heap[0].error > item.error;
}

void NexusMt::FlushPatch(unsigned int id) {
  Entry &entry = operator[](id);
  if(entry.vbo_element)
    FlushVbo(entry);

  if(entry.patch->start)
    delete [](entry.patch->start);
  delete entry.patch;  
  entry.patch = NULL;    
  ram_used -= entry.ram_size;      
}

void NexusMt::LoadVbo(Entry &entry) { 
  assert(entry.vbo_element == 0);
  //  if(entry.vbo_element) return;

  Patch &patch  = *entry.patch;    
  unsigned int size = patch.nf * sizeof(unsigned short);
  if((signature & NXS_FACES) != 0) size *= 3;
  
  glGenBuffersARB(1, &entry.vbo_element);
  assert(entry.vbo_element);
  glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, entry.vbo_element);
  glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, size, patch.FaceBegin(),
		  GL_STATIC_DRAW_ARB);
  vbo_used += size;
  
  //TODO fix this when we allow data :p
  size = sizeof(float) * patch.dstart;
    
  glGenBuffersARB(1, &entry.vbo_array);
  assert(entry.vbo_array);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, entry.vbo_array);
  glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, patch.VertBegin(), 
		  GL_STATIC_DRAW_ARB);
    
  vbo_used += size;
  delete [](entry.patch->start);
  entry.patch->start = NULL;
}

void NexusMt::FlushVbo(Entry &entry) {
  if(!entry.vbo_element) return;
  glDeleteBuffersARB(1, &entry.vbo_element);
  glDeleteBuffersARB(1, &entry.vbo_array);
  entry.vbo_element = 0;
  entry.vbo_array = 0;

  Patch &patch  = *entry.patch; 
  vbo_used -= patch.nf * sizeof(unsigned short);
  vbo_used -= sizeof(float) * patch.dstart;
}

//Kept for historical reasons.
/*void NexusMt::ExtractFixed(vector<unsigned int>  &selected, float error) {
  std::vector<Node>::iterator n;
  for(n = nodes.begin(); n != nodes.end(); n++)
    (*n).visited = false;
    
  std::queue<Node *> qnodo;
  qnodo.push(&nodes[0]);
  nodes[0].visited = true;
    
  for( ; !qnodo.empty(); qnodo.pop()) {
    Node &node = *qnodo.front();
      
    std::vector<Frag>::iterator fragment;
    std::vector<Node *>::iterator on;
    for(on = node.out.begin(), fragment = node.frags.begin(); 
	    on != node.out.end(); ++on, ++fragment) {

      if((*on)->visited) continue;
	
      if(error < (*on)->error) { //need to expand this node.
	      qnodo.push(*on);
	      (*on)->visited = 1;
      } else {
	      vector<unsigned int>::iterator cell;
	      for(cell=(*fragment).begin(); cell != (*fragment).end(); ++cell) 	        selected.push_back(*cell);                   
      }
    }
  }  
} */   
