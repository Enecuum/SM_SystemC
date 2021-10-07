#include "trp/low_latency_chord.h"

namespace P2P_MODEL
{
    ostream& operator<< (ostream& out, const node_address& r) {
        out << r.ip << " " << r.inSocket << " " << r.outSocket << " " << r.id.to_string(SC_HEX_US);
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
        m_howManyBuffers = MAX_BUFF_TYPE;
        m_IndexLastBufferCall = 0;

        //Set priority
        //priority "1" and immediate "true"  is the highest priority
        //priority "5" and immediate "false" is the lowest priority
        m_buffer.resize(m_howManyBuffers, req_buffer());
        m_buffer[BUFF_CONFIG_REQ].set(BUFF_CONFIG_REQ, MAX_DEEP_BUFF_CONFIG_REQ, MAX_SIZE_BUFF_CONFIG_REQ, true, 1);
        m_buffer[BUFF_TIMER_REQ].set(BUFF_TIMER_REQ, MAX_DEEP_BUFF_TIMER_REQ, MAX_SIZE_BUFF_TIMER_REQ, true, 2);
        m_buffer[BUFF_MESS_REQ].set(BUFF_MESS_REQ, MAX_DEEP_BUFF_MESS_REQ, MAX_SIZE_BUFF_MESS_REQ, false, 3);
        m_buffer[BUFF_RX_MESS].set(BUFF_RX_MESS, MAX_DEEP_BUFF_RX_MESS, MAX_SIZE_BUFF_RX_MESS, false, 4);
        m_buffer[BUFF_TX_MESS].set(BUFF_TX_MESS, MAX_DEEP_BUFF_TX_MESS, MAX_SIZE_BUFF_TX_MESS, false, 5);
        
        //Sort buffers by priority, comparison's operator of buffes is defined in req_buffer class
        std::sort(m_buffer.begin(), m_buffer.end());       
        
        //LOG
        for (int i = 0; i < m_buffer.size(); ++i)
            cout << m_buffer[i].toStr() << endl;
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
        chord_request& r = const_cast<chord_request&>(req);

        int i = chordReqType2buffIndex(r.type);
        if ((i == ERROR) || !(i < MAX_BUFF_TYPE)) {
            //ERROR
            m_strLogText = "pushNewRequest" + LOG_SPACER + r.toStr();
            msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, m_strLogText, DEBUG_LOG | ERROR_LOG | EXTERNAL_LOG);
        }
        else {
            if (m_buffer[i].push(r) == false) {
                //FEEDBACK
                m_strLogText = "pushNewRequest" + LOG_SPACER + r.toStr();
                msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, m_strLogText, DEBUG_LOG | ERROR_LOG | EXTERNAL_LOG);
            }
            else {
                m_strLogText = "pushNewRequest" + LOG_SPACER + r.toStr();                
                msgLog(name(), LOG_TXRX, LOG_IN, m_strLogText, DEBUG_LOG | EXTERNAL_LOG);
            
                m_eventCore.notify(0, SC_NS);
            }
        }
    }

    chord_request* low_latency_chord::firstReqByPriority() {                
        //m_buffer consists of array of sub-buffers as named "req_buffer".
        //req_buffers into m_buffer have already sorted by priority (service) order.
        //req_buffer in 0-position has the highest priority to service request.
        //method "firstReqPointerByImmediate" will return req with accordance immediate-priority
        
        bool buffersEmpty = false;
        while (buffersEmpty == false) {
            chord_request* p = nullptr;
            m_IndexLastBufferCall = BUFFER_NOT_CHOOSEN;
            for (uint i = 0; i < m_buffer.size(); ++i) {
                p = m_buffer[i].firstReqPointerByImmediate();
                if (p != nullptr) {
                    m_IndexLastBufferCall = i;
                    return p;
                }
            }
            

            buffersEmpty = true;
            if (m_IndexLastBufferCall == BUFFER_NOT_CHOOSEN) {
                for (uint i = 0; i < m_buffer.size(); ++i) {
                    if (m_buffer[i].size() > 0) {
                        buffersEmpty = false;    
                        m_buffer[i].resetReqCounter();
                    }
                }
            }
        }
        
        return nullptr;
    }


    void low_latency_chord::eraseFirstReq() {
        if ((m_IndexLastBufferCall != BUFFER_NOT_CHOOSEN) && (m_IndexLastBufferCall < m_buffer.size())) {
            m_buffer[m_IndexLastBufferCall].eraseFirstReq();
            m_IndexLastBufferCall = BUFFER_NOT_CHOOSEN;
        }
    }


    void low_latency_chord::core() {
        string currTime = sc_time_stamp().to_string();
        chord_request* p = firstReqByPriority();
        if (p != nullptr) {
            m_strLogText = "core" + LOG_SPACER + p->type2str();
            msgLog(name(), LOG_TXRX, LOG_INFO, m_strLogText, DEBUG_LOG | EXTERNAL_LOG);

            switch (p->type)
            {
                case CHORD_HARD_RESET:          hard_reset(); eraseFirstReq(); break;
                case CHORD_SOFT_RESET:          soft_reset(); eraseFirstReq(); break;
                case CHORD_FLUSH: low_latency_chord::flush(); eraseFirstReq(); break;
               
                case CHORD_BROADCAST:
                case CHORD_MULTICAST:
                case CHORD_SINGLE:
                
               
                case CHORD_RX_JOIN:
                case CHORD_RX_NOTIFY:
                case CHORD_RX_ACK:
                case CHORD_RX_REPLY_FIND_SUCESSOR:
                case CHORD_RX_FIND_SUCCESSOR:
                case CHORD_RX_BROADCAST:
                case CHORD_RX_MULTICAST:
                case CHORD_RX_SINGLE:
               
                case CHORD_TX_JOIN:
                case CHORD_TX_NOTIFY:
                case CHORD_TX_ACK:
                case CHORD_TX_REPLY_FIND_SUCESSOR:
                case CHORD_TX_FIND_SUCCESSOR:
                case CHORD_TX_BROADCAST:
                case CHORD_TX_MULTICAST:
                case CHORD_TX_SINGLE:
                    break;
            
                default:
                    //ERROR
                    msgLog(name(), LOG_TXRX, LOG_INFO, "core", DEBUG_LOG | EXTERNAL_LOG);
                    break;
            }
            
            eraseFirstReq();
            m_eventCore.notify(0, SC_NS);
        }
    }


    int low_latency_chord::chordReqType2buffIndex(const uint type) {
        static multimap<uint, uint> mapBuffIndex;
        uint buffType = 0;
        switch (type) {            
            case CHORD_HARD_RESET:
            case CHORD_SOFT_RESET:
            case CHORD_FLUSH:
                buffType = BUFF_CONFIG_REQ; 
                break;

            case CHORD_SINGLE:
            case CHORD_MULTICAST:
            case CHORD_BROADCAST:
                buffType = BUFF_MESS_REQ;
                break;

            case CHORD_RX_JOIN:
            case CHORD_RX_NOTIFY:
            case CHORD_RX_ACK:
            case CHORD_RX_REPLY_FIND_SUCESSOR:
            case CHORD_RX_FIND_SUCCESSOR:
            case CHORD_RX_BROADCAST:
            case CHORD_RX_MULTICAST:
            case CHORD_RX_SINGLE:
                buffType = BUFF_RX_MESS;
                break;

            case CHORD_TX_JOIN:
            case CHORD_TX_NOTIFY:
            case CHORD_TX_ACK:
            case CHORD_TX_REPLY_FIND_SUCESSOR:
            case CHORD_TX_FIND_SUCCESSOR:
            case CHORD_TX_BROADCAST:
            case CHORD_TX_MULTICAST:
            case CHORD_TX_SINGLE:
                buffType = BUFF_TX_MESS;
                break;

            default:
                //ERROR
                buffType = 0;
                msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, "chordReqType2buffIndex", DEBUG_LOG | ERROR_LOG | EXTERNAL_LOG);
                return ERROR;
        }

        multimap<uint, uint>::iterator it = mapBuffIndex.find(buffType);
        if (it == mapBuffIndex.end()) {
            for (int i = 0; i < m_buffer.size(); ++i) {
                if (m_buffer[i].buffType() == buffType) {
                    mapBuffIndex.insert(pair<uint, uint>(i, buffType));
                    return i;
                }
            }
            return ERROR;
        } 
        else
            return (*it).first;
    }


    void low_latency_chord::init() { }

    void low_latency_chord::hard_reset() {}
    void low_latency_chord::soft_reset() {}
    void low_latency_chord::flush() {}
}
