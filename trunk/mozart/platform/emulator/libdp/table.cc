/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Per Brand, 1998
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

#if defined(INTERFACE)
#pragma implementation "table.hh"
#endif

#include "base.hh"
#include "table.hh"
#include "value.hh"
#include "var.hh"
#include "var_obj.hh"
#include "msgContainer.hh"
#include "state.hh"
#include "fail.hh"
#include "protocolState.hh"
#include "dpResource.hh"

#include "os.hh"

int NetHashTable::hashFunc(NetAddress *na){
  unsigned char *p = (unsigned char*) na;
  int i;
  unsigned h = 0, g;

  for(i=0;i<8; i++,p++) {
    h = (h << 4) + (*p);
    if ((g = h & 0xf0000000)) {
      h = h ^ (g >> 24);
      h = h ^ g;}}
  return (int) h;}

int NetHashTable::findNA(NetAddress *na){
  GenHashNode *ghn;
  int hvalue=hashFunc(na);
  if(findPlace(hvalue,na,ghn)){
    int bindex= GenHashNode2BorrowIndex(ghn);
    PD((HASH,"borrow index b:%d",bindex));
    return bindex;}
  return -1;}

void NetHashTable::add(NetAddress *na,int bindex){
  int hvalue=hashFunc(na);
#ifdef DEBUG_CHECK
  GenHashNode *ghn;
  Assert(!findPlace(hvalue,na,ghn));
#endif
  PD((HASH,"adding hvalue=%d net=%d:%d bindex=%d",
	       hvalue,na->site,na->index,bindex));
  GenHashBaseKey* ghn_bk;
  GenHashEntry* ghn_e;
  GenCast(na,NetAddress*,ghn_bk,GenHashBaseKey*);
  GenCast(bindex,int,ghn_e,GenHashEntry*);  
  htAdd(hvalue,ghn_bk,ghn_e);}

void NetHashTable::sub(NetAddress *na){
  int hvalue=hashFunc(na);
  GenHashNode *ghn;
  findPlace(hvalue,na,ghn);
  PD((HASH,"deleting hvalue=%d net=%d:%d bindex=%d",
	       hvalue,na->site,na->index));
  htSub(hvalue,ghn);}

/* -------------------------------------------------------------------- */

OwnerTable *ownerTable;
BorrowTable *borrowTable;


// from var.cc
void oz_dpvar_localize(TaggedRef *);

void OwnerEntry::setUp(int index){
  setFlags(0);
  ocreditHandler.setUp(index);
}

void OwnerEntry::localize(int index)
{
  if (isVar()) { 
    if(GET_VAR(this,Manager)->getInfo()==NULL)
      oz_dpvar_localize(getPtr());
    else return;
  } else {
    if(isTertiary() &&
       !localizeTertiary(getTertiary()))
      return;
  }
  OT->freeOwnerEntry(index);
}

void OwnerTable::init(int beg,int end){
  int i=beg;
  while(i<end){
    array[i].makeFree(i+1);
    i++;}
  i--;
  array[i].makeFree(END_FREE);
  nextfree=beg;}

void OwnerTable::compactify()
{
  Assert(size>=no_used);
  if(size==ozconf.dpTableDefaultOwnerTableSize) return;
  PD((TABLE,"owner compactify enter: size:%d no_used:%d",
      size,no_used));
  int i=0;
  int used_slot= -1;
  int* base = &nextfree;
  while(TRUE){
    if(i>=size) break;
    if(array[i].isFree()){
      *base=i;
      base=&array[i].u.nextfree;}
    else used_slot=i;
    i++;}
  *base=END_FREE;
  int after_last=used_slot+1;

  if(size<ozconf.dpTableDefaultOwnerTableSize){
    int newsize = ozconf.dpTableDefaultOwnerTableSize;
    array = (OwnerEntry*) realloc(array,newsize*sizeof(OwnerEntry));
    if(array==NULL){
      OZ_error("Memory allocation: Owner Table growth not possible");}
    int i=size;
    while(i<ozconf.dpTableDefaultOwnerTableSize){
      array[i].makeFree(i+1);
      i++;}
    i--;
    array[i].makeFree(END_FREE);
    *base=size;
    PD((TABLE,"owner compactify and realloc to the default table size %d"
	,newsize));
    size=newsize;
    return;}

  if(no_used*100/size >= ozconf.dpTableLowLimit){
    PD((TABLE,"owner compactify no realloc"));
    return;}
  int newsize= after_last-no_used < ozconf.dpTableBuffer ? 
    after_last+ozconf.dpTableBuffer : after_last+1;
  if(newsize > ozconf.dpTableDefaultOwnerTableSize &&
     size - newsize > ozconf.dpTableWorthwhileRealloc){
    PD((TABLE,"owner compactify free slots: new%d",newsize));
    array = (OwnerEntry*) realloc(array,newsize*sizeof(OwnerEntry));
    size=newsize;
    Assert(array[newsize-1].isFree());
    array[newsize-1].u.nextfree = END_FREE;
    Assert(size>=no_used);
    return;}
  PD((TABLE,"owner compactify no realloc\n"));
  Assert(size>=no_used);}

void OwnerTable::resize(){
#ifdef BTRESIZE_CRITICAL
  OZ_warning("OwnerTable::resize: maybe incorrect");
#endif
  int newsize = ozconf.dpTableExpandFactor*size/100;
  PD((TABLE,"resize owner old:%d no_used:%d new:%d",
		size,no_used,newsize));
  array = (OwnerEntry*) realloc(array,newsize*sizeof(OwnerEntry));
  if(array==NULL){
    OZ_error("Memory allocation: Owner Table growth not possible");}
  init(size, newsize);  
  size=newsize;
  PD((TABLE2,"TABLE:resize owner complete"));
  return;}

int OwnerTable::newOwner(OwnerEntry *&oe){
  Assert(perdioInitialized);
  if(nextfree == END_FREE) resize();
  int index = nextfree;
  nextfree = array[index].u.nextfree;
  oe = (OwnerEntry *)&(array[index]);
  oe->setUp(index);
  PD((TABLE,"owner insert: o:%d",index));
  no_used++;
  return index;}

void OwnerTable::freeOwnerEntry(int i)
{
  array[i].setFree();
  array[i].u.nextfree=nextfree;
  nextfree=i;
  no_used--;
  PD((TABLE,"owner delete o:%d",i));
  localizing();
  return;}

#define PO_getValue(po) \
((po)->isTertiary() ? makeTaggedConst((po)->getTertiary()) : (po)->getRef())

OZ_Term OwnerTable::extract_info(){ 
  OZ_Term list=oz_nil();
  OZ_Term credit;
  
  for(int ctr = 0; ctr<size;ctr++){
    OwnerEntry *oe = OT->getEntry(ctr);
    if(oe==NULL){continue;}
    Assert(oe!=NULL);
    credit=oe->ocreditHandler.extract_info();
    list=
      oz_cons(OZ_recordInit(oz_atom("oe"),
	oz_cons(oz_pairAI("index", ctr),
	oz_cons(oz_pairAA("type", toC(PO_getValue(oe))),
	oz_cons(oz_pairA("credit", credit), 
		oz_nil())))), list);
  }
  return OZ_recordInit(oz_atom("ot"),
           oz_cons(oz_pairAI("size", size),
	   oz_cons(oz_pairAI("localized", getLocalized()),
           oz_cons(oz_pairA("list", list), oz_nil()))));
  return list;
}

void OwnerTable::print(){ 
  printf("***********************************************\n");
  printf("********* OWNER TABLE *************************\n");
  printf("***********************************************\n");
  printf("Size:%d No_used:%d \n",size,no_used);
  printf("site:%s\n\n",myDSite->stringrep());
  printf("OI\tCredit\tOWNER\n");
      
  int i;
  for(i=0;i<size;i++){
    if(!(array[i].isFree())){
      OwnerEntry *oe=getOwner(i);
      printf("<%d>\t", i);
      oe->ocreditHandler.print();
      printf("\t%s\n",toC(PO_getValue(oe)));
    }
  }
  printf("-----------------------------------------------\n");  
}

void BorrowEntry::print_entry(int nr) {
  printf("<%d>\t %d\t", nr, 
	 bcreditHandler.netaddr.index);
  bcreditHandler.print();
  printf("\t\t%s\n",toC(PO_getValue(this)));
}

void BorrowTable::print(){
  printf("***********************************************\n");
  printf("********* BORROW TABLE *************************\n");
  printf("***********************************************\n");
  printf("Size:%d No_used:%d \n",size,no_used);
  printf("site:%s\n\n",myDSite->stringrep());
  printf("BI\t OI\t PrimCredit\t SecCredit\t BORROW\n");
  int i;
  BorrowEntry *b;
  for(i=0;i<size;i++){
    if(!(array[i].isFree())){
      b=getBorrow(i);
      b->print_entry(i);
    }}
  printf("-----------------------------------------------\n");  
}

OZ_Term BorrowEntry::extract_info(int index) {
  OZ_Term primCred, secCred;
  OZ_Term na=
    OZ_recordInit(oz_atom("netAddress"),
      oz_cons(oz_pairA("site", oz_atom(bcreditHandler.netaddr.site->stringrep_notype())),
      oz_cons(oz_pairAI("index",(int)bcreditHandler.netaddr.index), oz_nil())));
/*
  OZ_Term na=
    OZ_recordInit(oz_atom("netAddress"),
      oz_cons(oz_pairA("site",OZ_recordInit(oz_atom("site"),
          oz_cons(oz_pairAI("port",(int)netaddr.site->getPort()),
	  oz_cons(oz_pairAI("timeint",(int)netaddr.site->getTimeStamp()->start),
	  oz_cons(oz_pairA("timestr",oz_atom(
			          ctime(&netaddr.site->getTimeStamp()->start))),
	  oz_cons(oz_pairAI("ipint",(unsigned int)netaddr.site->getAddress()),
	  oz_cons(oz_pairAI("hval",(int)netaddr.site),
		oz_nil()))))))), 
	oz_cons(oz_pairAI("index",(int)netaddr.index), oz_nil())));
*/
  bcreditHandler.extract_info(primCred, secCred);
  return OZ_recordInit(oz_atom("be"),
     oz_cons(oz_pairAI("index", index),
     oz_cons(oz_pairAA("type", toC(PO_getValue(this))),
     oz_cons(oz_pairA("na", na),
     oz_cons(oz_pairA("secCred", secCred),
     oz_cons(oz_pairA("primCred",primCred),
	     oz_nil()))))));
}


void BorrowEntry::copyBorrow(BorrowEntry* from,int i){
  if (from->isTertiary()) {
    mkTertiary(from->getTertiary(),from->getFlags());
    from->getTertiary()->setIndex(i);
  } else if (from->isVar()) {
    mkVar(from->getRef(),from->getFlags());
    VarKind vk=typeOfBorrowVar(from);
    if(vk==VAR_PROXY){
      GET_VAR(from,Proxy)->gcSetIndex(i);}
    else{
      Assert(vk==VAR_LAZY);
      GET_VAR(from, Lazy)->setIndex(i);}
  } else{
    Assert(from->isRef());
    mkRef(from->getRef(),from->getFlags());
  }
  bcreditHandler.copyHandler(&(from->bcreditHandler));
}

void BorrowEntry::freeBorrowEntry(){
  Assert(!bcreditHandler.isExtended());
  if(isVar() && typeOfBorrowVar(this)==VAR_PROXY){
    GET_VAR(this,Proxy)->nowGarbage(this);}
  if(!isPersistent())
    bcreditHandler.giveBackAllCredit();}

void BorrowEntry::gcBorrowRoot(int i) {
  if (isVar()) {
    if(oz_isGcMark(getRef())){
      gcPO();
      return;}
    PD((GC,"BT1 b:%d variable found",i));
    if (tagged2CVar(*getPtr())->getSuspList()!=0) {
      gcPO();
      return;
    }
    if(!bcreditHandler.canBeFreed()) {
      gcPO();
    }
    return;}
  if(isRef()){
    Assert(bcreditHandler.isExtended());
    gcPO();
    return;}
  Assert(isTertiary());
  // AN: Copy also if this is a secondary credit master, i.e. check with
  // credithandler.
  if(getTertiary()->cacIsMarked() || !bcreditHandler.canBeFreed() ||
     isTertiaryPending(getTertiary())){
    gcPO();
  }
  return;
}

void BorrowTable::init(int beg,int end)
{
  int i=beg;
  while(i<end){
    array[i].u.nextfree=i+1;
    array[i].setFree();
    i++;}
  i--;
  array[i].u.nextfree=nextfree;
  nextfree=beg;
}

void BorrowTable::compactify(){
  Assert(notGCMarked());
  if(no_used*100/size >= ozconf.dpTableLowLimit) return;

  if(size<ozconf.dpTableDefaultBorrowTableSize){
    int newsize=ozconf.dpTableDefaultBorrowTableSize;
    PD((TABLE,"TABLE:compactify borrow, resized to the default table size %d",
	newsize));
    BorrowEntry *oldarray=array;
    array = (BorrowEntry*) malloc(newsize*sizeof(BorrowEntry));
    if(array==NULL){
      OZ_error("Memory allocation: Borrow Table growth not possible");}
    int oldsize=size;
    size=newsize;
    copyBorrowTable(oldarray,oldsize);
    return;}
    
  if(size==ozconf.dpTableDefaultBorrowTableSize) return;
  int newsize= no_used+ozconf.dpTableBuffer;
  if(newsize<ozconf.dpTableDefaultBorrowTableSize) 
    newsize=ozconf.dpTableDefaultBorrowTableSize;
  PD((TABLE,"compactify borrow old:%d no_used:%d new:%d",
		size,no_used,newsize));
  BorrowEntry *oldarray=array;
  array = (BorrowEntry*) malloc(newsize*sizeof(BorrowEntry));
  if(array==NULL){
    PD((TABLE,"compactify borrow NOT POSSIBLE"));
    array=oldarray;
    return;}
  int oldsize=size;
  size=newsize; 
  copyBorrowTable(oldarray,oldsize);
  PD((TABLE,"compactify borrow complete"));
  Assert(notGCMarked());
  return;}

void BorrowTable::resize()
{
  Assert(notGCMarked());
#ifdef BTRESIZE_CRITICAL
  OZ_warning("BorrowTable::resize: maybe incorrect");
#endif
  Assert(no_used==size);
  int newsize = ozconf.dpTableExpandFactor*size/100;
  PD((TABLE,"resize borrow old:%d no_used:%d new:%d", size,no_used,newsize));
  BorrowEntry *oldarray=array;
  array = (BorrowEntry*) malloc(newsize*sizeof(BorrowEntry));
  if(array==NULL){
    OZ_error("Memory allocation: Borrow Table growth not possible");}
  int oldsize=size;
  size=newsize;
  copyBorrowTable(oldarray,oldsize);
  PD((TABLE,"resize borrow complete"));
  Assert(notGCMarked());
  return;}

int BorrowTable::newBorrowPersistent(DSite * sd,int off) {
  if(nextfree == END_FREE) resize();
  int index=nextfree;
  nextfree= array[index].u.nextfree;
  BorrowEntry* be = &(array[index]);
  Assert(be->isFree());
  be->initBorrowPersistent(sd,off); 
  
  hshtbl->add(be->getNetAddress(),index);
  no_used++;
  PD((TABLE,"borrow insert: b:%d",index));
  return index;
}

int BorrowTable::newBorrow(Credit c,DSite * sd,int off){
  if(nextfree == END_FREE) resize();
  int index=nextfree;
  nextfree= array[index].u.nextfree;
  BorrowEntry* be = &(array[index]);
  Assert(be->isFree());
  be->initBorrow(c,sd,off); 
  
  hshtbl->add(be->getNetAddress(),index);
  no_used++;
  PD((TABLE,"borrow insert: b:%d",index));
  return index;
}

Bool BorrowTable::maybeFreeBorrowEntry(int index){
  BorrowEntry *b = &(array[index]);
  if(!b->bcreditHandler.maybeFreeCreditHandler()) {
    if(b->isVar()){
      b->changeToRef();
    }
    return FALSE;
  }
  b->freeBorrowEntry();  
  hshtbl->sub(getBorrow(index)->getNetAddress());
  b->setFree();
  b->u.nextfree=nextfree;
  nextfree=index;
  no_used--;
  PD((TABLE,"borrow delete: b:%d",index));
  return TRUE;
}

void BorrowTable::copyBorrowTable(BorrowEntry *oarray,int osize){
  hshtbl->clear();
  int oindex=0;
  int nindex= 0;
  BorrowEntry *ob,*nb;
  while(TRUE){
    nb=&(array[nindex]);
    ob= &(oarray[oindex]);
    while(ob->isFree()) {
      oindex++;
      if(oindex>=osize) goto fin;
      ob= &(oarray[oindex]);}
    nb->copyBorrow(ob,nindex);
    hshtbl->add(nb->getNetAddress(),nindex);
    nindex++;
    oindex++;
    if(oindex>=osize) goto fin;}
fin:
  nextfree=END_FREE;
  init(nindex,size);
  free(oarray);
}


void OB_Entry::gcPO() {
  Assert(!isFree());
  if (isGCMarked()) return;
  makeGCMark();
  Assert(isTertiary() || isRef() || isVar());
  PD((GC,"var/ref/tert found"));
  oz_gCollectTerm(u.tert, u.tert);
}

Bool withinBorrowTable(int i){
  if(i<borrowTable->getSize()) return OK;
  return NO;}


void OwnerTable::gcOwnerTableRoots()
{
  PD((GC,"owner gc"));
  for(int i=0;i<size;i++) {
    OwnerEntry* o = getOwner(i); 
    if(!o->isFree()) {
      PD((GC,"OT o:%d",i));
      o->gcPO();
    }
  }
  return;
}

void OwnerTable::gcOwnerTableFinal()
{
  PD((GC,"owner gc"));
  for(int i=0;i<size;i++) {
    OwnerEntry* o = getOwner(i);
    if(!o->isFree()) {
      o->removeGCMark();
      if (o->isVar()){
	TaggedRef *ptr = o->getPtr();
	DEREFPTR(v,ptr,_tag);
	Assert(oz_isManagerVar(v));
	o->mkVar(makeTaggedRef(ptr),o->getFlags());
      }
    }
  }
  compactify();
  return;
}

void BorrowTable::gcBorrowTableRoots()
{
  PD((GC,"borrowTable1 gc"));
  for(int i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    if (!b->isFree() && !b->isGCMarked())
      b->gcBorrowRoot(i);}
}

void BorrowEntry::gcBorrowUnusedFrame(Tertiary* t) {
  Assert(!(isTertiaryPending(getTertiary())));
  if(t->getType()==Co_Cell){
    CellFrame *cf = (CellFrame*) t;
    if(cf->dumpCandidate()){
      cellLockSendDump(this);}}
  else{
    Assert(t->getType()==Co_Lock);
    LockFrame *lf = (LockFrame*) t;
    if(lf->dumpCandidate()){
      cellLockSendDump(this);}
  }
  gcPO();
}

void BorrowTable::gcBorrowTableUnusedFrames()
{
  PD((GC,"borrow unused frames"));
  int i;
  for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    if((!b->isGCMarked()) && b->isTertiary() && b->getTertiary()->isFrame()){
      b->gcBorrowUnusedFrame(b->getTertiary());}}
}

void BorrowTable::gcFrameToProxy(){
  PD((GC,"borrow frame to proxy"));
  int i;
  for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    Assert(!b->isRef() || b->bcreditHandler.isExtended());
    if((b->isTertiary())){
      Tertiary *t=b->getTertiary();
      if(t->isFrame()) {
	if((t->getType()==Co_Cell)
	   && ((CellFrame*)t)->getState()==Cell_Lock_Invalid
	   && ((CellSec*)((CellFrame*)t)->getSec())->getPending()==NULL){
	  ((CellFrame*)t)->convertToProxy();}
	else{
	  if((t->getType()==Co_Lock)
	     && ((LockFrame*)t)->getState()==Cell_Lock_Invalid){
	    ((LockFrame*)t)->convertToProxy();}}}}}
}


/* OBSERVE - this must done at the end of other gc */
void BorrowTable::gcBorrowTableFinal()
{
  PD((GC,"borrow gc"));
  int i;
  for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    if(b->isVar()) {
      if(b->isGCMarked()){
	b->removeGCMark();
	b->getSite()->makeGCMarkSite();
	PD((GC,"BT b:%d mark variable found",i));} 
      else{
	PD((GC,"BT b:%d unmarked variable found",i));
	if(!errorIgnoreVar(b)) maybeUnaskVar(b);
	borrowTable->maybeFreeBorrowEntry(i);
      }} 
    else if(b->isTertiary()){
	Tertiary *t = b->getTertiary();
	if(b->isGCMarked()) {
	  b->removeGCMark();
	  b->getSite()->makeGCMarkSite();
	  PD((GC,"BT b:%d mark tertiary found",i));}
	else{
	  if(!errorIgnore(t)) maybeUnask(t);
	  Assert(t->isProxy());
	  borrowTable->maybeFreeBorrowEntry(i);
	}
    }
    else if(b->isRef()){
      Assert(b->isGCMarked());
      b->removeGCMark();
      b->getSite()->makeGCMarkSite();}
    else {
      Assert(b->isFree());}
  }
  compactify();
  hshtbl->compactify(); 
  DebugCode(for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    if(b->isVar()) {
      ; 
	      }
	      else 
		if(b->isTertiary()){
		  ;
		} 
		else 
		  if(b->isRef()){ 
		    ; 
		  }
		  else {
		    Assert(b->isFree());}
	    })
}



void BorrowTable::closeFrameToProxy(unsigned int ms){
  BorrowEntry *be;
  unsigned long start_time = osTotalTime();
  int j=0;
  for(int i=0;i<size;i++){
    be = getBorrow(i);
    if(be->isTertiary()){
      Tertiary *t = be->getTertiary();
      int type = t->getType();
      int state;
      if(t->isFrame()) {
	if(type==Co_Cell)
	  state = ((CellFrame*)t)->getState();
	else if(type==Co_Lock)
	  state = ((LockFrame*)t)->getState();
	else
	  continue;

	switch(state){
	case Cell_Lock_Invalid:
	  if(type==Co_Lock){
	    ((CellFrame*)t)->convertToProxy();}
	  else{
	    ((LockFrame*)t)->convertToProxy();}
	  break;
	case Cell_Lock_Requested:
	case Cell_Lock_Valid:
	  cellLockSendDump(be);
	  break;
	case Cell_Lock_Requested|Cell_Lock_Next:
	  break;
	case Cell_Lock_Valid|Cell_Lock_Next:
	  if(type==Co_Lock){
	    NetAddress *na=be->getNetAddress();
	    LockSec* sec = ((LockFrame*)t)->getLockSec();
	    DSite* toS = sec->getNext();
	    lockSendToken(na->site,na->index,toS);
	  }
	  else
	    Assert(0);
	  break;
	default:
	  Assert(0);
	}
      }
    }
    if(j>500) {
      if(ms <= (osTotalTime() - start_time)) 
	return;
    j=0; 
    }
    else j++;
  }
}

int BorrowTable::closeProxyToFree(unsigned int ms){
  BorrowEntry *be;
  int proxies = 0;
  int frames = 0;
  unsigned long start_time = osTotalTime();
  int j=0;
  for(int i=0;i<size;i++){
    be = getBorrow(i);
    if(!be->isFree()) 
      if(be->isTertiary()) {
	Tertiary *t = be->getTertiary();
	if(t->isProxy()) {
	  maybeFreeBorrowEntry(i);
	  proxies++;
	}
	if(t->isFrame()) {
	  int type = t->getType();
	  int state;
	  if(type==Co_Cell)
	    state = ((CellFrame*)t)->getState();
	  else if(type==Co_Lock)
	    state = ((LockFrame*)t)->getState();
	  else
	    continue;

	  switch(state){
	  case Cell_Lock_Invalid:
	    if(type==Co_Lock)
	      ((CellFrame*)t)->convertToProxy();
	    else
	      ((LockFrame*)t)->convertToProxy();
	    break;
	  case Cell_Lock_Requested:
	  case Cell_Lock_Valid:
	  case Cell_Lock_Requested|Cell_Lock_Next:
	    break;
	  case Cell_Lock_Valid|Cell_Lock_Next:
	    if(type==Co_Lock){
	      NetAddress *na=be->getNetAddress();
	      LockSec* sec = ((LockFrame*)t)->getLockSec();
	      DSite* toS = sec->getNext();
	      lockSendToken(na->site,na->index,toS);
	    }
	    else
	      Assert(0);
	    break;
	  default:
	    Assert(0);
	  }
	  frames++;
	}
      }
      else {
	if(be->isVar() && oz_isProxyVar(oz_deref(be->getRef()))) {
	  maybeFreeBorrowEntry(i);
	  proxies++;
	}
      }
    if(j>500) {
      if(ms <= (osTotalTime() - start_time)) 
	return -1;
      j=0;
    }
    else j++;    
  }
  //  printf("%d frames and %d proxies left\n", frames, proxies);
  //  printf("time left:%d\n", ms+start_time-osTotalTime());
  return proxies+frames;
}

int OwnerTable::notGCMarked() {
  OwnerEntry *be;
  for(int i=0;i<size;i++){
    be = getOwner(i);
    if(!be->isFree()) {
      if(be->isGCMarked())
	return FALSE;
      if(be->isTertiary()) {
	if(be->getTertiary()->cacIsMarked())
	  return FALSE;
	Assert(be->getTertiary()->getIndex() == i);
      }
    }
  }
  return TRUE;
}

int BorrowTable::notGCMarked() {
  BorrowEntry *be;
  for(int i=0;i<size;i++){
    be = getBorrow(i);
    if(!be->isFree()) {
      if(be->isGCMarked())
	return FALSE;
      if(be->isTertiary()) {
	if(be->getTertiary()->cacIsMarked())
	  return FALSE;
	Assert(be->getTertiary()->getIndex() == i);
      }
    }
  }
  return TRUE;
}
