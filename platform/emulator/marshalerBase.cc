/* -*- C++ -*-
 *  Authors:
 *    Ralf Scheidhauer <Ralf.Scheidhauer@ps.uni-sb.de>
 *    Konstantin Popov <kost@sics.se>
 * 
 *  Contributors:
 *    Per Brand <perbrand@sics.se>
 *    Michael Mehl <mehl@dfki.de>
 *    Denys Duchier <duchier@ps.uni-sb.de>
 *    Andreas Sundstroem <andreas@sics.se>
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifdef INTERFACE
#pragma implementation "marshalerBase.hh"
#endif

#include "base.hh"
#include "marshalerBase.hh"
#if !defined(TEXT2PICKLE)
#include <math.h>
#include "mbuffer.hh"
#include "site.hh"
#include "pickle.hh"
#endif

//

#if !defined(TEXT2PICKLE)
//
unsigned int RobustMarshaler_Max_Shift;
unsigned int RobustMarshaler_Max_Hi_Byte;

//
int32* NMMemoryManager::freelist[NMMM_SIZE];

//
// Stuff needed for to check that no overflow is done in unmarshalNumber()
void initRobustMarshaler()
{
  unsigned int intsize = sizeof(int);
  unsigned int shft = intsize*7;
  while(shft <= (intsize*8)-7) shft += 7;
  RobustMarshaler_Max_Shift = shft;
  RobustMarshaler_Max_Hi_Byte = 
    (int) pow(2, (intsize*8)-RobustMarshaler_Max_Shift);
}

//
class DoubleConv {
public:
  union {
    unsigned char c[sizeof(double)];
    int i[sizeof(double)/sizeof(int)];
    double d;
  } u;
};

Bool isLowEndian()
{
  DoubleConv dc;
  dc.u.i[0] = 1;
  return dc.u.c[0] == 1;
}
//
static const Bool lowendian = isLowEndian();

//
SendRecvCounter dif_counter[DIF_LAST];
SendRecvCounter misc_counter[MISC_LAST];

//
void marshalFloat(MarshalerBuffer *bs, double d)
{
  static DoubleConv dc;
  dc.u.d = d;
  if (lowendian) {
    marshalNumber(bs, dc.u.i[0]);
    marshalNumber(bs, dc.u.i[1]);
  } else {		       
    marshalNumber(bs, dc.u.i[1]);
    marshalNumber(bs, dc.u.i[0]);
  }
}

//
void marshalGName(MarshalerBuffer *bs, GName *gname)
{
  misc_counter[MISC_GNAME].send();

  // generic one (not distributions'!)
  gname->site->marshalSiteForGName(bs);
  for (int i = 0; i < fatIntDigits; i++) {
    marshalNumber(bs, gname->id.number[i]);
  }
  marshalNumber(bs, (int) gname->gnameType);
}

//
//
void marshalSmallInt(MarshalerBuffer *bs, OZ_Term siTerm)
{
  marshalDIF(bs, DIF_SMALLINT);
  marshalNumber(bs, tagged2SmallInt(siTerm));
}

//
void marshalFloat(MarshalerBuffer *bs, OZ_Term floatTerm)
{
  marshalDIF(bs, DIF_FLOAT);
  marshalFloat(bs, tagged2Float(floatTerm)->getValue());
}

//
void marshalLiteral(MarshalerBuffer *bs, OZ_Term litTerm, int litTermInd)
{
  Literal *lit = tagged2Literal(litTerm);

  MarshalTag litTag;
  GName *gname = NULL;

  if (lit->isAtom()) {
    litTag = DIF_ATOM;
  } else if (lit->isUniqueName()) {
    litTag = DIF_UNIQUENAME;
  } else if (lit->isCopyableName()) {
    litTag = DIF_COPYABLENAME;
  } else {
    litTag = DIF_NAME;
    gname = ((Name *) lit)->globalize();
  }

  marshalDIF(bs, litTag);
  const char *name = lit->getPrintName();
  marshalTermDef(bs, litTermInd);
  marshalString(bs, name);
  if (gname) marshalGName(bs, gname);
}

//
void marshalBigInt(MarshalerBuffer *bs, OZ_Term biTerm, ConstTerm *biConst)
{ 
  marshalDIF(bs, DIF_BIGINT);
  marshalString(bs, toC(biTerm));
}

//
void marshalBuiltin(MarshalerBuffer *bs,
		    OZ_Term biTerm, ConstTerm *biConst, int biTermInd)
{
  Builtin *bi= (Builtin *) biConst;
  marshalDIF(bs, DIF_BUILTIN);
  marshalTermDef(bs, biTermInd);
  marshalString(bs, bi->getPrintName());
}

//
void marshalExtension(MarshalerBuffer *bs, OZ_Term extTerm)
{
  marshalDIF(bs, DIF_EXTENSION);
  marshalNumber(bs, tagged2Extension(extTerm)->getIdV());
}

//
void marshalRepetition(MarshalerBuffer *bs, int repNumber)
{
  Assert(repNumber >= 0);
  marshalDIF(bs, DIF_REF);
  marshalTermRef(bs, repNumber);
}

//
void marshalLTupleHead(MarshalerBuffer *bs, OZ_Term ltupleTerm, int ltupleTermInd)
{
  marshalDIF(bs, DIF_LIST);
  marshalTermDef(bs, ltupleTermInd);
}

//
void marshalSRecordHead(MarshalerBuffer *bs,
			OZ_Term srecordTerm, int srecordTermInd)
{
  SRecord *rec = tagged2SRecord(srecordTerm);
  TaggedRef label = rec->getLabel();

  if (rec->isTuple()) {
    marshalDIF(bs, DIF_TUPLE);
    marshalTermDef(bs, srecordTermInd);
    marshalNumber(bs, rec->getTupleWidth());
  } else {
    marshalDIF(bs, DIF_RECORD);
    marshalTermDef(bs, srecordTermInd);
  }
}

//
GName *globalizeConst(ConstTerm *t, MarshalerBuffer *bs)  
{ 
  switch(t->getType()) {
  case Co_Object:      return ((Object*)t)->globalize();
  case Co_Class:       return ((ObjectClass*)t)->globalize();
  case Co_Chunk:       return ((SChunk*)t)->globalize();
  case Co_Abstraction: return ((Abstraction*)t)->globalize();
  default: Assert(0); return NULL;
  }
}

//
void marshalChunk(MarshalerBuffer *bs, OZ_Term chunkTerm,
		  ConstTerm *chunkConst, int chunkTermInd)
{ 
  SChunk *ch    = (SChunk *) chunkConst;
  GName *gname  = globalizeConst(ch, bs);

  marshalDIF(bs, DIF_CHUNK);
  marshalTermDef(bs, chunkTermInd);
  if (gname) marshalGName(bs, gname);
}

//
void skipNumber(MarshalerBuffer *bs)
{
  unsigned int c = bs->get();
  while (c >= SBit)
    c = bs->get();
}

//
// Code area;
//
void marshalBuiltin(GenTraverser *gt, Builtin *entry)
{
  gt->marshalOzValue(makeTaggedConst(entry));
}
void traverseBuiltin(GenTraverser *gt, Builtin *entry)
{
  gt->marshalOzValue(makeTaggedConst(entry));
}

//
void marshalProcedureRef(GenTraverser *gt,
			 AbstractionEntry *entry, MarshalerBuffer *bs)
{
  Bool copyable = entry && entry->isCopyable();
  marshalNumber(bs, copyable);
  if (copyable) {
    int ind = gt->findPtr(entry);
    if (ind >= 0) {
      marshalDIF(bs, DIF_REF);
      marshalTermRef(bs, ind);
    } else {
      marshalDIF(bs, DIF_ABSTRENTRY);
      rememberNode(gt, bs, entry);
    }
  } else {
    Assert(entry==NULL || entry->getAbstr() != NULL);
  }
}

//
static inline
void marshalRecordArityType(RecordArityType type, MarshalerBuffer *bs)
{
  marshalNumber(bs, type);
}

//
//
// NOTE: this code cannot be changed without changing
// marshaling/unmarshaling of pred id"s, hash table refs, gen call
// info"s and app meth info"s !!! Sorry, i (kost@) don't know how to
// express an appropriate assertion... may be the be way is just not
// to use this procedure for marshaling structures named above;
void marshalRecordArity(GenTraverser *gt,
			SRecordArity sra, MarshalerBuffer *bs)
{
  if (sraIsTuple(sra)) {
    marshalRecordArityType(TUPLEWIDTH, bs);
    marshalNumber(bs, getTupleWidth(sra));
  } else {
    marshalRecordArityType(RECORDARITY, bs);
    gt->marshalOzValue(getRecordArity(sra)->getList());
  }
}

//
void traverseRecordArity(GenTraverser *gt, SRecordArity sra)
{
  if (!sraIsTuple(sra))
    gt->marshalOzValue(getRecordArity(sra)->getList());
}

//
void marshalPredId(GenTraverser *gt, PrTabEntry *p, MarshalerBuffer *bs)
{
  gt->marshalOzValue(p->getName());
  marshalRecordArity(gt, p->getMethodArity(), bs);
  gt->marshalOzValue(p->getFile());
  marshalNumber(bs, p->getLine());
  marshalNumber(bs, p->getColumn());
  gt->marshalOzValue(p->getFlagsList());
  marshalNumber(bs, p->getMaxX());
}

//
void traversePredId(GenTraverser *gt, PrTabEntry *p)
{
  gt->marshalOzValue(p->getName());
  traverseRecordArity(gt, p->getMethodArity());
  gt->marshalOzValue(p->getFile());
  gt->marshalOzValue(p->getFlagsList());
}

//
void marshalCallMethodInfo(GenTraverser *gt,
			   CallMethodInfo *cmi, MarshalerBuffer *bs)
{
  int compact = (cmi->regIndex<<1) | (cmi->isTailCall);
  marshalNumber(bs, compact);
  gt->marshalOzValue(cmi->mn);
  marshalRecordArity(gt, cmi->arity, bs);
}

//
void traverseCallMethodInfo(GenTraverser *gt, CallMethodInfo *cmi)
{
  gt->marshalOzValue(cmi->mn);
  traverseRecordArity(gt, cmi->arity);
}

//
void marshalGRegRef(AssRegArray *gregs, MarshalerBuffer *bs)
{ 
  int nGRegs = gregs->getSize();
  marshalNumber(bs, nGRegs);

  for (int i = 0; i < nGRegs; i++) {
    int out = ((*gregs)[i].getIndex()<<2) | (int)(*gregs)[i].getKind();
    marshalNumber(bs, out);
  }
}

//
void marshalLocation(Builtin *bi, OZ_Location *loc, MarshalerBuffer *bs) 
{ 
  int inAr  = bi->getInArity();
  int outAr = bi->getOutArity();
  marshalNumber(bs, inAr);
  marshalNumber(bs, outAr);

  for (int i = 0; i < (inAr + outAr); i++) {
    int out = loc->getIndex(i);
    marshalNumber(bs, out);
  }
}

//
// The hash table is considered to be compound: its subtrees are table
// nodes;
void marshalHashTableRef(GenTraverser *gt,
			 int start, IHashTable *table, MarshalerBuffer *bs)
{
  int sz = table->getSize();
  marshalNumber(bs, sz);	// kost@ : but it's not used anymore;
  marshalLabel(bs, start, table->lookupElse());
  marshalLabel(bs, start, table->lookupLTuple());
  int entries = table->getEntries();
  // total number of entries (and, thus, of 'ht_???_entry' tasks);
  marshalNumber(bs, entries);

  //
  for (int i = table->getSize(); i--; ) {
    if (table->entries[i].val) {
      if (oz_isLiteral(table->entries[i].val)) {
	if (table->entries[i].sra == mkTupleWidth(0)) {
	  // That's a literal entry
	  marshalNumber(bs, ATOMTAG);
	  marshalLabel(bs, start, table->entries[i].lbl);
	  gt->marshalOzValue(table->entries[i].val);
	} else {
	  // That's a record entry
	  marshalNumber(bs, RECORDTAG);
	  marshalLabel(bs,start, table->entries[i].lbl);
	  gt->marshalOzValue(table->entries[i].val);
	  marshalRecordArity(gt, table->entries[i].sra, bs);
	}
      } else {
	Assert(oz_isNumber(table->entries[i].val));
	// That's a number entry
	marshalNumber(bs,NUMBERTAG);
	marshalLabel(bs, start, table->entries[i].lbl);
	gt->marshalOzValue(table->entries[i].val);
      }
    }
  }
}

//
void traverseHashTableRef(GenTraverser *gt, int start, IHashTable *table)
{
  int sz = table->getSize();
  int entries = table->getEntries();

  for (int i = table->getSize(); i--; ) {
    if (table->entries[i].val) {
      if (oz_isLiteral(table->entries[i].val)) {
	if (table->entries[i].sra == mkTupleWidth(0)) {
	  // That's a literal entry
	  gt->marshalOzValue(table->entries[i].val);//makeTaggedLiteral(aux->getLiteral()));
	} else {
	  // That's a record entry
	  gt->marshalOzValue(table->entries[i].val);
	  traverseRecordArity(gt, table->entries[i].sra);
	}
      } else {
	Assert(oz_isNumber(table->entries[i].val));
	// That's a number entry
	gt->marshalOzValue(table->entries[i].val);//aux->getNumber());
      }
    }
  }
}

#ifdef USE_FAST_UNMARSHALER   

//
unsigned int unmarshalNumber(MarshalerBuffer *bs)
{
  unsigned int ret = 0, shft = 0;
  unsigned int c = bs->get();
  while (c >= SBit) {
    ret += ((c-SBit) << shft);
    c = bs->get();
    shft += 7;
  }
  ret |= (c<<shft);
  return ret;
}

//
// MarshalerBuffer's 'getString' does not check overflows etc.;
static
char *getString(MarshalerBuffer *bs, unsigned int i)
{
  char *ret = new char[i+1];
  if (ret == (char *) 0)
    return ((char *) 0);
  for (unsigned int k=0; k<i; k++)
    ret[k] = bs->get();
  ret[i] = '\0';
  return (ret);
}

//
double unmarshalFloat(MarshalerBuffer *bs)
{
  static DoubleConv dc;
  if (lowendian) {
    dc.u.i[0] = unmarshalNumber(bs);
    dc.u.i[1] = unmarshalNumber(bs);
  } else {
    dc.u.i[1] = unmarshalNumber(bs);
    dc.u.i[0] = unmarshalNumber(bs);
  }
  return dc.u.d;
}

//
char *unmarshalString(MarshalerBuffer *bs)
{
  misc_counter[MISC_STRING].recv();
  unsigned int i = unmarshalNumber(bs);

  return getString(bs,i);
}

//
static
void unmarshalGName1(GName *gname, MarshalerBuffer *bs)
{
  gname->site=unmarshalSite(bs);
  for (int i=0; i<fatIntDigits; i++) {
    gname->id.number[i] = unmarshalNumber(bs);
  }
  gname->gnameType = (GNameType) unmarshalNumber(bs);
}

//
GName *unmarshalGName(TaggedRef *ret, MarshalerBuffer *bs)
{
  misc_counter[MISC_GNAME].recv();
  GName gname;
  unmarshalGName1(&gname,bs);
  
  TaggedRef aux = oz_findGName(&gname);
  if (aux) {
    if (ret) *ret = aux; // ATTENTION
    return 0;
  }
  return new GName(gname);
}

#else // USE_FAST_UNMARSHALER

//
unsigned int unmarshalNumberRobust(MarshalerBuffer *bs, int *overflow)
{
  unsigned int ret = 0, shft = 0;
  unsigned int c = bs->get();
  while (c >= SBit) {
    ret += ((c-SBit) << shft);
    c = bs->get();
    shft += 7;
  }
  if(shft > RobustMarshaler_Max_Shift) {
    *overflow = OK;
    return 0;
  }
  else if(shft == RobustMarshaler_Max_Shift) {
    if(c >= RobustMarshaler_Max_Hi_Byte) {
      *overflow = OK;
      return 0;
    }
  }
  ret |= (c<<shft);
  *overflow = NO;
  return ret;
}

//
static
char *getStringRobust(MarshalerBuffer *bs, unsigned int i, int *error)
{
  char *ret = new char[i+1];
  if (ret==NULL) {
    *error = OK;
    return NULL;
  }
  for (unsigned int k=0; k<i; k++)
    ret[k] = bs->get();
  ret[i] = '\0';
  *error = NO;
  return ret;
}

//
double unmarshalFloatRobust(MarshalerBuffer *bs, int *overflow)
{
  static DoubleConv dc;
  int o1, o2;
  if (lowendian) {
    dc.u.i[0] = unmarshalNumberRobust(bs, &o1);
    dc.u.i[1] = unmarshalNumberRobust(bs, &o2);
  } else {
    dc.u.i[1] = unmarshalNumberRobust(bs, &o1);
    dc.u.i[0] = unmarshalNumberRobust(bs, &o2);
  }
  *overflow = o1 || o2;
  return dc.u.d;
}

//
char *unmarshalStringRobust(MarshalerBuffer *bs, int *error)
{
  int e1,e2;
  char *string;
  misc_counter[MISC_STRING].recv();
  unsigned int i = unmarshalNumberRobust(bs,&e1);
  string = getStringRobust(bs,i,&e2);
  *error = e1 || e2;
  return string;
}

//
static
void unmarshalGName1Robust(GName *gname, MarshalerBuffer *bs, int *error)
{
  int e1,e2;
  gname->site=unmarshalSiteRobust(bs, &e1);
  for (int i=0; (i<fatIntDigits && !e1); i++) {
    int e;
    unsigned int num;
    num = unmarshalNumberRobust(bs, &e);
    e1 = e || (num > maxDigit);
    gname->id.number[i] = num;
  }
  gname->gnameType = (GNameType) unmarshalNumberRobust(bs, &e2);
  *error = e1 || e2 || (gname->gnameType > MAX_GNT);
}

//
GName *unmarshalGNameRobust(TaggedRef *ret, MarshalerBuffer *bs, int *error)
{
  misc_counter[MISC_GNAME].recv();
  GName gname;
  unmarshalGName1Robust(&gname,bs,error);
  
  TaggedRef aux = oz_findGName(&gname);
  if (aux) {
    if (ret) *ret = aux; // ATTENTION
    return 0;
  }
  return new GName(gname);
}

#endif

//
// Occasionally Oz terms must be placed in the current binary area
// (which representation will follow in the stream): this is arranged
// with 'putOzValueCA' processor. Note that writing into a code area
// is that code area's business;
class CodeAreaLocation : public NMMemoryManager {
private:
  ProgramCounter ptr;
  CodeArea *code;
public:
  CodeAreaLocation(ProgramCounter ptrIn, CodeArea* codeIn)
    : ptr(ptrIn), code(codeIn) {}
  CodeAreaLocation(ProgramCounter ptrIn)
    : ptr(ptrIn) { DebugCode(code = (CodeArea *) -1;); }
  //
  ProgramCounter getPtr() { return (ptr); }
  CodeArea *getCodeArea() { return (code); }
};

//
// A "CodeAreaLocation" argument ist not sufficient for more
// interesting cases, in which more than one Oz value is used to fill
// up a cell in a code area.
//
// For predicate id"s these are a predicate's name, *optional* record
// arity list, file name and flags list. Given the reversed order they
// appear in stream, the last three must be saved until the
// 'PrTabEntry' be really constructed.
class PredIdLocation : public CodeAreaLocation {
private:
  int line, column, maxX;
  // 'name' will be supplied last and does not need to be stored;
  SRecordArity sra;		// of a tuple
  OZ_Term arityList;		// of a record;
  OZ_Term file;
  OZ_Term flagsList;
public:
  PredIdLocation(ProgramCounter ptrIn)
    : CodeAreaLocation(ptrIn), sra((SRecordArity) 0)
  {
    DebugCode(line = column = maxX = -1;);
    DebugCode(file = flagsList = arityList = (OZ_Term) -1;);
  }

  //
  void setSRA(SRecordArity sraIn) { sra = sraIn; }
  SRecordArity getSRA() { return (sra); }
  void setArityList(OZ_Term ra) { arityList = ra; }
  OZ_Term getArityList() { return (arityList); }
  void setFile(OZ_Term fileIn) { file = fileIn; }
  OZ_Term getFile() { return (file); }
  void setFlagsList(OZ_Term flagsListIn ) { flagsList = flagsListIn; }
  OZ_Term getFlagsList() { return (flagsList); }

  //
  void setLine(int lineIn) { line = lineIn; }
  int getLine() { return (line); }
  void setColumn(int columnIn) { column = columnIn; }
  int getColumn() { return (column); }
  void setMaxX(int maxXIn) { maxX = maxXIn; }
  int getMaxX() { return (maxX); }
};

//
// 
class CallMethodInfoLocation : public CodeAreaLocation {
private:
  int compact;
  SRecordArity sra;		// tuples;
  OZ_Term arityList;		// records;

  //
public:
  CallMethodInfoLocation(ProgramCounter ptrIn, int compactIn)
    : CodeAreaLocation(ptrIn), compact(compactIn), sra((SRecordArity) 0)
  {
    DebugCode(arityList = (OZ_Term) -1;);
  }

  //
  int getCompact() { return (compact); }
  SRecordArity getSRA() { return (sra); }
  void setSRA(SRecordArity sraIn) { sra = sraIn; }
  OZ_Term getArityList() { return (arityList); }
  void setArityList(OZ_Term ar) { arityList = ar; }
};

//
// Hash table entries are constructed using the table itself, label,
// and either an Oz value or an Oz value and SRecordArity. Thus, a
// descriptor of an entry used for the 'Builder::getOzValue()' task
// keeps table, label and may be a record arity list.
class HashTableEntryDesc : public NMMemoryManager {
private:
  IHashTable *table;
  int label;
  SRecordArity sra;		// for "tuple" record entries only;
  OZ_Term arityList;		// for "proper" record entries only;
public:
  HashTableEntryDesc(IHashTable *tableIn, int labelIn)
    : table(tableIn), label(labelIn), sra((SRecordArity) 0)
  {
    DebugCode(arityList = (OZ_Term) -1;);
  }

  //
  IHashTable* getTable() { return (table); }
  int getLabel() { return (label); }
  SRecordArity getSRA() { return (sra); }
  void setSRA(SRecordArity sraIn) { sra = sraIn; }
  void setArityList(OZ_Term ra) { arityList = ra; }
  OZ_Term getArityList() { return (arityList); }
};

//
static void putOzValueCA(void *arg, OZ_Term value)
{
  CodeAreaLocation *loc = (CodeAreaLocation *) arg;
  //
  (void) (loc->getCodeArea())->writeTagged(value, loc->getPtr());
  delete loc;
}

//
// A builtin in a code area is stored not as an 'OzTerm' but as an
// Builtin* (while in the stream it appears as an 'OzTerm'):
static void putBuiltinCA(void *arg, OZ_Term value)
{
  CodeAreaLocation *loc = (CodeAreaLocation *) arg;
  //
  Assert(oz_isBuiltin(value));
  CodeArea::writeAddressAllocated(tagged2Builtin(value), loc->getPtr());
  delete loc;
}

//
static inline
SRecordArity makeRealRecordArity(OZ_Term arityList)
{
  Assert(isSorted(arityList));
  Arity *ari = aritytable.find(arityList);
  Assert(!ari->isTuple());
  return (mkRecordArity(ari));
}

//
// Gets a (complete) arity list and puts 'SRecordArity';
static void putRealRecordArityCA(void *arg, OZ_Term value) {
  CodeAreaLocation *loc = (CodeAreaLocation *) arg;

  //
  Assert(isSorted(value));
  SRecordArity sra = makeRealRecordArity(value);
  CodeArea::writeWordAllocated(sra, loc->getPtr());
  delete loc;
}

//
// Thus, there are four processors, out of which three saves values
// and the last one does the job:
static void getPredIdNameCA(void *arg, OZ_Term value)
{
  PredIdLocation *loc = (PredIdLocation *) arg;
  SRecordArity sra = loc->getSRA();

  //
  if (!sra) {
    // must be a record:
    sra = makeRealRecordArity(loc->getArityList());
  }

  //
  // 'value' is the name argument;
  PrTabEntry *pred = new
    PrTabEntry(value, 
	       sra, loc->getFile(),
	       loc->getLine(), loc->getColumn(),
	       loc->getFlagsList(), loc->getMaxX());
  CodeArea::writeAddressAllocated(pred, loc->getPtr());
  //
  delete loc;
}

//
// Note that these processors do not delete the argument;
// Note also that 'saveSRACA' is used only when the arity is of a real
// record (i.e. not a tuple);
static void saveRecordArityPredIdCA(void *arg, OZ_Term value)
{
  PredIdLocation *loc = (PredIdLocation *) arg;
  // 'value' *will be* an arity list - now we have only head cell;
  loc->setArityList(value);
}
static void saveFileCA(void *arg, OZ_Term value)
{
  PredIdLocation *loc = (PredIdLocation *) arg;
  loc->setFile(value);
}
static void saveFlagsListCA(void *arg, OZ_Term value)
{
  PredIdLocation *loc = (PredIdLocation *) arg;
  loc->setFlagsList(value);
}

//
static void getCallMethodInfoNameCA(void *arg, OZ_Term value)
{
  CallMethodInfoLocation *loc = (CallMethodInfoLocation *) arg;
  int compact = loc->getCompact();
  SRecordArity sra = loc->getSRA();

  //
  if (!sra) {
    // must be a record:
    sra = makeRealRecordArity(loc->getArityList());
  }

  //
  int ri      = compact>>1;
  Bool ist    = (compact&1);
  CallMethodInfo *cmi = new CallMethodInfo(ri, value, ist, sra);
  CodeArea::writeAddressAllocated(cmi, loc->getPtr());

  //
  delete loc;
}

//
static void saveCallMethodInfoRecordArityCA(void *arg, OZ_Term value)
{
  CallMethodInfoLocation *loc = (CallMethodInfoLocation *) arg;
  loc->setArityList(value);
}

//
// Processors...
static void getHashTableRecordEntryLabelCA(void *arg, OZ_Term value)
{
  HashTableEntryDesc *desc = (HashTableEntryDesc *) arg;
  SRecordArity sra = desc->getSRA();

  //
  if (!sra) {
    OZ_Term recordArity = desc->getArityList();
    Assert(isSorted(recordArity));
    sra = makeRealRecordArity(recordArity);
  }
  //
  (desc->getTable())->addRecord(value, sra, desc->getLabel());

  //
  delete desc;
}

//
static void saveRecordArityHashTableEntryCA(void *arg, OZ_Term value)
{
  HashTableEntryDesc *desc = (HashTableEntryDesc *) arg;
  desc->setArityList(value);
}

//
static void getHashTableAtomEntryLabelCA(void *arg, OZ_Term value)
{
  HashTableEntryDesc *desc = (HashTableEntryDesc *) arg;

  //
  (desc->getTable())->addScalar(value, desc->getLabel());
  delete desc;
}

//
static void getHashTableNumEntryLabelCA(void *arg, OZ_Term value)
{
  HashTableEntryDesc *desc = (HashTableEntryDesc *) arg;

  //
  (desc->getTable())->addScalar(value, desc->getLabel());
  delete desc;
}

//
void handleDEBUGENTRY(void *arg)
{
  ProgramCounter PC = (ProgramCounter) arg;
  Assert(PC);

  TaggedRef file = getTaggedArg(PC+1);
  int line = tagged2SmallInt(getNumberArg(PC+2));
  CodeArea::writeDebugInfo(PC, file, line);
}

//
ProgramCounter writeAddress(void *ptr, ProgramCounter pc)
{
  return (pc ? CodeArea::writeAddress(ptr, pc) : (ProgramCounter) 0);
}
ProgramCounter unmarshalCache(ProgramCounter pc, CodeArea *code)
{
  return (pc ? code->writeCache(pc) : 0);
}

//
ProgramCounter unmarshalOzValue(Builder *b,
				ProgramCounter pc, CodeArea *code)
{
  ProgramCounter retPC;
  if (pc) {
    CodeAreaLocation *loc = new CodeAreaLocation(pc, code);
    b->getOzValue(putOzValueCA, loc);
    retPC = CodeArea::allocateWord(pc);
  } else {
    b->discardOzValue();
    retPC = 0;
  }
  return (retPC);
}

//
ProgramCounter unmarshalBuiltin(Builder *b, ProgramCounter pc)
{
  if (pc) {
    CodeAreaLocation *loc = new CodeAreaLocation(pc);
    b->getOzValue(putBuiltinCA, loc);
    return (CodeArea::allocateWord(pc));
  } else {
    b->discardOzValue();
    return ((ProgramCounter) 0);
  }
}


#ifdef USE_FAST_UNMARSHALER   

//
ProgramCounter unmarshalGRegRef(ProgramCounter PC, MarshalerBuffer *bs)
{ 
  int nGRegs = unmarshalNumber(bs);
  AssRegArray *gregs = PC ? AssRegArray::allocate(nGRegs) : 0;

  for (int i = 0; i < nGRegs; i++) {
    unsigned int reg = unmarshalNumber(bs);
    if (PC) {
      (*gregs)[i].set(reg>>2,reg&3);
    }
  }

  return (writeAddress(gregs, PC));
}

//
ProgramCounter unmarshalLocation(ProgramCounter PC, MarshalerBuffer *bs)
{ 
  int inAr = unmarshalNumber(bs);
  int outAr = unmarshalNumber(bs);
  OZ_Location::initLocation();

  for (int i = 0; i < inAr+outAr; i++) {
    int n = unmarshalNumber(bs);
    OZ_Location::set(i,n);
  }

  return (writeAddress(OZ_Location::getLocation(inAr+outAr), PC));
}

//
static inline
RecordArityType unmarshalRecordArityType(MarshalerBuffer *bs)
{
  return ((RecordArityType) unmarshalNumber(bs));
}

//
// 'unmarshalRecordArity' exists in two flavors: one fills a cell in a
// code area with an 'SRecordArity' value (resp. discards it in "skip"
// mode), while another one just fills up a given cell;
ProgramCounter unmarshalRecordArity(Builder *b,
				    ProgramCounter pc, MarshalerBuffer *bs) 
{
  RecordArityType at = unmarshalRecordArityType(bs);
  if (pc) {
    if (at == RECORDARITY) {
      CodeAreaLocation *loc = new CodeAreaLocation(pc);
      b->getOzValue(putRealRecordArityCA, loc);
      return (CodeArea::allocateWord(pc));
    } else {
      Assert(at == TUPLEWIDTH);
      int width = unmarshalNumber(bs);
      return (CodeArea::writeInt(mkTupleWidth(width), pc));
    }
  } else {
    if (at == RECORDARITY)
      b->discardOzValue();
    else
      skipNumber(bs);
    return ((ProgramCounter) 0);
  }
}

//
// (of course, this code must resemble 'marshalPredId()')
ProgramCounter unmarshalPredId(Builder *b, ProgramCounter pc,
			       ProgramCounter lastPC, MarshalerBuffer *bs) 
{
  if (pc) {
    PredIdLocation *loc = new PredIdLocation(pc);

    //
    b->getOzValue(getPredIdNameCA, loc);
    //
    RecordArityType at = unmarshalRecordArityType(bs);
    if (at == RECORDARITY) {
      b->getOzValue(saveRecordArityPredIdCA, loc);
    } else {
      Assert(at == TUPLEWIDTH);
      int width = unmarshalNumber(bs);
      // set 'SRecordArity' directly (and there will be no arity
      // list);
      loc->setSRA(mkTupleWidth(width));
    }
    //
    b->getOzValue(saveFileCA, loc);
    //
    loc->setLine(unmarshalNumber(bs));
    loc->setColumn(unmarshalNumber(bs));
    //
    b->getOzValue(saveFlagsListCA, loc);
    //
    loc->setMaxX(unmarshalNumber(bs));

    //
    return (CodeArea::allocateWord(pc));

  } else {
    //
    b->discardOzValue();	// name;
    //
    RecordArityType at = unmarshalRecordArityType(bs);
    if (at == RECORDARITY) {
      b->discardOzValue();	// arity list;
    } else {
      Assert(at == TUPLEWIDTH);
      skipNumber(bs);
    }
    //
    b->discardOzValue();	// file;
    //
    skipNumber(bs);		// line & column;
    skipNumber(bs);
    //
    b->discardOzValue();	// flags list;
    //
    skipNumber(bs);		// maxX;

    //
    return ((ProgramCounter) 0);
  }
}

//
ProgramCounter unmarshalCallMethodInfo(Builder *b,
				       ProgramCounter pc, MarshalerBuffer *bs) 
{
  int compact = unmarshalNumber(bs);

  //
  if (pc) {
    CallMethodInfoLocation *loc = new CallMethodInfoLocation(pc, compact);

    //
    b->getOzValue(getCallMethodInfoNameCA, loc);
    //
    RecordArityType at = unmarshalRecordArityType(bs);
    if (at == RECORDARITY) {
      b->getOzValue(saveCallMethodInfoRecordArityCA, loc);
    } else {
      Assert(at == TUPLEWIDTH);
      int width = unmarshalNumber(bs);
      loc->setSRA(mkTupleWidth(width));
    }

    //
    return (CodeArea::allocateWord(pc));
  } else {
    b->discardOzValue();	// name;
    //
    RecordArityType at = unmarshalRecordArityType(bs);
    if (at == RECORDARITY)
      b->discardOzValue();
    else
      skipNumber(bs);

    //
    return ((ProgramCounter) 0);
  }
}

//
ProgramCounter unmarshalHashTableRef(Builder *b, ProgramCounter pc,
				     MarshalerBuffer *bs)
{
  //
  if (pc) {
    (void) unmarshalNumber(bs); // Backward compat
    int elseLabel = unmarshalNumber(bs); /* the else label */
    int listLabel = unmarshalNumber(bs);
    int nEntries = unmarshalNumber(bs);
    IHashTable *table;

    //
    table = IHashTable::allocate(nEntries, elseLabel);
    if (listLabel)
      table->addLTuple(listLabel);

    //
    for (int i = nEntries; i--; ) {    
      int termTag = unmarshalNumber(bs);
      int label   = unmarshalNumber(bs);
      HashTableEntryDesc *desc = new HashTableEntryDesc(table, label);

      //
      switch (termTag) {
      case RECORDTAG:
	{
	  b->getOzValue(getHashTableRecordEntryLabelCA, desc);
	  //
	  RecordArityType at = unmarshalRecordArityType(bs);
	  if (at == RECORDARITY) {
	    b->getOzValue(saveRecordArityHashTableEntryCA, desc);
	  } else {
	    Assert(at == TUPLEWIDTH);
	    int width = unmarshalNumber(bs);
	    desc->setSRA(mkTupleWidth(width));
	  }
	  break;
	}

      case ATOMTAG:
	b->getOzValue(getHashTableAtomEntryLabelCA, desc);
	break;

      case NUMBERTAG:
	b->getOzValue(getHashTableNumEntryLabelCA, desc);
	break;

      default: Assert(0); break;
      }
    }

    // 
    // The hash table is stored already, albeit it is not yet filled
    // up;
    return (CodeArea::writeIHashTable(table, pc));
  } else {
    skipNumber(bs);		// size
    skipNumber(bs);		// elseLabel
    skipNumber(bs);		// listLabel
    int nEntries = unmarshalNumber(bs);

    //
    for (int i = nEntries; i--; ) {
      int termTag = unmarshalNumber(bs);
      skipNumber(bs);		// label

      //
      switch (termTag) {
      case RECORDTAG:
	{
	  b->discardOzValue();
	  //
	  RecordArityType at = unmarshalRecordArityType(bs);
	  if (at == RECORDARITY)
	    b->discardOzValue();
	  else
	    skipNumber(bs);
	  break;
	}

      case ATOMTAG:
	b->discardOzValue();
	break;

      case NUMBERTAG:
	b->discardOzValue();
	break;

      default: Assert(0); break;
      }
    }

    //
    return ((ProgramCounter) 0);
  }
}

//
ProgramCounter unmarshalProcedureRef(Builder *b, ProgramCounter pc,
				     MarshalerBuffer *bs, CodeArea *code)
{
  AbstractionEntry *entry = 0;
  Bool copyable = unmarshalNumber(bs);
  if (copyable) {
    MarshalTag tag = (MarshalTag) bs->get();
    if (tag == DIF_REF) {
      int i = unmarshalNumber(bs);
      entry = (AbstractionEntry*) ToPointer(b->get(i));
    } else {
      Assert(tag == DIF_ABSTRENTRY);
      int refTag = unmarshalRefTag(bs);
      entry = new AbstractionEntry(OK);
      b->set(ToInt32(entry),refTag);
    }
  }
  return (pc ? code->writeAbstractionEntry(entry,pc) : (ProgramCounter) pc);
}

#else 

//
ProgramCounter unmarshalGRegRefRobust(ProgramCounter PC,
				      MarshalerBuffer *bs, int *error)
{ 
  int e1,e2;
  int nGRegs = unmarshalNumberRobust(bs, &e1);
  AssRegArray *gregs = PC ? AssRegArray::allocate(nGRegs) : 0;

  for (int i = 0; i < nGRegs; i++) {
    unsigned int reg = unmarshalNumberRobust(bs, &e2);
    e1 = e1 || e2;
    if (PC) {
      (*gregs)[i].set(reg>>2,(reg&3));
    }
  }
  *error = e1;

  return (writeAddress(gregs, PC));
}

//
ProgramCounter unmarshalLocationRobust(ProgramCounter PC, MarshalerBuffer *bs,
				     int *error) 
{
  int e1,e2;
  int inAr = unmarshalNumberRobust(bs, &e1);
  int outAr = unmarshalNumberRobust(bs, &e2);

  OZ_Location::initLocation();

  e1 = e1 || e2;
  for (int i = 0; i < inAr+outAr; i++) {
    int n = unmarshalNumberRobust(bs, &e2);
    e1 = e1 || e2;
    OZ_Location::set(i,n);
  }
  *error = e1;
  return (writeAddress(OZ_Location::getLocation(inAr+outAr), PC));
}

//
static inline
RecordArityType unmarshalRecordArityTypeRobust(MarshalerBuffer *bs, int *overflow)
{
  return ((RecordArityType) unmarshalNumberRobust(bs, overflow));
}

//
// 'unmarshalRecordArity' exists in two flavors: one fills a cell in a
// code area with an 'SRecordArity' value (resp. discards it in "skip"
// mode), while another one just fills up a given cell;
ProgramCounter unmarshalRecordArityRobust(Builder *b, ProgramCounter pc, 
					  MarshalerBuffer *bs, int *error) 
{
  RecordArityType at = unmarshalRecordArityTypeRobust(bs, error);
  if (pc) {
    if (at == RECORDARITY) {
      CodeAreaLocation *loc = new CodeAreaLocation(pc);
      b->getOzValue(putRealRecordArityCA, loc);
      return (CodeArea::allocateWord(pc));
    } else {
      int e;
      int width = unmarshalNumberRobust(bs, &e);
      *error = *error || e || (at != TUPLEWIDTH);
      return (CodeArea::writeInt(mkTupleWidth(width), pc));
    }
  } else {
    if (at == RECORDARITY)
      b->discardOzValue();
    else
      skipNumber(bs);
    return ((ProgramCounter) 0);
  }
}

//
// (of course, this code must resemble 'marshalPredId()')
ProgramCounter unmarshalPredIdRobust(Builder *b, ProgramCounter pc,
				     ProgramCounter lastPC, MarshalerBuffer *bs, 
				     int *error) 
{
  if (pc) {
    int e1,e2,e3,e4;
    PredIdLocation *loc = new PredIdLocation(pc);

    //
    b->getOzValue(getPredIdNameCA, loc);
    //
    RecordArityType at = unmarshalRecordArityTypeRobust(bs, &e1);
    if (at == RECORDARITY) {
      b->getOzValue(saveRecordArityPredIdCA, loc);
      e1 = NO;
    } else {
      Assert(at == TUPLEWIDTH);
      int width = unmarshalNumberRobust(bs, &e1);
      // set 'SRecordArity' directly (and there will be no arity
      // list);
      loc->setSRA(mkTupleWidth(width));
    }
    //
    b->getOzValue(saveFileCA, loc);
    //
    loc->setLine(unmarshalNumberRobust(bs, &e2));
    loc->setColumn(unmarshalNumberRobust(bs, &e3));
    //
    b->getOzValue(saveFlagsListCA, loc);
    //
    loc->setMaxX(unmarshalNumberRobust(bs, &e4));

    *error = e1 || e2 || e3 || e4;
    //
    return (CodeArea::allocateWord(pc));

  } else {
    //
    b->discardOzValue();	// name;
    //
    RecordArityType at = unmarshalRecordArityTypeRobust(bs, error);
    if (at == RECORDARITY) {
      b->discardOzValue();	// arity list;
    } else {
      Assert(at == TUPLEWIDTH);
      skipNumber(bs);
    }
    //
    b->discardOzValue();	// file;
    //
    skipNumber(bs);		// line & column;
    skipNumber(bs);
    //
    b->discardOzValue();	// flags list;
    //
    skipNumber(bs);		// maxX;

    //
    return ((ProgramCounter) 0);
  }
}

//
ProgramCounter unmarshalCallMethodInfoRobust(Builder *b, ProgramCounter pc, 
					     MarshalerBuffer *bs, int *error) 
{
  int e1,e2,e3;
  int compact = unmarshalNumberRobust(bs, &e1);

  //
  if (pc) {
    CallMethodInfoLocation *loc = new CallMethodInfoLocation(pc, compact);

    //
    b->getOzValue(getCallMethodInfoNameCA, loc);
    //
    RecordArityType at = unmarshalRecordArityTypeRobust(bs, &e2);
    if (at == RECORDARITY) {
      b->getOzValue(saveCallMethodInfoRecordArityCA, loc);
      e3 = NO;
    } else {
      Assert(at == TUPLEWIDTH);
      int width = unmarshalNumberRobust(bs, &e3);
      loc->setSRA(mkTupleWidth(width));
    }

    //
    *error = e1 || e2 || e3;
    return (CodeArea::allocateWord(pc));
  } else {
    b->discardOzValue();	// name;
    //
    RecordArityType at = unmarshalRecordArityTypeRobust(bs, &e2);
    if (at == RECORDARITY)
      b->discardOzValue();
    else
      skipNumber(bs);

    //
    *error = e1 || e2;
    return ((ProgramCounter) 0);
  }
}

//
ProgramCounter unmarshalHashTableRefRobust(Builder *b, ProgramCounter pc,
					   MarshalerBuffer *bs, int *error)
{
  //
  if (pc) {
    int e1,e2,e3,e4;
    int sz = unmarshalNumberRobust(bs, &e1);
    int elseLabel = unmarshalNumberRobust(bs, &e2); /* the else label */
    int listLabel = unmarshalNumberRobust(bs, &e3);
    int nEntries = unmarshalNumberRobust(bs, &e4);
    e1 = e1 || e2 || e3 || e4;
    IHashTable *table;

    //
    table = new IHashTable(sz, elseLabel);
    if (listLabel)
      table->addList(listLabel);

    //
    for (int i = 0; i < nEntries; i++) {    
      int termTag = unmarshalNumberRobust(bs, &e2);
      int label = unmarshalNumberRobust(bs, &e3);
      HashTableEntryDesc *desc = new HashTableEntryDesc(table, label);
      e1 = e1 || e2 || e3;

      //
      switch (termTag) {
      case RECORDTAG:
	{
	  b->getOzValue(getHashTableRecordEntryLabelCA, desc);
	  //
	  RecordArityType at = unmarshalRecordArityTypeRobust(bs, &e2);
	  if (at == RECORDARITY) {
	    b->getOzValue(saveRecordArityHashTableEntryCA, desc);
	    e3 = NO;
	  } else {
	    Assert(at == TUPLEWIDTH);
	    int width = unmarshalNumberRobust(bs, &e3);
	    desc->setSRA(mkTupleWidth(width));
	  }
	  e1 = e1 || e2 || e3;
	  break;
	}

      case ATOMTAG:
	b->getOzValue(getHashTableAtomEntryLabelCA, desc);
	break;

      case NUMBERTAG:
	b->getOzValue(getHashTableNumEntryLabelCA, desc);
	break;

      default: e1 = OK; break;
      }
    }

    *error = e1;
    // 
    // The hash table is stored already, albeit it is not yet filled
    // up;
    return (CodeArea::writeIHashTable(table, pc));
  } else {
    int e1,e2;
    skipNumber(bs);		// size
    skipNumber(bs);		// elseLabel
    skipNumber(bs);		// listLabel
    int nEntries = unmarshalNumberRobust(bs, &e1);

    //
    for (int i = 0; i < nEntries; i++) {
      int termTag = unmarshalNumberRobust(bs, &e2);
      e1 = e1 || e2;
      skipNumber(bs);		// label

      //
      switch (termTag) {
      case RECORDTAG:
	{
	  b->discardOzValue();
	  //
	  RecordArityType at = unmarshalRecordArityTypeRobust(bs, &e2);
	  e1 = e1 || e2;
	  if (at == RECORDARITY)
	    b->discardOzValue();
	  else
	    skipNumber(bs);
	  break;
	}

      case ATOMTAG:
	b->discardOzValue();
	break;

      case NUMBERTAG:
	b->discardOzValue();
	break;

      default: e1 = OK; break;
      }
    }

    //
    return ((ProgramCounter) 0);
  }
}

//
ProgramCounter unmarshalProcedureRefRobust(Builder *b, ProgramCounter pc,
					   MarshalerBuffer *bs, CodeArea *code, 
					   int *error)
{
  int e1,e2;
  AbstractionEntry *entry = 0;
  Bool copyable = unmarshalNumberRobust(bs, &e1);
  if (copyable) {
    MarshalTag tag = (MarshalTag) bs->get();
    if (tag == DIF_REF) {
      int i = unmarshalNumberRobust(bs, &e2);
      entry = (AbstractionEntry*) ToPointer(b->get(i));
    } else {
      Assert(tag == DIF_ABSTRENTRY);
      int e;
      int refTag = unmarshalRefTagRobust(bs, b, &e);
      e2 = e || (tag != DIF_ABSTRENTRY);
      entry = new AbstractionEntry(OK);
      b->set(ToInt32(entry),refTag);
    }
  }
  else e2 = NO;
  *error = e1 || e2;
  return ((pc && !(*error)) ? code->writeAbstractionEntry(entry,pc)
	  : (ProgramCounter) pc);
}

#endif // defined(USE_FAST_UNMARSHALER)

#endif // !defined(TEXT2PICKLE)