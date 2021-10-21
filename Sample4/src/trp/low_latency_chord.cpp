#include "trp/low_latency_chord.h"

namespace P2P_MODEL
{

 



    low_latency_chord::low_latency_chord(sc_module_name name) : sc_module(name) {
        SC_METHOD(core);
        dont_initialize();
        sensitive << m_eventCore;

        preinit();
    }


    low_latency_chord::~low_latency_chord() {    }


    void low_latency_chord::preinit() {
        m_state = STATE_OFF;
        m_nodeAddr.clear();                        //ÀÄÐÅÑ, ÈÑÏÎËÜÇÓÅÌÛÉ ÄËß ÈÄÅÍÒÈÔÈÊÀÖÈÈ ÓÇËÀ ÍÀ Transport+ ÓÐÎÂÍÅ ïî ID, âû÷èñëÿåìîì, êàê SHA-1
        m_confParams.setDefaultTimersCountersFingersSize();
        hardReset();
                
        //Create buffers
        // 
        //Set priority
        //priority "1" and immediate "true"  is the highest priority
        //priority "5" and immediate "false" is the lowest priority
        uint priority = 1;
        m_indexLastBufferCall = 0;
        m_howManyBuffers = MAX_BUFF_TYPE;
        m_buffer.resize(m_howManyBuffers, message_buffer());
        m_buffer[BUFF_CONFIG].set(BUFF_CONFIG, MAX_DEEP_BUFF_CONFIG, MAX_SIZE_BUFF_CONFIG, true, priority++);
        m_buffer[BUFF_TIMER] .set(BUFF_TIMER,  MAX_DEEP_BUFF_TIMER, MAX_SIZE_BUFF_TIMER, true, priority++);

        m_buffer[BUFF_TX_JOIN]   .set(BUFF_TX_JOIN,   MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_NOTIFY] .set(BUFF_TX_NOTIFY, MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_ACK]    .set(BUFF_TX_ACK,    MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_REPLY_FIND_SUCCESSOR].set(BUFF_TX_REPLY_FIND_SUCCESSOR, MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_FIND_SUCCESSOR].set(BUFF_TX_FIND_SUCCESSOR, MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_FWD_BROADCAST] .set(BUFF_TX_FWD_BROADCAST,  MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_FWD_MULTICAST] .set(BUFF_TX_FWD_MULTICAST,  MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_FWD_SINGLE].set(BUFF_TX_FWD_SINGLE,  MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_BROADCAST] .set(BUFF_TX_BROADCAST,   MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_MULTICAST] .set(BUFF_TX_MULTICAST,   MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_SINGLE]    .set(BUFF_TX_SINGLE,      MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);        
        m_buffer[BUFF_APPTXDATA].set(BUFF_APPTXDATA, MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_RX_MESS] .set(BUFF_RX_MESS,  MAX_DEEP_BUFF_RX_MESS,  MAX_SIZE_BUFF_RX_MESS,  false, priority++);
        
        //Sort buffers by priority and immediate fields of message_buffer class, comparison's operator for sorting is defined in message_buffer class
        std::sort(m_buffer.begin(), m_buffer.end());       
        
        //Print info about buffers (priority...)
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


    void low_latency_chord::pushNewMessage(const chord_message& mess) {
        //chord_message& r = const_cast<chord_message&>(mess);
        chord_message r = const_cast<chord_message&>(mess);        
        r.appearanceTime = sc_time_stamp();

        int i = chordMessType2buffIndex(r.type);
        if ((i == ERROR) || (i >= MAX_BUFF_TYPE)) {
            //ERROR
            m_logText = "pushNewMessage" + LOG_TAB + r.toStr();
            msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
        }
        else {
            
            if (m_buffer[i].push(r) == false) {
                //OVERFLOW BUFFER
                m_logText = "pushNewMessage" + LOG_TAB + r.toStr() + LOG_TAB + LOG_ERROR_OVERFLOW;
                msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
            }
            else {
                m_logText = "pushNewMessage" + LOG_TAB + r.toStr();                
                msgLog(name(), LOG_TXRX, LOG_IN, m_logText, DEBUG_LOG | EXTERNAL_LOG);
            
                m_eventCore.notify(0, SC_NS);
            }
        }
    }


    chord_message* low_latency_chord::firstMessByPriority() {                
        //m_buffer consists of array of sub-buffers as named "message_buffer".
        //message_buffers into m_buffer have already sorted by priority (service) order.
        //message_buffer in 0-position has the highest priority to service a message.
        //method "firstMessPointerByImmediate" will return mess with accordance immediate-priority
        
        bool buffersEmpty = false;
        while (buffersEmpty == false) {
            chord_message* p = nullptr;
            m_indexLastBufferCall = BUFFER_NOT_CHOOSEN;
            for (uint i = 0; i < m_buffer.size(); ++i) {
                p = m_buffer[i].firstMessPointerByImmediate();
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
                        m_buffer[i].clearMessCounter(); //Clear messages counter is necessary to repeat the first cycle and retry find not handled message (i.e. p != nullptr)
                    }
                }
            }
        }
        
        return nullptr;
    }


    void low_latency_chord::eraseFirstMess() {
        if ((m_indexLastBufferCall != BUFFER_NOT_CHOOSEN) && (m_indexLastBufferCall < m_buffer.size())) {
            m_buffer[m_indexLastBufferCall].eraseFirstMess();
            m_indexLastBufferCall = BUFFER_NOT_CHOOSEN;
        }
    }


    void low_latency_chord::core() {
        string currTime = sc_time_stamp().to_string();
        chord_message* p = firstMessByPriority();
        if (p == nullptr)
            return;
                    
        m_logText = "c o r e" + LOG_TAB + p->toStr();
        msgLog(name(), LOG_TXRX, LOG_INFO, m_logText, DEBUG_LOG | INTERNAL_LOG);

        switch (m_state)
        {
        case STATE_OFF:
            if (p->type == CHORD_HARD_RESET) {
                eraseFirstMess();
                hardReset();
                goStateLoad();
            }
            else
                eraseFirstMess();
            break;

        case STATE_INIT:
            goStateInit(p);
            eraseFirstMess();
            break;
            
        case STATE_JOIN:
            goStateJoin(p);
            eraseFirstMess();
            break;                
            
        case STATE_IDLE:
            switch (p->type) {
            case CHORD_HARD_RESET:  eraseFirstMess(); hardReset();  goStateLoad(); break;
            case CHORD_SOFT_RESET:  eraseFirstMess(); softReset();  goStateInit(); break;
            case CHORD_FLUSH:       eraseFirstMess(); flush();      goStateIdle(); break;

            case CHORD_TX_FIND_SUCCESSOR:            eraseFirstMess();  break;

            case CHORD_RX_JOIN:                      findSuccessor(p); eraseFirstMess(); break;
                        
            case CHORD_RX_REPLY_FIND_SUCCESSOR:
            case CHORD_RX_ACK:                    eraseFirstMess(); break;

            default:                eraseFirstMess(); break;
            } break;
            
            
            
            
        case STATE_INDATA: eraseFirstMess(); break;
            
        case STATE_SERVICE: eraseFirstMess(); break;
            
        case STATE_UPDATE: eraseFirstMess(); break;
            
        case STATE_APPREQUEST: eraseFirstMess(); break;

        default:
            eraseFirstMess();
            //ERROR
            m_logText = "core" + LOG_TAB + state2str(m_state);
            msgLog(name(), LOG_TXRX, LOG_INFO, m_logText, DEBUG_LOG | ERROR_LOG);
            return;
        }
            
        /* {
        case CHORD_HARD_RESET:
                
            break;
            
        case CHORD_SOFT_RESET:
            if (m_state != STATE_OFF) {
                eraseFirstMess();
                soft_reset();
                init();
            } break;
        case CHORD_FLUSH:
            if (m_state != STATE_OFF) {
                eraseFirstMess();
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
                eraseFirstMess();
                flush();
                init();
            } break;


        case CHORD_RX_JOIN:
        case CHORD_RX_NOTIFY:
        case CHORD_RX_ACK:
        case CHORD_RX_REPLY_FIND_SUCCESSOR:
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
            m_logText = "core" + LOG_TAB + p->toStr();
            msgLog(name(), LOG_TXRX, LOG_INFO, m_logText, ALL_LOG);
            break;
        }*/
            
                       
        m_eventCore.notify(0, SC_NS);
    }

    
    void low_latency_chord::goStateLoad() {
        m_state = STATE_LOAD;
        //TODO Needs loading new config parameters
        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state), DEBUG_LOG | INTERNAL_LOG);
        
        goStateInit();
    }


    void low_latency_chord::goStateInit(const chord_message* mess) {
        m_state = STATE_INIT;
        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state), DEBUG_LOG | INTERNAL_LOG);

        if (doResetFlushIfMess(mess) == true)
            return;

        //Initializing of fingers, successor, precessor, latency by default values
        m_currSeed = 0;
        m_latency.resize(m_confParams.fingersSize * 2, sc_time(0.1, SC_SEC));
        if (m_confParams.seed.size() == 0) {
            m_logText = "init" + LOG_TAB + string("NO SEED");
            msgLog(name(), LOG_TXRX, LOG_INFO, m_logText, ALL_LOG);
            
            m_fingersNeg.clear();
            m_fingersPos.clear();
            m_precessor.clear();
            m_successor.clear();
            m_isAcked.clear();
            goStateIdle();
        }
        else {
            m_fingersNeg.resize(m_confParams.fingersSize, m_confParams.seed.front());
            m_fingersPos.resize(m_confParams.fingersSize, m_confParams.seed.front());
            m_precessor = m_confParams.seed.front();
            m_successor = m_confParams.seed.front();            
            m_isAcked.clear();
            goStateJoin();
        }
    }

    bool low_latency_chord::isMessValid(const chord_message* mess) {        
        if (mess->destNodeID == m_nodeAddr.id)
            return true;
        return false;
    }


    void low_latency_chord::goStateJoin(const chord_message* mess) {
        m_state = STATE_JOIN;
        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state), DEBUG_LOG | INTERNAL_LOG);

        if (doResetFlushIfMess(mess) == true)
            return;

        if (mess == nullptr) {            
            chord_message newMess;
            if (m_isSuccessorSet == false) {
                newMess.destination.push_back(m_confParams.seed.front());
                newMess.destNodeID     = node_address(newMess.destination[0]).id;
                newMess.source         = m_nodeAddr;
                newMess.mediator       = m_nodeAddr;
                newMess.type           = CHORD_TX_JOIN;
                newMess.appearanceTime = sc_time_stamp();
            
                if (m_counterJoin <= m_confParams.CtxJoin) {
                    m_logText = "goStateJoin" + LOG_TAB + newMess.toStr();
                    msgLog(name(), LOG_TXRX, LOG_INFO, m_logText, DEBUG_LOG | INTERNAL_LOG);

                    pushNewMessage(newMess);

                    chord_message timer;
                    timer.type = CHORD_TIMER_REPLY_FIND_SUCC_JOIN;
                    timer.retryMess = newMess.clone();
                    pushNewMessage(timer);
                    m_eventCore.notify(0, SC_NS);
                }
            }
        }
        else {            
            if ((mess->type == CHORD_TX_JOIN) || (mess->type == CHORD_TX_FIND_SUCCESSOR)) {
                sendMessage(*mess);
            }            
            else if (mess->type == CHORD_RX_REPLY_FIND_SUCCESSOR) {
                if (isMessValid(mess) == true) {
                    ;
                }
            }
            else
            ;
        }

        
    }

    bool low_latency_chord::doResetFlushIfMess(const chord_message* mess) {
        if (mess == nullptr)
            return false;

        if (mess->type == CHORD_HARD_RESET) {
            hardReset();
            goStateLoad();
            return true;
        }
        else if (mess->type == CHORD_SOFT_RESET) {
            softReset();
            goStateInit();
            return true;
        }
        else if (mess->type == CHORD_FLUSH) {
            flush();
            goStateIdle();
            return true;
        }
        else return false;
    }


    void low_latency_chord::goStateIdle(const chord_message* mess) {
        m_state = STATE_IDLE;
        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state), DEBUG_LOG | INTERNAL_LOG);
    }


    void low_latency_chord::goStateIndata(const chord_message* mess)  {
        m_state = STATE_INDATA;
        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state), DEBUG_LOG | INTERNAL_LOG);
    }


    void low_latency_chord::goStateService(const chord_message* mess) {
        m_state = STATE_SERVICE;
        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state), DEBUG_LOG | INTERNAL_LOG);
    }


    void low_latency_chord::goStateUpdate(const chord_message* mess)  {
        m_state = STATE_UPDATE;
        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state), DEBUG_LOG | INTERNAL_LOG);
    }

    void low_latency_chord::goStateApprequest(const chord_message* mess) {    }


    //void low_latency_chord::finiteStateMachine(chord_message* p) {
 
    //}


    int low_latency_chord::chordMessType2buffIndex(const uint type) {
        static map<uint, uint> mapBuffIndex;
        uint buffType = 0;
        switch (type) {            
        case CHORD_HARD_RESET:
        case CHORD_SOFT_RESET:
        case CHORD_FLUSH:        buffType = BUFF_CONFIG; break;

        case CHORD_TIMER_ACK:                   
        case CHORD_TIMER_REPLY_FIND_SUCC:       
        case CHORD_TIMER_REPLY_FIND_SUCC_JOIN:  
        case CHORD_TIMER_UPDATE: buffType = BUFF_TIMER;   break;

        case CHORD_SINGLE:
        case CHORD_MULTICAST:
        case CHORD_BROADCAST:    buffType = BUFF_APPTXDATA; break;

        case CHORD_RX_JOIN:
        case CHORD_RX_NOTIFY:
        case CHORD_RX_ACK:
        case CHORD_RX_REPLY_FIND_SUCCESSOR:
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
            msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, "chordMessType2buffIndex", DEBUG_LOG | ERROR_LOG | EXTERNAL_LOG);
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
        
        //Messages, timers are resetted
        flush();

        //Fingers, latency, precessor, successor are resetted
        softReset();
    }


    void low_latency_chord::softReset() {
        m_eventCore.cancel();
        //Messages, timers are stored
        //Fingers, latency, precessor, successor are resetted
        m_fingersPos.clear();
        m_fingersNeg.clear();
        m_latency.clear();
        m_isAcked.clear();
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

        //Messages, timers are resetted
        for (uint i = 0; i < m_buffer.size(); ++i)
            m_buffer[i].clearMessages();

        //Fingers, latency, precessor, successor are stored
    }


    void low_latency_chord::sendMessage(const chord_message& mess) {    
        m_logText = state2str(m_state) + LOG_TAB + string("sendMessage") + LOG_TAB + const_cast<chord_message&>(mess).toStr();
        msgLog(name(), LOG_TX, LOG_OUT, m_logText, DEBUG_LOG | INTERNAL_LOG);

        trp_port->send_mess(mess);
    }


    void low_latency_chord::findSuccessor(const chord_message* mess) {
        if (mess->destNodeID == m_nodeAddr.id) {

        }

        if (mess->destNodeID == m_precessor.id) {

        }

        if (mess->destNodeID == m_successor.id) {

        }

        for (int i = 0; i < m_fingersPos.size(); ++i)
            ;
    }


    string& low_latency_chord::state2str(const finite_state& state) {
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
}