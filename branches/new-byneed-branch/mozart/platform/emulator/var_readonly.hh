/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    Raphael Collet <raph@info.ucl.ac.be>
 *    Alfred Spiessens <fsp@info.ucl.ac.be>
 * 
 *  Copyright:
 *    Denys Duchier (1998)
 *    Michael Mehl (1998)
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __READONLY__HH__
#define __READONLY__HH__

#if defined(INTERFACE)
#pragma interface
#endif

#include "var_base.hh"

/*
 * fred+raph:
 * A ReadOnly object implements a read-only variable.
 * An attempt to bind by unification such a variable blocks.
 * The variable object has two possible types: OZ_VAR_READONLY_QUIET
 * when it is not needed, and OZ_VAR_READONLY when it becomes needed.
 */

class ReadOnly: public OzVariable {
public:
  // raph: read-only variables are not needed by default
  ReadOnly(Board *bb) : OzVariable(OZ_VAR_READONLY_QUIET,bb) {}

  OZ_Return bind(TaggedRef* vPtr,TaggedRef t);
  OZ_Return unify(TaggedRef* vPtr,TaggedRef* tPtr);
  OZ_Return forceBind(TaggedRef* vPtr,TaggedRef v);

  // raph: the variable must be quiet; makes it needed
  OZ_Return becomeNeeded();

  // raph: use this method for adding demanding suspensions only
  OZ_Return addSusp(TaggedRef*, Suspendable *);

  Bool valid(TaggedRef /* val */) { return TRUE; }
  void dispose(void) {
    disposeS();
    oz_freeListDispose(this, sizeof(ReadOnly));
  }
  void printStream(ostream &out,int depth = 10) {
    if (getType() == OZ_VAR_READONLY_QUIET) {
      out << "<readonly quiet>";
    } else {
      out << "<readonly>";
    }
  }
  void printLongStream(ostream &out,int depth = 10,
				int offset = 0) {
    printStream(out,depth); out << endl;
  }
};

// create a new ReadOnly
inline
OZ_Term oz_newReadOnly(Board *bb) {
  return makeTaggedRef(newTaggedVar(new ReadOnly(bb)));
}

// bind a ReadOnly, don't care about the variable
inline
void oz_bindReadOnly(OZ_Term *vPtr,OZ_Term val)
{
  oz_var_forceBind(tagged2Var(*vPtr),vPtr,val);
}

#endif /* __READONLY__HH__ */
