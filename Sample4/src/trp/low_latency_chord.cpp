#include "trp/low_latency_chord.h"

namespace P2P_MODEL
{
    ostream& operator<< (ostream& out, const node_address& r) {
        out << "node_address: ip " << r.ip << ", out socket " << r.outSocket << ", in socket " << r.inSocket << ", id: " << r.id.to_string(SC_HEX_US);
        return out;
    }



    low_latency_chord::low_latency_chord(sc_module_name name) : sc_module(name) {
        SC_METHOD(core);
        dont_initialize();
        sensitive << m_eventCore;

        preinit();
    }


    low_latency_chord::~low_latency_chord() {    }


    void low_latency_chord::preinit() {
        m_state = OFF;
        m_howManyBuffers = MAX_CHORD_REQ_TYPE;
        m_bufferReq.resize(m_howManyBuffers, vector<chord_request>());
        
        int maxBufferDeepForData = 10;
        m_maxBufferDeepProcess.resize(m_howManyBuffers, 10);
        m_maxBufferDeepProcess[CHORD_HARD_RESET] = ALL;
        m_maxBufferDeepProcess[CHORD_SOFT_RESET] = ALL;
        m_maxBufferDeepProcess[CHORD_FLUSH] = ALL;
    }


    void low_latency_chord::setNetworkAddress(const network_address& addr) {
        m_netwAddr = addr;
        m_nodeAddr.set(addr);
    }


    network_address& low_latency_chord::getNetworkAddress() {
        return m_netwAddr;
    }

    node_address& low_latency_chord::getNodeAddress() {
        return m_nodeAddr;
    }


    void low_latency_chord::setSeedNodes(const vector<network_address>& seed) {
        m_seed = seed;
    }


    void low_latency_chord::pushNewRequest(const chord_request& req) {
        if ((req.type >= 0) && (req.type << MAX_CHORD_REQ_TYPE)) {
            m_bufferReq.at(req.type).push_back(req);
            m_eventCore.notify(0, SC_NS);
            
            m_strLogText = "pushNewRequest";
            m_strLogText += " " + const_cast<chord_request&>(req).toStr();
            msgLog(name(), LOG_TXRX, LOG_IN, m_strLogText, DEBUG_LOG | EXTERNAL_LOG);
        }
    }

    vector<chord_request>::iterator low_latency_chord::getReqByPriority() {
        vector<chord_request>::iterator it;

        for (int i = 0; i < m_bufferReq.size(); ++i) {
                  ;
        }

        return it;
    }


    void low_latency_chord::core() {
        vector<chord_request>::iterator it = getReqByPriority();
       
    }

}