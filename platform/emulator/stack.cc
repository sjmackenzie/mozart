/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
*/

#ifdef __GNUC__
#pragma implementation "stack.hh"
#endif

#include "stack.hh"

void Stack::resize(int newSize)
{
  message("Resizing stack from %d to %d\n",size,newSize);
  int used = tos-array;
  array = reallocate(array, size, newSize);
  if(!array)
    error("Cannot realloc stack memory at %s:%d.", __FILE__, __LINE__);
  size     = newSize;
  tos      = array+used;
  stackEnd = array+size;
}


Stack::Stack(int sz) 
{
  size = sz;
  array = allocate(size);
  if(!array)
    error("Cannot alloc stack memory at %s:%d.", __FILE__, __LINE__);
  tos = array;
  stackEnd = array+size;
}



StackEntry *Stack::allocate(int n)
{ 
  return (StackEntry *) malloc(sizeof(StackEntry) * n); 
}

void Stack::deallocate(StackEntry *p, int n) 
{ 
  free(p); 
}

StackEntry *Stack::reallocate(StackEntry *p, int oldsize, int newsize) 
{
  return (StackEntry*) realloc(p,sizeof(StackEntry) * newsize);
}
