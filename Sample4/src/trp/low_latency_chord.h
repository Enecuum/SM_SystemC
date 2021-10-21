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
        vector<message_buffer> m_buffer;
        
        uint m_howManyBuffers;
        int  m_indexLastBufferCall;

        //map<uint, node_address> m_fingersPos;
        //map<uint, node_address> m_fingersNeg;
        //map<uint, sc_time>      m_latency;
        node_address m_precessor;
        node_address m_successor;
        vector<node_address>  m_fingersPos;
        vector<node_address>  m_fingersNeg;
        vector<sc_time>       m_latency;
        map<uint160, sc_time>       m_isAcked;
        chord_conf_parameters m_confParams; 

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

        void findSuccessor(const chord_message* mess);

       
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


        void sendMessage(const chord_message & mess);

        string& state2str(const finite_state& state);
       
    };
}
#endif