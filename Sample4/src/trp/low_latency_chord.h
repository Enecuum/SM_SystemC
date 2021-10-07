#ifndef __LOW_LATENCY_CHORD_H__
#define __LOW_LATENCY_CHORD_H__

#include "log.h"
#include "inc.h"
#include "sha1.hpp"
#include "req_buffer.h"

using namespace std;



namespace P2P_MODEL
{

   



    enum finite_state {
        LOAD = 0,
        INIT,
        JOIN,
        IDLE,
        INDATA,
        SERVICE,
        UPDATE,
        APPREQUEST,
        MAX_FINITE_STATE,
        OFF
    };


    const int MAX_SIZE_BUFF_CONFIG_REQ = 100;
    const int MAX_SIZE_BUFF_TIMER_REQ = 100;
    const int MAX_SIZE_BUFF_MESS_REQ = 100;
    const int MAX_SIZE_BUFF_RX_MESS = 100;
    const int MAX_SIZE_BUFF_TX_MESS = MAX_SIZE_BUFF_MESS_REQ + MAX_SIZE_BUFF_RX_MESS + 1;

    const int ERROR = -1;
    const int MAX_DEEP_BUFF_CONFIG_REQ = MAX_SIZE_BUFF_CONFIG_REQ;
    const int MAX_DEEP_BUFF_TIMER_REQ = MAX_SIZE_BUFF_TIMER_REQ;
    const int MAX_DEEP_BUFF_MESS_REQ = 1;
    const int MAX_DEEP_BUFF_RX_MESS = 1;
    const int MAX_DEEP_BUFF_TX_MESS = MAX_DEEP_BUFF_MESS_REQ + MAX_DEEP_BUFF_RX_MESS + 1;

    const int BUFFER_NOT_CHOOSEN = -1;


    class low_latency_chord: public sc_module,
                             public log
    {
    private:
        network_address m_netwAddr;
        node_address m_nodeAddr;                        //¿ƒ–≈—, »—œŒÀ‹«”≈Ã€… ƒÀﬂ »ƒ≈Õ“»‘» ¿÷»» ”«À¿ Õ¿ Transport+ ”–Œ¬Õ≈ ÔÓ ID, ‚˚˜ËÒÎˇÂÏÓÏ, Í‡Í SHA-1
        vector<network_address> m_seed;
        
        finite_state m_state;
        vector<req_buffer> m_buffer;
        int m_IndexLastBufferCall;
        sc_event m_eventCore;
        
        uint m_howManyBuffers;



    public:
        SC_HAS_PROCESS(low_latency_chord);

        low_latency_chord(sc_module_name name);
        ~low_latency_chord();

        void              setNetworkAddress(const network_address& netwAddr);      
        network_address&  getNetworkAddress();        
        node_address&     getNodeAddress();

        void setSeedNodes(const vector<network_address>& seed);
        void pushNewRequest(const chord_request& req);

    private:
        void preinit();
        void core();
        void init();
        void hard_reset();
        void soft_reset();
        void flush();

        chord_request* firstReqByPriority();
        void eraseFirstReq();
        
        int chordReqType2buffIndex(const uint type);
    };
}
#endif