/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  implementation of threads and queues of threads with priorities
  ------------------------------------------------------------------------
*/

#if defined(__GNUC__) && !defined(NOPRAGMA)
#pragma implementation "thread.hh"
#endif

/* some random comments on threads:

   threads are 'switch'ed, if
     - 'timed out' by alarm clock
     - 'finish'ed
     - 'displace'd by higher priority
     - 'freeze'd

   need for thread switching is signaled by 'ThreadSwitch' S-Flag

   "current thread"
     - the currently running thread

   */

#include "tagged.hh"
#include "constter.hh"
#include "actor.hh"
#include "board.hh"
#include "stack.hh"
#include "taskstk.hh"
#include "thread.hh"

#include "suspensi.hh"

#ifdef OUTLINE
#define inline
#include "thread.icc"
#undef inline
#endif

// --------------------------------------------------------------------------


/* class Thread
   static variables
     Head: pointer to the head of the thread queue
     Tail: pointer to the tail of the thread queue
     am.currentThread: pointer to the current thread
     am.rootThread: pointer to the root thread
   member data
     priority: the thread priority
               -MININT ... +MAXINT
	       low     ... high priority
     taskStack: the stack of elaboration tasks
     */

/*
 * the mode is 3, if all tasks on the thread have seq mode
 */
void Thread::setCompMode(int newMode) {
  Assert((compMode&1) != newMode);
  Assert(SEQMODE==1 && PARMODE==0);
  if (taskStack.isEmpty()) {
    if (newMode == SEQMODE) {
      compMode=ALLSEQMODE;
    } else {
      compMode=newMode;
    }
  } else {
    taskStack.pushCompMode(compMode);
    compMode=newMode;
  }
}
