/*
 *  Authors:
 *    Konstantin Popov, Per Brand
 * 
 *  Contributors:
 *
 *  Copyright:
 *    Konstantin Popov, Per Brand 1997-1998
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

#ifndef __VIRTUAL_HH
#define __VIRTUAL_HH

#include "base.hh"
#include "dpBase.hh"
#include "vs_comm.hh"

//
// Here is the interface to virtual sites.
// Pointers to procedures are set by 'new mailbox'/'init server'
// builtins
//

//
VirtualSite* createVirtualSiteImpl(DSite* s);

//
void marshalVirtualInfoImpl(VirtualInfo *vi, MsgBuffer *mb);
VirtualInfo* unmarshalVirtualInfoImpl(MsgBuffer *mb); 
void unmarshalUselessVirtualInfoImpl(MsgBuffer *);

//
void zeroRefsToVirtualImpl(VirtualSite *vs);

//
// The 'mt', 'storeSite' and 'storeIndex' parameters are used whenever a 
// communication layer reports a problem with a message which has
// been previously accepted for delivery;
int sendTo_VirtualSiteImpl(VirtualSite *vs, MsgBuffer *mb,
			   MessageType mt, DSite *storeSite, int storeIndex);

//
int discardUnsentMessage_VirtualSiteImpl(VirtualSite *vs, int msgNum);

//
int getQueueStatus_VirtualSiteImpl(VirtualSite *vs, int &noMsgs);

//
SiteStatus siteStatus_VirtualSiteImpl(VirtualSite* vs); 

//
MonitorReturn
monitorQueue_VirtualSiteImpl(VirtualSite *vs, 
			     int size, int no_msgs, void *storePtr);
MonitorReturn demonitorQueue_VirtualSiteImpl(VirtualSite* vs);

//
ProbeReturn
installProbe_VirtualSiteImpl(VirtualSite *vs, ProbeType pt, int frequency);
ProbeReturn
deinstallProbe_VirtualSiteImpl(VirtualSite *vs, ProbeType pt);
ProbeReturn
probeStatus_VirtualSiteImpl(VirtualSite *vs,
			    ProbeType &pt, int &frequncey, void* &storePtr);

//
GiveUpReturn giveUp_VirtualSiteImpl(VirtualSite* vs);

//
void discoveryPerm_VirtualSiteImpl(VirtualSite *vs);

//
void dumpVirtualInfoImpl(VirtualInfo* vi);

//
MsgBuffer* getVirtualMsgBufferImpl(DSite* site);

//
void dumpVirtualMsgBufferImpl(MsgBuffer* m);

//
void siteAlive_VirtualSiteImpl(VirtualSite *vs);

///
/// An exit hook - kill slaves, reclaim shared memory, etc...
/// In fact, this declaration cannot be used since 'virtaulSite.hh' 
/// should not be included into 'am.cc';
void virtualSitesExitImpl();

//
// There are also (local) methods of (virtual site) module ...
//
VSMsgBufferOwned* composeVSInitMsg();
VSMsgBufferOwned* composeVSSiteIsAliveMsg(DSite *s);
VSMsgBufferOwned* composeVSSiteAliveMsg(DSite *s, VirtualSite *vs);
VSMsgBufferOwned* composeVSUnusedShmIdMsg(DSite *s, key_t shmid);

//
VSMsgType getVSMsgType(VSMsgBufferImported *mb);

//
void decomposeVSInitMsg(VSMsgBuffer *mb, DSite* &s);
void decomposeVSSiteIsAliveMsg(VSMsgBuffer *mb, DSite* &s);
void decomposeVSSiteAliveMsg(VSMsgBuffer *mb, DSite* &s, VirtualSite* &vs);
void decomposeVSUnusedShmIdMsg(VSMsgBuffer *mb, DSite* &s, key_t &shmid);

#endif // __VIRTUAL_HH
