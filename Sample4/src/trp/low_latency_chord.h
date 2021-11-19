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
        DO_REQUEST,
        ACTION_UNKNOWN
    };

    enum event_type {
        MIN_EVENT_TYPE = 0,
        CALLED_BY_ANOTHER_STATE,
        RX_MESS_RECEIVED,
        TX_MESS_SHOULD_SEND,
        TIMER_EXPIRED,
        APPTXREQUEST,
        UNAVAILABLE_NODE,
        MAX_EVENT_TYPE,
        EVENT_TYPE_UNKNOWN
    };


    
    
    class low_latency_chord: public sc_module,
                             public log
    {
    public:
        sc_port<trp_llchord_if> trp_port;

    private:
        node_address_latency m_nodeAddr;                        //�����, ������������ ��� ������������� ���� �� Transport+ ������ �� ID, �����������, ��� SHA-1
        uint         m_currSeed;
        
        sc_event     m_eventCore;

        finite_state       m_state;
        vector< message_buffer<chord_message> > m_buffer;
        message_buffer<chord_message> m_timersBuffer;
        
        uint m_howManyBuffers;
        int  m_indexLastBufferCall;        
        int m_timerBufferIndex;
        int m_txMessBufferIndex;
        buffer_container::iterator m_timerItForRemove;

        //map<uint, node_address> m_cwFingers;
        //map<uint, node_address> m_ccwFingers;
        //map<uint, sc_time>      m_latency;
        vector<node_address> m_seedAddrs;
        node_address_latency m_predecessor;
        node_address_latency m_successor;
        vector<node_address_latency>  m_cwFingers;  //clock wise fingers
        vector<node_address_latency>  m_ccwFingers; //counter clock wise fingers
        map<uint, sc_time>    m_latency;        
        chord_conf_parameters m_confParams; 
        map<uint160, sc_time> m_isAcked;

        bool m_isSuccessorSet;
        bool m_isPredecessorSet;
        
        vector<uint160> m_fingerMask;
        bool m_isClockwise;
        uint m_messageID;

        bool m_canTakeDelayedTimers;

        string m_errCode;
        //uint m_cwFingerIndexToUpdate;
        //uint m_ccwFingerIndexToUpdate;
        uint m_cwFingerIndex;
        uint m_ccwFingerIndex;
        
        vector<node_address_latency> m_unavailableFingers;
        vector<vector<uint>> allowableTimers;
        vector<vector<uint>> allowableRxMess;
        vector<vector<uint>> allowableTxMess;
        vector<vector<uint>> allowableAppreqs;

        class message_id_with_state {
        public:
            uint id;
            uint type;
            finite_state state;
            sc_time time;

            message_id_with_state(const uint _id, const uint _type, const finite_state _state, const sc_time& _time) {
                this->id    = _id;
                this->type  = _type;
                this->state = _state;
                this->time  = _time;
            }
        };

        //vector<message_id_with_state> m_messMemoryList;
        message_buffer<chord_message> m_txMemoryList;
        

    public:
        SC_HAS_PROCESS(low_latency_chord);

        low_latency_chord(sc_module_name name);
        ~low_latency_chord();

        void              setNetworkAddress(const network_address& netwAddr);      
        network_address&  getNetworkAddress();        
        node_address&     getNodeAddress();

        void setSeedNodes(const vector<network_address>& seed);
        void setConfParameters(const chord_conf_parameters& params);
        void pushNewMessage(const chord_message& mess, const bool toBack = true);   

    private:
        void preinit();
        void core();        

        void hardReset();
        void softReset();
        void flush();
        bool doResetFlushIfMess(const chord_message& mess, const bool existMess); 

        void goStateLoad(); 
        void goStateInit(const chord_message& mess, const bool existMess);
        void goStateJoin(const chord_message& mess, const bool existMess);
        void goStateIdle(const chord_message& mess, const bool existMess);
        void goStateIndata(const chord_message& mess, const bool existMess);
        void goStateService(const chord_message& mess, const bool existMess);
        void goStateUpdate(const chord_message& mess , const bool existMess);
        void goStateApprequest(const chord_message& mess, const bool existMess);
       

        

        bool isAddrValid(const chord_message& mess);
        bool isMessageIDvalid(const chord_message& mess, buffer_container::iterator& it);
        int  checkMessage(const chord_message& mess, buffer_container::iterator& it, const string& errCode);
        int  checkMessage(const chord_message& mess, chord_timer_message& timer, const string& errCode);

        int  chordMessType2buffIndex(const uint type);
        chord_message firstMessByPriority(bool& exist);
        void eraseFirstMess();
        void eraseTxMess(const chord_tx_message_type type);
        void pushNewTimer(const chord_timer_message& type, const bool toBack = true);
        void pushNewTimer(const chord_timer_message_type type, const uint retryCounter, const uint requestCounter, const chord_byte_message_fields& retryMess, const bool toBack = true);
        void removeTimer(const chord_timer_message_type timerType, const chord_tx_message_type retryMessType, const uint retryMessID);
        void removeTimer(buffer_container::iterator timerIt);
        void removeTimers(const vector<chord_timer_message_type>& timerType, const vector<chord_tx_message_type>& retryMessTypes, const uint retryMessID);
        buffer_container::iterator findMessageOnTimersWithRetryParams(bool& exist, const chord_timer_message_type timerType, const vector<chord_tx_message_type>& retryMessType, const uint messageID);
        
        void sendMessage(const chord_message& mess);

        string state2str(const finite_state& state) const;

        chord_action findSuccessor(const uint160& id, node_address& found);
        chord_action findPredecessor(const uint160& searchedID, node_address& found);
        bool isClockWiseDirection(const uint160& id);
        node_address closestPrecedingNode(const uint160& id);
        bool isInRange(const uint160& id, const uint160& A, const bool includeA, const uint160& B, const bool includeB);

        chord_message createMessage(const chord_message& params);


        chord_message createJoinMessage(const node_address& dest);
        chord_message createNotifyMessage(const node_address& dest);
        chord_message createAckMessage(const node_address& dest, const uint messageID);
        chord_message createFindSuccessorMessage(const node_address& dest, const node_address& whoInitiator, const uint160& whatID);
        chord_message createSuccessorMessage(const node_address& dest, const uint messageID, const node_address& foundIDwithSocket);
        chord_message createFindPredecessorMessage(const node_address& dest, const node_address& whoInitiator, const uint160& whatID);
        chord_message createPredecessorMessage(const node_address& dest, const uint messageID, const node_address& foundIDwithSocket);
        chord_message createSingleMessage(const node_address& addr);
        
        event_type eventType(const chord_message& mess, const bool existMess);
        string logHeadStateString(const chord_message& mess, const bool existMess);
        uint   nextUniqueMessageID();

        void setFingerRemoveTimers(const chord_byte_message_fields& rxMess, const chord_timer_message& timer);
        void setCopyPreviousFinger();
        bool setSuccessorRemoveTimers(const chord_byte_message_fields& rxMess, const chord_timer_message& timer);
        bool setPredecessor(const chord_byte_message_fields& rxMess, const chord_timer_message& timer);
        bool setSuccessorStabilize(const chord_byte_message_fields& rxMess, const chord_timer_message& timer);
        void setNextFingerToUpdate();
        
        bool issueMessagePushTimers(const chord_tx_message_type& type, const bool isRetry = false, const uint requestCounter = 0, const chord_byte_message_fields rxMess = chord_byte_message_fields(), const chord_timer_message expiredTimer = chord_timer_message(), const chord_action action = DO_REQUEST, const node_address fingerAddr = node_address());
        bool repeatMessage(const chord_tx_message_type& type, const chord_byte_message_fields& rxMess, const chord_timer_message& expiredTimer);
        void setNextState(const finite_state& state);
                       
        typedef void(low_latency_chord::*FP)(const chord_message&, const bool);  //FP is function pointer
        vector<FP> arrFunctions;
        
        FP mess2state(const event_type eventType, const chord_message& mess);

        bool pushTxMessageMemoryList(const chord_message& mess);
        bool eraseTxMessageMemoryList(const chord_message& mess);
        buffer_container::iterator findTxMessageMemoryList(bool& exist, const uint messageID, const uint type, const finite_state state = STATE_UNKNOWN);
        finite_state findStateOnRxReplyMess(const chord_message& rxMess);
    };
}
#endif