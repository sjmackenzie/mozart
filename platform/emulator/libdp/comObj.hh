#ifndef __COMOBJ_HH
#define __COMOBJ_HH

#include "dpBase.hh"
#include "genhashtbl.hh"
#include "prioQueues.hh"

class TransObj;
class MsgContainer;
class DSite;
class TimerElement;

enum CState {
  CLOSED,                      // CLOSED means no transObj (=no connection)
  CLOSED_WF_HANDOVER,          // WF=WAIT FOR
  CLOSED_WF_REMOTE,            //        waiting for incoming connection
  CLOSED_PROBLEM,
  ANONYMOUS_WF_NEGOTIATE,
  OPENING_WF_PRESENT,          
  OPENING_WF_NEGOTIATE_ANS,
  WORKING,
  CLOSING_HARD,
  CLOSING_WEAK,
  CLOSING_WF_DISCONNECT
};

class ComObj {
  friend class ComController;
  friend class TransController;
private:
  CState state;

  // Pointers to distribution-layer and transport-layer entities
  TransObj *transObj;
protected:
  DSite *site;
private:

  // Storage for MsgContainers
  PrioQueues queues;

  // Numbers for messages and acking-scheme
  int lastSent;
  int lastReceived;
  //  Bool ackQueued;

  // One timer to be used for opening/closing/acking (one at a time)
  TimerElement *timer;
  TimerElement *reopentimer;
  TimerElement *closetimer;

  // For probing
  Bool probing;
  Bool probeFired;
  int minrtt;
  int maxrtt;
  int probeinterval;
  TimerElement *probeIntervalTimer;
  TimerElement *probeFaultTimer;

  // Lost connections
  int retryTimeout;

  Bool localRef;
  Bool remoteRef;
  Bool sentclearref;
  int msgAckTimeOut;
  int msgAckLength;

  // Statistics
  int nosm; // Number of sent messages since getNOSM() was last called
  int norm; // Number of received messages since getNORM() was last called

  // private methods
  void open();
  void closeTemp();
  inline TaggedRef createCI(int);
  inline void extractCI(TaggedRef,int&);
  void errorRec(int);
  Bool hasNeed();
  void adoptCI(OZ_Term channelinfo);
  void merge(ComObj *old,ComObj *anon,OZ_Term channelinfo);
  void close(CState statetobe,Bool merging);
protected:
  ComObj *next; // For ComController usage
  void close(CState statetobe);
  void gcComObj();
  void clearTimers();
  void shutDown();
  ComObj *next_cache; // For TransController usage
  Bool hasQueued();
  Bool canBeClosed();
public:
  ComObj(DSite *site,int recCtr);
  void init(DSite *site,int recCtr);

  DSite *getSite() {return site;}

  // For Site:
  void send(MsgContainer *,int priority);
//    void siteIsPerm(); // Information from site. Should we seize to exist?
  void installProbe(int lowerBound, int higherBound, int interval);  
           // Should this be moved to the comController?
  Bool canBeFreed(); // A question that implicitly tells the comObj
                     // that no local references exist.
  
  // For TransController
  void preemptTransObj();

  // For TransObj:
  MsgContainer *getNextMsgContainer(int &); // Provides the TransObj 
                  // with the MsgContainer of the next message to be 
                  // sent. The current acknowledgement number is given by 
                  // the int &.
  void msgSent(MsgContainer *);
  void msgPartlySent(MsgContainer *); // Store away a message to be 
                                       // continued later.
  void msgPartlyReceived(MsgContainer *);
  void msgAcked(int num);

  MsgContainer *getMsgContainer(); // Gives a new clean MsgContainer 
                                // to be filled with an incomming message.
  MsgContainer *getMsgContainer(int num); // Gives the priviously stored 
                          // MsgContainer for message num to be continued.
  Bool msgReceived(MsgContainer *); // A full message was received and is 
                                    // now handed up. Return: continue?
  void connectionLost(void *info);

  // For connection procedure
  Bool handover(TransObj *);
  void accept(TransObj *);

  // Statistics
  int getNOSM();
  int getNORM();

  // Extras for internal use (must be public anyway)
  Bool openTimerExpired();
  Bool closeTimerExpired();
  Bool sendProbePing();
  Bool probeFault();
  Bool sendAck(Bool timerInvoked);
  Bool reopen();

  // For connection.cc usage (DEVEL) remove with connection-proc impl.
  void *infoelement;
};

void comController_acceptHandler(TransObj *);

#define ComObj_CUTOFF 100

class ComController: public FreeListManager {
private:
  ComObj *list; // To be divided in open, closed and closedRemote

public:
  ComController():FreeListManager(ComObj_CUTOFF) {
    list=NULL;
    wc = 0;
  }
  int wc;

  ComObj *newComObj(DSite *site,int recCtr);
  void deleteComObj(ComObj* comObj);
  int getCTR(){ return wc;}

  void gcComObjs();
  int closeDownCount();
private:
  Bool inList(ComObj *list,ComObj *testObj);
};

extern ComController *comController;

#endif // __COMOBJ_HH