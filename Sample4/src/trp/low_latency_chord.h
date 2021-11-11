#ifndef __LOW_LATENCY_CHORD_H__
#define __LOW_LATENCY_CHORD_H__

#include "log.h"
#include "inc.h"

#include "message_buffer.h"
#include "llchord_classes_defs.h"
#include "trp_llchord_if.h"

using namespace std;



namespace P2P_MODEL
{
    



    enum chord_action {
        DO_REPLY = 0,
        DO_FORWARD,
        DROP_MESSAGE,
        ACTION_UNKNOWN
    };

    enum event_type {
        MIN_EVENT_TYPE = 0,
        CALLED_BY_ANOTHER_STATE,
        RX_MESS_RECEIVED,
        TX_MESS_SHOULD_SEND,
        TIMER_EXPIRED,
        MAX_EVENT_TYPE,
        EVENT_TYPE_UNKNOWN
    };

    class low_latency_chord: public sc_module,
                             public log
    {
    public:
        sc_port<trp_llchord_if> trp_port;

    private:
        node_address m_nodeAddr;                        //¿ƒ–≈—, »—œŒÀ‹«”≈Ã€… ƒÀﬂ »ƒ≈Õ“»‘» ¿÷»» ”«À¿ Õ¿ Transport+ ”–Œ¬Õ≈ ÔÓ ID, ‚˚˜ËÒÎˇÂÏÓÏ, Í‡Í SHA-1
        uint         m_currSeed;
        
        sc_event     m_eventCore;

        finite_state       m_state;
        vector< message_buffer<chord_message> > m_buffer;
        message_buffer<chord_message> m_timersBuffer;
        
        uint m_howManyBuffers;
        int  m_indexLastBufferCall;        

        //map<uint, node_address> m_cwFingers;
        //map<uint, node_address> m_ccwFingers;
        //map<uint, sc_time>      m_latency;
        vector<node_address> m_seedAddrs;
        node_address m_predecessor;
        node_address m_successor;
        vector<node_address_latency>  m_cwFingers;  //clock wise fingers
        vector<node_address_latency>  m_ccwFingers; //counter clock wise fingers
        map<uint, sc_time>    m_latency;        
        chord_conf_parameters m_confParams; 
        map<uint160, sc_time> m_isAcked;

        bool m_isSuccessorSet;
        bool m_isPrecessorSet;
        
        vector<uint160> m_fingerMask;
        uint m_currCwFinger;
        uint m_counterJoin;
        uint m_messageID;
        int m_timerBufferIndex;
        int m_repeatCounter;

        string m_errCode;
        

    public:
        SC_HAS_PROCESS(low_latency_chord);

        low_latency_chord(sc_module_name name);
        ~low_latency_chord();

        void              setNetworkAddress(const network_address& netwAddr);      
        network_address&  getNetworkAddress();        
        node_address&     getNodeAddress();

        void setSeedNodes(const vector<network_address>& seed);
        void setConfParameters(const chord_conf_parameters& params);
        void pushNewMessage(const chord_message& mess);

    private:
        void preinit();
        void core();        

        void hardReset();
        void softReset();
        void flush();
        bool doResetFlushIfMess(const chord_message* mess);        

        void goStateLoad(); 
        void goStateInit(const chord_message* mess = nullptr);
        void goStateJoin(const chord_message* mess = nullptr);
        void goStateIdle(const chord_message* mess = nullptr);
        void goStateIndata(const chord_message* mess = nullptr);
        void goStateService(const chord_message* mess = nullptr);
        void goStateUpdate(const chord_message* mess = nullptr);
        void goStateApprequest(const chord_message* mess = nullptr);

        

        bool isAddrValid(const chord_message& mess);
        bool isMessageIDvalid(const chord_message& mess, buffer_container::iterator& it);
        int checkMessage(const chord_message& mess, buffer_container::iterator& it, const string& errCode);

        int chordMessType2buffIndex(const uint type);
        chord_message* firstMessByPriority();
        void eraseFirstMess();
        void pushNewTimer(const uint type, const uint retryCounter, const chord_byte_message_fields* retryMess = nullptr);
        void removeTimer(const uint timerType, const uint retryMessType, const uint retryMessID);
        void removeTimer(buffer_container::iterator timerIt);
        buffer_container::iterator findMessageOnTimersWithRetryParams(bool& exist, const uint timerType, const vector<uint>& retryMessType, const uint messageID);
        
        void sendMessage(const chord_message& mess);

        string& state2str(const finite_state& state);

        chord_action findSuccessor(const uint160& id, node_address& found);
        bool isClockWiseDirection(const uint160& id);
        node_address& closestPrecedingNode(const uint160& id);
        bool isInRange(const uint160& id, const uint160& A, const bool includeA, const uint160& B, const bool includeB);

        chord_message& createMessage(const chord_message& params);


        chord_message& createJoinMessage(const node_address& dest);
        chord_message& createNotifyMessage(const node_address& dest);
        chord_message& createAckMessage(const node_address& dest, const uint messageID);
        chord_message& createFindSuccessorMessage(const node_address& dest, const node_address& whoInitiator, const uint160& whatID);
        chord_message& createSuccessorMessage(const node_address& dest, const uint messageID, const node_address& foundIDwithSocket);
        chord_message& createSingleMessage(const node_address& addr);
        
        event_type eventType(const chord_message* mess = nullptr);

        uint nextUniqueMessageID();
    };
}
#endif