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
Revision 1.10  2004/09/30 23:56:33  ponchio
Backup (added strips and normals)

Revision 1.9  2004/07/20 14:04:32  ponchio
Improved efficience in operator[]

Revision 1.8  2004/07/15 14:32:49  ponchio
Debug.

Revision 1.7  2004/07/05 15:49:39  ponchio
Windows (DevCpp, mingw) port.

Revision 1.6  2004/07/04 15:23:48  ponchio
Debug

Revision 1.5  2004/07/02 17:41:37  ponchio
Debug.

Revision 1.4  2004/07/02 13:02:39  ponchio
Added GetRegion, Read and Write

Revision 1.3  2004/07/01 21:36:54  ponchio
*** empty log message ***

Revision 1.2  2004/06/25 16:47:13  ponchio
Various debug

Revision 1.1  2004/06/24 14:32:45  ponchio
Moved from wrap/nexus

Revision 1.3  2004/06/22 15:32:09  ponchio
Tested

Revision 1.2  2004/06/22 10:27:16  ponchio
*** empty log message ***

Revision 1.1  2004/06/22 00:39:56  ponchio
Created


****************************************************************************/

#ifndef VFILE_H
#define VFILE_H

#ifndef WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <assert.h>
#include <errno.h>
//#include <hash_map>
#include <map>
#include <list>
#include <string>

/**Vector structure on file with simulated mmapping.
 * a priority queue of buffers is used 
 * TODO: port to over 4Gb usable space
 *       some mechanism to report errors?
 *       use an Iterator?
 */

namespace nxs {

template <class T> class VFile {
 public:
  
  struct Buffer {
    unsigned int key;
    unsigned int size; //in number of elements
    T *data;
  };

 private:
#ifdef WIN32
   HANDLE fp;
#else
  FILE *fp;  
#endif

  std::list<Buffer> buffers;
  typedef typename std::list<Buffer>::iterator list_iterator;

  std::map<unsigned int, list_iterator> index;   //TODO move to hash_map 
  Buffer *last_buffer;

  unsigned int chunk_size; //default buffer size (expressed in number of T)
  unsigned int queue_size;
  unsigned int n_elements; //size of the vector

 public:
  class iterator {
  public:
    iterator(unsigned int p = 0, VFile *b = 0): n(p), buffer(b) {}
    T &operator*() { return (*buffer)[n]; }
    void operator++() { n++; }
    bool operator!=(const iterator &i) { return i.n != n; }
  private:
    unsigned int n;
    VFile *buffer;
  };
  
  VFile(): fp(NULL), last_buffer(NULL) {}
  ~VFile() { Close(); }                    
  bool Create(const std::string &filename, 
	      unsigned int _chunk_size = 4096/sizeof(T), 
	      unsigned int _queue_size = 1000) {

    assert(_chunk_size > 0);
    last_buffer = NULL;
    chunk_size = _chunk_size;
    queue_size = _queue_size;
    n_elements = 0;    

#ifdef WIN32
    fp = CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                    0, NULL);
    if(fp == INVALID_HANDLE_VALUE) return false;
#else
    fp = fopen(filename.c_str(), "wb+");
    if(!fp) return false;        
#endif


    return true;
  }

  bool Load(const std:: string &filename, 
	    unsigned int _chunk_size = 4096/sizeof(T), 
	    unsigned int _queue_size = 1000) {

    assert(_chunk_size > 0);
    last_buffer = NULL;
    chunk_size = _chunk_size;
    queue_size = _queue_size;
#ifdef WIN32
    fp = CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                    0, NULL); 
    if(fp == INVALID_HANDLE_VALUE) return false;
#else
    fp = fopen(filename.c_str(), "rb+");
    if(!fp) return false;
#endif


#ifdef WIN32    
    n_elements = GetFileSize(fp, NULL)/ sizeof(T);
#else
    fseek(fp, 0, SEEK_END);
    n_elements = ftell(fp)/ sizeof(T);   
#endif
    return true;
  }

  void Close() {
    if(fp) {
      Flush();
#ifdef WIN32
      CloseHandle(fp);
#else
      fclose(fp);
#endif
      fp = NULL;
    }
  }

  void Flush() {
    list_iterator i;
    for(i = buffers.begin(); i != buffers.end(); i++)       
      FlushBuffer(*i);
    buffers.clear();
    index.clear();
    last_buffer = NULL;
  }

  void FlushBuffer(Buffer buffer) {
    SetPosition(buffer.key);
    WriteBuffer(buffer.data, buffer.size);
/*#ifdef WIN32
    SetFilePointer(fp, buffer.key * chunk_size * sizeof(T), FILE_BEGIN);
    unsigned int tmp;
    WriteFile(fp, buffer.data, sizeof(T) * buffer.size, &tmp, NULL);
    if(tmp != sizeof(T) * buffer.size)
      assert(0 && "Could not write");        
#else
    fseek(fp, buffer.key * chunk_size * sizeof(T), SEEK_SET);
    if(buffer.size != fwrite(buffer.data, sizeof(T), buffer.size, fp)) 
      assert(0 && "Could not write");    
#endif*/
    delete []buffer.data;
  }

  void Resize(unsigned int elem) {
    assert(fp);
    Flush();
    if(elem > n_elements) {
#ifdef WIN32
      if(INVALID_SET_FILE_POINTER == SetFilePointer(fp, elem*sizeof(T)-1, 0, FILE_BEGIN))
#else
      if(-1 == fseek(fp, elem*sizeof(T) -1, SEEK_SET)) 
#endif
	      assert(0 && "Could not resize");
      
      unsigned char a;
#ifdef WIN32
      DWORD tmp;
      WriteFile(fp, &a, 1, &tmp, NULL);
#else
      fwrite(&a, sizeof(unsigned char), 1, fp);
#endif
    } else {
      //TODO optimize: we do not need flush for buffers over elem.
      
#ifndef WIN32
      int fd = fileno(fp);
      ftruncate(fd, elem*sizeof(T));
#else
      SetFilePointer(fp, elem*sizeof(T), 0, FILE_BEGIN);
      SetEndOfFile(fp);
#endif
    }    
    n_elements = elem;
  }

  /** Remember that T is a valid pointer only until next call of
   * getElement or setElement
   */
  T &operator[](unsigned int n) { 

    assert(n < n_elements);

    unsigned int chunk = n/chunk_size;
    unsigned int offset = n - chunk*chunk_size;
    assert(offset < chunk_size * sizeof(T));

    if(last_buffer && last_buffer->key == chunk) 
        return *(last_buffer->data + offset);
        
    if(index.count(chunk)) {
      last_buffer = &*index[chunk];
      return *((*(index[chunk])).data + offset);
    }
    
    if(buffers.size() > queue_size) {
      Buffer &buffer= buffers.back();
      assert(buffer.key != chunk);
      FlushBuffer(buffer);      
      index.erase(buffer.key);  
      buffers.pop_back();
    }
    
    Buffer buffer;
    buffer.key = chunk;
    buffer.size = chunk_size;

    if(buffer.size + chunk * chunk_size > n_elements)
      buffer.size = n_elements - chunk * chunk_size;

    buffer.data = new T[buffer.size];  

    buffers.push_front(buffer);   
    index[buffer.key] = buffers.begin();   
    last_buffer = &*buffers.begin();
    
    SetPosition(chunk);
    ReadBuffer(buffer.data, buffer.size);    

    return *(buffer.data + offset);
  }

  /** you can get a region instead of an element but:
      1)region must be Chunk aligned.
      2)you get impredictable results if regions overlap or mix with operator[]
  */
  T *GetRegion(unsigned int start, unsigned int size, bool flush = true) {
    assert(start + size <= n_elements);
    assert((size % chunk_size) == 0);
    assert((start % chunk_size) == 0);
    if(size == 0) return NULL;
    
    unsigned int chunk = start/chunk_size;

    if(index.count(chunk)) 
      return ((*(index[chunk])).data);
    
    while(flush && buffers.size() > queue_size) {
      Buffer &buffer= buffers.back();
      FlushBuffer(buffer);      
      index.erase(buffer.key);  
      buffers.pop_back();
    }
    
    Buffer buffer;
    buffer.key = chunk;
    buffer.size = size;
    buffer.data = new T[buffer.size];  

    buffers.push_front(buffer);    
    index[chunk] = buffers.begin();   

    SetPosition(chunk);
    ReadBuffer(buffer.data, buffer.size);
    return buffer.data;
  } 

  void PushBack(const T &t) {
    Resize(n_elements+1);
    operator[](n_elements-1) = t;
  }
  
  unsigned int Size() { return n_elements; }
  unsigned int ChunkSize() { return chunk_size; }
  unsigned int QueueSize() { return queue_size; }
  iterator Begin() { return iterator(0, this); }
  iterator End() { return iterator(Size(), this); }

 protected:
  void SetPosition(unsigned int chunk) {
#ifdef WIN32
    SetFilePointer(fp, chunk * chunk_size * sizeof(T), 0, FILE_BEGIN);
#else
    fseek(fp, chunk * chunk_size * sizeof(T), SEEK_SET);
#endif
  }
  void ReadBuffer(T *data, unsigned int size) {
#ifdef WIN32
    DWORD tmp;
    ReadFile(fp, data, sizeof(T) * size, &tmp, NULL);
    if(tmp != sizeof(T) * size)
      assert(0 && "Could not read");        
#else    
    if(size != fread(data, sizeof(T), size, fp)) 
      assert(0 && "Could not read");    
#endif
  }
  void WriteBuffer(T *data, unsigned int size) {
#ifdef WIN32
    DWORD tmp;
    WriteFile(fp, data, sizeof(T) * size, &tmp, NULL);
    if(tmp != sizeof(T) * size)
      assert(0 && "Could not write");        
#else    
    if(size != fwrite(data, sizeof(T), size, fp)) 
      assert(0 && "Could not write");    
#endif
  }
};

}//namespace

#endif
