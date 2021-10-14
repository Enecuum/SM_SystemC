#include "trp/low_latency_chord.h"

namespace P2P_MODEL
{

    string& state2str(const finite_state& state) {
        static string res;
        switch (state) {
        case STATE_LOAD:       return res = "STATE_LOAD";
        case STATE_INIT:       return res = "STATE_INIT";
        case STATE_JOIN:       return res = "STATE_JOIN";
        case STATE_IDLE:       return res = "STATE_IDLE";
        case STATE_INDATA:     return res = "STATE_INDATA";
        case STATE_SERVICE:    return res = "STATE_SERVICE";
        case STATE_UPDATE:     return res = "STATE_UPDATE";
        case STATE_APPREQUEST: return res = "STATE_APPREQUEST";
        case STATE_OFF:        return res = "STATE_OFF";
        default:               return res = "STATE_UNKNOWN";
        }
    }



    low_latency_chord::low_latency_chord(sc_module_name name) : sc_module(name) {
        SC_METHOD(core);
        dont_initialize();
        sensitive << m_eventCore;

        preinit();
    }


    low_latency_chord::~low_latency_chord() {    }


    void low_latency_chord::preinit() {
        m_nodeAddr.clear();                        //ÀÄÐÅÑ, ÈÑÏÎËÜÇÓÅÌÛÉ ÄËß ÈÄÅÍÒÈÔÈÊÀÖÈÈ ÓÇËÀ ÍÀ Transport+ ÓÐÎÂÍÅ ïî ID, âû÷èñëÿåìîì, êàê SHA-1
        m_currSeed = 0;
        m_eventCore.cancel();

        m_state = STATE_OFF;
        m_buffer.clear();

        m_howManyBuffers = MAX_BUFF_TYPE;
        m_indexLastBufferCall = 0;

        m_precessor.clear();
        m_successor.clear();
        m_fingersPos.clear();
        m_fingersNeg.clear();
        m_latency.clear();
        m_confParams.clear();

        m_isSuccessorSet = false;
        m_isPrecessorSet = false;
        m_currFinger = 0;


        //Set priority
        //priority "1" and immediate "true"  is the highest priority
        //priority "5" and immediate "false" is the lowest priority
        uint priority = 1;
        m_buffer.resize(m_howManyBuffers, req_buffer());
        m_buffer[BUFF_CONFIG_REQ].set(BUFF_CONFIG_REQ, MAX_DEEP_BUFF_CONFIG_REQ, MAX_SIZE_BUFF_CONFIG_REQ, true, priority++);
        m_buffer[BUFF_TIMER_REQ] .set(BUFF_TIMER_REQ,  MAX_DEEP_BUFF_TIMER_REQ, MAX_SIZE_BUFF_TIMER_REQ, true, priority++);

        m_buffer[BUFF_TX_JOIN]   .set(BUFF_TX_JOIN,   MAX_DEEP_BUFF_MESS_REQ, MAX_SIZE_BUFF_MESS_REQ, false, priority++);
        m_buffer[BUFF_TX_NOTIFY] .set(BUFF_TX_NOTIFY, MAX_DEEP_BUFF_MESS_REQ, MAX_SIZE_BUFF_MESS_REQ, false, priority++);
        m_buffer[BUFF_TX_ACK]    .set(BUFF_TX_ACK,    MAX_DEEP_BUFF_MESS_REQ, MAX_SIZE_BUFF_MESS_REQ, false, priority++);
        m_buffer[BUFF_TX_REPLY_FIND_SUCCESSOR].set(BUFF_TX_REPLY_FIND_SUCCESSOR, MAX_DEEP_BUFF_MESS_REQ, MAX_SIZE_BUFF_MESS_REQ, false, priority++);
        m_buffer[BUFF_TX_FIND_SUCCESSOR].set(BUFF_TX_FIND_SUCCESSOR, MAX_DEEP_BUFF_MESS_REQ, MAX_SIZE_BUFF_MESS_REQ, false, priority++);
        m_buffer[BUFF_TX_FWD_BROADCAST] .set(BUFF_TX_FWD_BROADCAST,  MAX_DEEP_BUFF_MESS_REQ, MAX_SIZE_BUFF_MESS_REQ, false, priority++);
        m_buffer[BUFF_TX_FWD_MULTICAST] .set(BUFF_TX_FWD_MULTICAST,  MAX_DEEP_BUFF_MESS_REQ, MAX_SIZE_BUFF_MESS_REQ, false, priority++);
        m_buffer[BUFF_TX_FWD_SINGLE].set(BUFF_TX_FWD_SINGLE,  MAX_DEEP_BUFF_MESS_REQ, MAX_SIZE_BUFF_MESS_REQ, false, priority++);
        m_buffer[BUFF_TX_BROADCAST] .set(BUFF_TX_BROADCAST,   MAX_DEEP_BUFF_MESS_REQ, MAX_SIZE_BUFF_MESS_REQ, false, priority++);
        m_buffer[BUFF_TX_MULTICAST] .set(BUFF_TX_MULTICAST,   MAX_DEEP_BUFF_MESS_REQ, MAX_SIZE_BUFF_MESS_REQ, false, priority++);
        m_buffer[BUFF_TX_SINGLE]    .set(BUFF_TX_SINGLE,      MAX_DEEP_BUFF_MESS_REQ, MAX_SIZE_BUFF_MESS_REQ, false, priority++);
        //m_buffer[BUFF_TX_MESS].set(BUFF_TX_MESS, MAX_DEEP_BUFF_TX_MESS, MAX_SIZE_BUFF_TX_MESS, false, 5);
        m_buffer[BUFF_MESS_REQ].set(BUFF_MESS_REQ, MAX_DEEP_BUFF_MESS_REQ, MAX_SIZE_BUFF_MESS_REQ, false, priority++);
        m_buffer[BUFF_RX_MESS] .set(BUFF_RX_MESS,  MAX_DEEP_BUFF_RX_MESS,  MAX_SIZE_BUFF_RX_MESS,  false, priority++);
        
        //Sort buffers by priority and immediate fields of req_buffer class, comparison's operator for sorting is defined in req_buffer class
        std::sort(m_buffer.begin(), m_buffer.end());       
        
        //LOG
        for (int i = 0; i < m_buffer.size(); ++i)
            cout << m_buffer[i].toStr() << endl;
    }


    void low_latency_chord::setNetworkAddress(const network_address& addr) {
        m_confParams.netwAddr = addr;
        m_nodeAddr.set(addr);
    }


    network_address& low_latency_chord::getNetworkAddress() {
        return m_confParams.netwAddr;
    }

    node_address& low_latency_chord::getNodeAddress() {
        return m_nodeAddr;
    }


    void low_latency_chord::setSeedNodes(const vector<network_address>& seed) {
        m_confParams.seed.clear();
        for (uint i = 0; i < seed.size(); ++i) {
            node_address addr; 
            addr.set(seed[i]);
            m_confParams.seed.push_back( addr );
        }
    }


    void low_latency_chord::setConfParameters(const chord_conf_parameters& params) {
        m_confParams = params;
        m_nodeAddr.set(m_confParams.netwAddr);
    }


    void low_latency_chord::pushNewRequest(const chord_request& req) {
        chord_request r = req;
        r.appearanceTime = sc_time_stamp();

        int i = chordReqType2buffIndex(r.type);
        if ((i == ERROR) || (i >= MAX_BUFF_TYPE)) {
            //ERROR
            m_logText = "pushNewRequest" + LOG_SPACER + r.toStr();
            msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
        }
        else {
            
            if (m_buffer[i].push(r) == false) {
                //OVERFLOW BUFFER
                m_logText = "pushNewRequest" + LOG_SPACER + r.toStr() + LOG_SPACER + LOG_ERROR_OVERFLOW;
                msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
            }
            else {
                m_logText = "pushNewRequest" + LOG_SPACER + r.toStr();                
                msgLog(name(), LOG_TXRX, LOG_IN, m_logText, DEBUG_LOG | EXTERNAL_LOG);
            
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
            m_indexLastBufferCall = BUFFER_NOT_CHOOSEN;
            for (uint i = 0; i < m_buffer.size(); ++i) {
                p = m_buffer[i].firstReqPointerByImmediate();
                if (p != nullptr) {
                    m_indexLastBufferCall = i;
                    return p;
                }
            }

            buffersEmpty = true;
            if (m_indexLastBufferCall == BUFFER_NOT_CHOOSEN) {
                for (uint i = 0; i < m_buffer.size(); ++i) {
                    if (m_buffer[i].size() > 0) {
                        buffersEmpty = false;    
                        m_buffer[i].clearReqsCounter();
                    }
                }
            }
        }
        
        return nullptr;
    }


    void low_latency_chord::eraseFirstReq() {
        if ((m_indexLastBufferCall != BUFFER_NOT_CHOOSEN) && (m_indexLastBufferCall < m_buffer.size())) {
            m_buffer[m_indexLastBufferCall].eraseFirstReq();
            m_indexLastBufferCall = BUFFER_NOT_CHOOSEN;
        }
    }


    void low_latency_chord::core() {
        string currTime = sc_time_stamp().to_string();
        chord_request* p = firstReqByPriority();
        if (p != nullptr) {
            
            m_logText = "core" + LOG_SPACER + p->type2str();
            msgLog(name(), LOG_TXRX, LOG_INFO, m_logText, DEBUG_LOG | INTERNAL_LOG);

            switch (m_state)
            {
            case STATE_OFF:
                if (p->type == CHORD_HARD_RESET) {
                    eraseFirstReq();
                    hardReset();   
                    goStateLoad();
                } break;

            case STATE_INIT:
                break;
            
            case STATE_JOIN:
                if (p->type == CHORD_TX_JOIN) {
                    
                }
                break;
            
            case STATE_IDLE:
                break;
            
            case STATE_INDATA:
                break;
            
            case STATE_SERVICE:
                break;
            
            case STATE_UPDATE:
                break;
            
            case STATE_APPREQUEST:
                break;

            default:
                //ERROR
                m_logText = "core" + LOG_SPACER + state2str(m_state);
                msgLog(name(), LOG_TXRX, LOG_INFO, m_logText, DEBUG_LOG | ERROR_LOG);
                return;
            }
            eraseFirstReq();

            /*
            switch (p->type)
            {
            case CHORD_HARD_RESET:
                
                break;
            
            case CHORD_SOFT_RESET:
                if (m_state != STATE_OFF) {
                    eraseFirstReq();
                    soft_reset();
                    init();
                } break;
            case CHORD_FLUSH:
                if (m_state != STATE_OFF) {
                    eraseFirstReq();
                    flush();
                    init();
                } break;

            case CHORD_BROADCAST:
            case CHORD_MULTICAST:
            case CHORD_SINGLE:
                if (m_state != STATE_OFF) {
                    ;
                } break;

            case CHORD_CONF: 
                if (m_state != STATE_OFF) {
                    eraseFirstReq();
                    flush();
                    init();
                } break;


            case CHORD_RX_JOIN:
            case CHORD_RX_NOTIFY:
            case CHORD_RX_ACK:
            case CHORD_RX_REPLY_FIND_SUCESSOR:
            case CHORD_RX_FIND_SUCCESSOR:
            case CHORD_RX_BROADCAST:
            case CHORD_RX_MULTICAST:
            case CHORD_RX_SINGLE:


            case CHORD_TX_JOIN:
                if (m_state == STATE_JOIN) {
                } break;

            case CHORD_TX_FIND_SUCCESSOR: 
                if ((m_state == STATE_JOIN) || (m_state == STATE_SERVICE) || (m_state == STATE_UPDATE)) {
                } break;

            case CHORD_TX_NOTIFY:
            case CHORD_TX_ACK:


            case CHORD_TX_REPLY_FIND_SUCESSOR:
            
            case CHORD_TX_BROADCAST:
            case CHORD_TX_MULTICAST:
            case CHORD_TX_SINGLE:
                break;

            default:
                //ERROR
                m_logText = "core" + LOG_SPACER + p->toStr();
                msgLog(name(), LOG_TXRX, LOG_INFO, m_logText, ALL_LOG);
                break;
            }*/
            
                       
            m_eventCore.notify(0, SC_NS);
        }
    }

    
    void low_latency_chord::goStateLoad() {
        m_state = STATE_LOAD;
        //Needs loading new config parameters
        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state), DEBUG_LOG | INTERNAL_LOG);
        
        
        goStateInit();
    }


    void low_latency_chord::goStateInit() {
        m_state = STATE_INIT;
        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state), DEBUG_LOG | INTERNAL_LOG);


        //Initializing of fingers, successor, precessor, latency by default values
        m_currSeed = 0;
        m_latency.resize(m_confParams.fingersSize * 2, sc_time(0.1, SC_SEC));
        if (m_confParams.seed.size() == 0) {
            m_logText = "init" + LOG_SPACER + string("NO SEED");
            msgLog(name(), LOG_TXRX, LOG_INFO, m_logText, ALL_LOG);
            
            m_fingersNeg.clear();
            m_fingersPos.clear();
            m_precessor.clear();
            m_successor.clear();
            goStateIdle();
        }
        else {
            m_fingersNeg.resize(m_confParams.fingersSize, m_confParams.seed.front());
            m_fingersPos.resize(m_confParams.fingersSize, m_confParams.seed.front());
            m_precessor = m_confParams.seed.front();
            m_successor = m_confParams.seed.front();            
            goStateJoin();
        }
    }


    void low_latency_chord::goStateJoin() {
        m_state = STATE_JOIN;
        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state), DEBUG_LOG | INTERNAL_LOG);

        chord_request req;
        if (m_isSuccessorSet == false) {
            req.destination.push_back(m_confParams.seed.front());
            req.source = m_nodeAddr;
            req.type = CHORD_TX_JOIN;
            req.appearanceTime = sc_time_stamp();
            
            if (m_counterJoin <= m_confParams.CtxJoin) {

                req.reqCopy = req.clone(); 
                pushNewRequest(req);

                chord_request timer;
                timer.type = CHORD_TIMER_REPLY_FIND_SUCC_JOIN;
                pushNewRequest(timer);
                m_eventCore.notify(0, SC_NS);
            }
        }

        
    }


    void low_latency_chord::goStateIdle() {
        m_state = STATE_IDLE;
        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state), DEBUG_LOG | INTERNAL_LOG);
    }


    void low_latency_chord::goStateIndata()  {
        m_state = STATE_INDATA;
        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state), DEBUG_LOG | INTERNAL_LOG);
    }


    void low_latency_chord::goStateService() {
        m_state = STATE_SERVICE;
        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state), DEBUG_LOG | INTERNAL_LOG);
    }


    void low_latency_chord::goStateUpdate()  { 
        m_state = STATE_UPDATE;
        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state), DEBUG_LOG | INTERNAL_LOG);
    }

    void low_latency_chord::goStateApprequest() {    }


    //void low_latency_chord::finiteStateMachine(chord_request* p) {
 
    //}


    int low_latency_chord::chordReqType2buffIndex(const uint type) {
        static map<uint, uint> mapBuffIndex;
        uint buffType = 0;
        switch (type) {            
        case CHORD_HARD_RESET:
        case CHORD_SOFT_RESET:
        case CHORD_FLUSH:        buffType = BUFF_CONFIG_REQ; break;

        case CHORD_TIMER_ACK:                   
        case CHORD_TIMER_REPLY_FIND_SUCC:       
        case CHORD_TIMER_REPLY_FIND_SUCC_JOIN:  
        case CHORD_TIMER_UPDATE: buffType = BUFF_TIMER_REQ;   break;

        case CHORD_SINGLE:
        case CHORD_MULTICAST:
        case CHORD_BROADCAST:    buffType = BUFF_MESS_REQ; break;

        case CHORD_RX_JOIN:
        case CHORD_RX_NOTIFY:
        case CHORD_RX_ACK:
        case CHORD_RX_REPLY_FIND_SUCESSOR:
        case CHORD_RX_FIND_SUCCESSOR:
        case CHORD_RX_BROADCAST:
        case CHORD_RX_MULTICAST:
        case CHORD_RX_SINGLE:    buffType = BUFF_RX_MESS; break;
                                 
        case CHORD_TX_JOIN:      buffType = BUFF_TX_JOIN;   break;
        case CHORD_TX_NOTIFY:    buffType = BUFF_TX_NOTIFY; break;
        case CHORD_TX_ACK:       buffType = BUFF_TX_ACK;    break;
        case CHORD_TX_REPLY_FIND_SUCESSOR: buffType = BUFF_TX_REPLY_FIND_SUCCESSOR; break;
        case CHORD_TX_FIND_SUCCESSOR: buffType = BUFF_TX_FIND_SUCCESSOR; break;
        case CHORD_TX_FWD_BROADCAST:  buffType = BUFF_TX_FWD_BROADCAST;  break;
        case CHORD_TX_FWD_MULTICAST:  buffType = BUFF_TX_FWD_MULTICAST;  break;
        case CHORD_TX_FWD_SINGLE:     buffType = BUFF_TX_FWD_SINGLE;     break;
        case CHORD_TX_BROADCAST:      buffType = BUFF_TX_BROADCAST;      break;
        case CHORD_TX_MULTICAST:      buffType = BUFF_TX_MULTICAST;      break;
        case CHORD_TX_SINGLE:         buffType = BUFF_TX_SINGLE;         break;
        //buffType = BUFF_TX_MESS;              

        default:
            //ERROR
            msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, "chordReqType2buffIndex", DEBUG_LOG | ERROR_LOG | EXTERNAL_LOG);
            return ERROR;
        }

        map<uint, uint>::iterator it = mapBuffIndex.find(buffType);
        if (it == mapBuffIndex.end()) {
            for (uint buffI = 0; buffI < m_buffer.size(); ++buffI) {
                if (m_buffer[buffI].buffType() == buffType) {
                    mapBuffIndex.insert(pair<uint, uint>(buffType, buffI));
                    return buffI;
                }
            }
            return ERROR;
        } 
        else
            return (*it).second;
    }



    void low_latency_chord::hardReset() {
        m_eventCore.cancel();
        
        //Reqs, timers are resetted
        flush();

        //Fingers, latency, precessor, successor are resetted
        softReset();
    }


    void low_latency_chord::softReset() {
        m_eventCore.cancel();
        //Reqs, timers are stored
        //Fingers, latency, precessor, successor are resetted
        m_fingersPos.clear();
        m_fingersNeg.clear();
        m_latency.clear();
        m_successor.clear();
        m_precessor.clear();
        m_currSeed = 0;
        m_isSuccessorSet = false;
        m_isPrecessorSet = false;
        m_currFinger = 0;
        m_counterJoin = 0;
    }

    void low_latency_chord::flush() {
        m_eventCore.cancel();

        //Reqs, timers are resetted
        for (uint i = 0; i < m_buffer.size(); ++i)
            m_buffer[i].clearReqs();

        //Fingers, latency, precessor, successor are stored
    }



}
