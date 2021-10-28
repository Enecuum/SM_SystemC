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
    

    enum finite_state {
        STATE_LOAD = 0,
        STATE_INIT,
        STATE_JOIN,
        STATE_IDLE,
        STATE_INDATA,
        STATE_SERVICE,
        STATE_UPDATE,
        STATE_APPREQUEST,
        MAX_FINITE_STATE,
        STATE_OFF,
        STATE_UNKNOWN
    };

    


    enum chord_action {
        DO_REPLY = 0,
        DO_FORWARD,
        DROP_MESSAGE,
        ACTION_UNKNOWN
    };

    class low_latency_chord: public sc_module,
                             public log
    {
    public:
        sc_port<trp_llchord_if> trp_port;

    private:
        node_address m_nodeAddr;                        //�����, ������������ ��� ������������� ���� �� Transport+ ������ �� ID, �����������, ��� SHA-1
        uint         m_currSeed;
        
        sc_event     m_eventCore;

        finite_state       m_state;
        vector< message_buffer<chord_message> > m_buffer;
        
        uint m_howManyBuffers;
        int  m_indexLastBufferCall;        

        //map<uint, node_address> m_cwFingers;
        //map<uint, node_address> m_ccwFingers;
        //map<uint, sc_time>      m_latency;
        vector<node_address> m_seedAddrs;
        node_address m_precessor;
        node_address m_successor;
        vector<node_address>  m_cwFingers;  //clock wise fingers
        vector<node_address>  m_ccwFingers; //counter clock wise fingers
        vector<sc_time>       m_latency;        
        chord_conf_parameters m_confParams; 
        map<uint160, sc_time> m_isAcked;

        bool m_isSuccessorSet;
        bool m_isPrecessorSet;
        uint m_currFinger;
        uint m_counterJoin;



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
        //void finiteStateMachine(chord_message* p);

        void hardReset();
        void softReset();
        void flush();

        chord_action findSuccessor(const chord_message* mess, node_address& existAddr);

       
        void goStateLoad(); 
        void goStateInit(const chord_message* mess = nullptr);
        void goStateJoin(const chord_message* mess = nullptr);
        void goStateIdle(const chord_message* mess = nullptr);
        void goStateIndata(const chord_message* mess = nullptr);
        void goStateService(const chord_message* mess = nullptr);
        void goStateUpdate(const chord_message* mess = nullptr);
        void goStateApprequest(const chord_message* mess = nullptr);

        bool doResetFlushIfMess(const chord_message* mess);
        bool isMessValid(const chord_message* mess);

        chord_message* firstMessByPriority();
        void eraseFirstMess();
        
        int chordMessType2buffIndex(const uint type);


        void sendMessage(const chord_message& byteMess);

        string& state2str(const finite_state& state);
       
        bool isClockWiseDirection(const uint160& id);

        uint160& closestPrecedingNode(const uint160& id);

        uint160& closestPrecedingNodeCwFingers(const uint160& id);
    };
}
#endif