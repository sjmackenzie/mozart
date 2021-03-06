/*
 *  Authors:
 *    Zacharias El Banna (zeb@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Zacharias El Banna, 2002
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

#ifndef __PROTOCOL_MIGRATORY_HH
#define __PROTOCOL_MIGRATORY_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dssBase.hh"
#include "protocols.hh"
#include "dss_templates.hh"

namespace _dss_internal{ //Start namespace

  class ProtocolMigratoryManager:public ProtocolManager {
  private:
    SimpleQueue<Pair<DSite*, int> > a_chain;   // the chain of proxies
    DSite*                          a_last;    // last proxy in chain

    // The chain contains pairs (proxy, request_id).  The proxies will
    // receive the token in the order given by the chain.  The value
    // request_id is used when the manager asks a proxy whether it has
    // already received and forwarded the token.  It is required to
    // avoid any ambiguity between requests.
    //
    // Invariants:
    //  - a_last is the last proxy in the chain (if not empty);
    //  - a proxy never appears twice in a_chain;
    //  - a_chain is empty and a_last==NULL iff the entity is permfail.

    // run the inquiry algorithm
    void inquire(DSite*);
    // called when the token has been lost
    void lostToken();

    ProtocolMigratoryManager(const ProtocolMigratoryManager&) {}
    ProtocolMigratoryManager& operator=(const ProtocolMigratoryManager&){ return *this; }

  public:
    ProtocolMigratoryManager(DSite*);
    ProtocolMigratoryManager(::MsgContainer*);
    ~ProtocolMigratoryManager() {}

    void makeGCpreps() { ProtocolManager::makeGCpreps(); t_gcList(a_chain); }
    void msgReceived(::MsgContainer*,DSite*);
    
    void sendMigrateInfo(::MsgContainer*); 

    // check for failed proxies
    void m_siteStateChange(DSite*, const FaultState&);
  };


  // state of a migratory proxy (when not permfail)
  enum MigratoryToken {
    MIGT_EMPTY,       // token not here
    MIGT_REQUESTED,   // token not here, but has been requested
    MIGT_HERE         // token here
  };
  

  class ProtocolMigratoryProxy:public ProtocolProxy{
    friend class ProtocolMigratoryManager;
  private:
    DSite* a_successor;   // next proxy in chain
    int    a_request;     // request id

    void requestToken();
    void forwardToken();
    void resumeOperations();
    void lostToken();

    ProtocolMigratoryProxy(const ProtocolMigratoryProxy&):
      ProtocolProxy(PN_MIGRATORY_STATE), a_successor(NULL) {}
    ProtocolMigratoryProxy& operator=(const ProtocolMigratoryProxy&){ return *this; }

  public:
    ProtocolMigratoryProxy();
    bool m_initRemoteProt(DssReadBuffer*);
    ~ProtocolMigratoryProxy();

    // WEAK protocol (to get rid of the token)
    // 1) I send a "need_no_more" to manager
    // 2) manager check if I am last, 
    //    if so it sends me a "forward" (to home proxy)
    //    else  it has already sent me a forward message and everething is ok
    bool isWeakRoot() { return !isPermFail() && getStatus() > MIGT_EMPTY; }
    bool clearWeakRoot();    
    
    OpRetVal protocol_Access(GlobalThread* const,
			     ::PstOutContainerInterface**&);
    virtual OpRetVal operationRead(GlobalThread*, PstOutContainerInterface**&);
    virtual OpRetVal operationWrite(GlobalThread*, PstOutContainerInterface**&);
    virtual OpRetVal operationWrite(PstOutContainerInterface**&x){return ProtocolProxy::operationWrite(x);};

    void makeGCpreps();
    void msgReceived(::MsgContainer*,DSite*);
    
    // check fault state
    virtual FaultState siteStateChanged(DSite*, const FaultState&);
  };

} //End namespace
#endif 
