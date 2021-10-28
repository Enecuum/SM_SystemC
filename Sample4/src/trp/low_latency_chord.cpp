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
        m_nodeAddr.clear();                        //�����, ������������ ��� ������������� ���� �� Transport+ ������ �� ID, �����������, ��� SHA-1
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
        m_buffer.resize(m_howManyBuffers, message_buffer<chord_message>());
        m_buffer[BUFF_CONFIG].set(BUFF_CONFIG, MAX_DEEP_BUFF_CONFIG, MAX_SIZE_BUFF_CONFIG, true, priority++);
        m_buffer[BUFF_TIMER].set(BUFF_TIMER, MAX_DEEP_BUFF_TIMER, MAX_SIZE_BUFF_TIMER, true, priority++);

        m_buffer[BUFF_TX_JOIN].set(BUFF_TX_JOIN, MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_NOTIFY].set(BUFF_TX_NOTIFY, MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_ACK].set(BUFF_TX_ACK, MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_REPLY_FIND_SUCCESSOR].set(BUFF_TX_REPLY_FIND_SUCCESSOR, MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_FIND_SUCCESSOR].set(BUFF_TX_FIND_SUCCESSOR, MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_FWD_BROADCAST].set(BUFF_TX_FWD_BROADCAST, MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_FWD_MULTICAST].set(BUFF_TX_FWD_MULTICAST, MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_FWD_SINGLE].set(BUFF_TX_FWD_SINGLE, MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_BROADCAST].set(BUFF_TX_BROADCAST, MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_MULTICAST].set(BUFF_TX_MULTICAST, MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_TX_SINGLE].set(BUFF_TX_SINGLE, MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_APPTXDATA].set(BUFF_APPTXDATA, MAX_DEEP_BUFF_APPTXDATA, MAX_SIZE_BUFF_APPTXDATA, false, priority++);
        m_buffer[BUFF_RX_MESS].set(BUFF_RX_MESS, MAX_DEEP_BUFF_RX_MESS, MAX_SIZE_BUFF_RX_MESS, false, priority++);

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
        m_seedAddrs.resize(seed.size(), node_address());
        m_confParams.seed.resize(seed.size(), network_address());
        for (uint i = 0; i < seed.size(); ++i) {
            m_confParams.seed[i] = seed[i];
            m_seedAddrs[i].set(seed[i]);
        }
    }


    void low_latency_chord::setConfParameters(const chord_conf_parameters& params) {
        m_confParams = params;
        m_nodeAddr.set(m_confParams.netwAddr);
    }


    void low_latency_chord::pushNewMessage(const chord_message& mess) {
        chord_message& r = const_cast<chord_message&>(mess);
        r.appearanceTime = sc_time_stamp();

        m_logText = "pushNewMessage" + LOG_TAB + r.toStr();
        msgLog(name(), LOG_TXRX, LOG_IN, m_logText, DEBUG_LOG | EXTERNAL_LOG);

        //Find buffer index
        int i = chordMessType2buffIndex(r.type);
        if ((i == ERROR) || (i >= MAX_BUFF_TYPE)) {
            //ERROR
            m_logText = "pushNewMessage" + LOG_TAB + LOG_ERROR_NOT_RECOGNIZED;
            msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
            return;
        }

        //Drop rx message, if addr is zero
        if ((r.destNodeID.isNone()) && (r.type > MIN_CHORD_RX_TYPE) && (r.type < MAX_CHORD_RX_TYPE)) {
            m_logText = "pushNewMessage" + LOG_TAB + LOG_ERROR_NO_ADDR;
            msgLog(name(), LOG_TXRX, LOG_INFO, m_logText, DEBUG_LOG | ERROR_LOG);
            return;
        }           

        //Push message into buffer
        if (m_buffer[i].push(r) == false) {
            //OVERFLOW BUFFER
            m_logText = "pushNewMessage" + LOG_TAB + LOG_ERROR_OVERFLOW;
            msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
            return;
        }
            
        m_eventCore.notify(0, SC_NS);
    }


    chord_message* low_latency_chord::firstMessByPriority() {                
        //m_buffer consists of array of sub-buffers as named "message_buffer".
        //message_buffers have already sorted by priority (service) order.
        //message_buffer in 0-position in m_buffer has the highest priority to service a message.
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
        if ((m_nodeAddr.isNone()) && (m_state != STATE_LOAD)) {
            //ERROR
            m_logText = "core" + LOG_TAB + LOG_ERROR_NO_ADDR;
            msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
            m_eventCore.cancel();
            return;
        }


        string currTime = sc_time_stamp().to_string();
        chord_message* p = firstMessByPriority();
        if (p == nullptr)
            return;                                  //Nothing to do
                    
        m_logText = "core" + LOG_TAB + p->toStr();
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
            goStateIdle(p);
            eraseFirstMess();
            break;
            
        case STATE_INDATA:  goStateIndata(p); eraseFirstMess(); break;
            
        case STATE_SERVICE: goStateService(p); eraseFirstMess(); break;
            
        case STATE_UPDATE:  eraseFirstMess(); break;
            
        case STATE_APPREQUEST: eraseFirstMess(); break;

        default:
            eraseFirstMess();
            //ERROR
            m_logText = "core" + LOG_TAB + LOG_ERROR_NOT_RECOGNIZED + state2str(m_state);
            msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
            return;
        }
            
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
            
            //m_ccwFingers.clear();
            //m_cwFingers.clear();
            //m_precessor.clear();
            //m_successor.clear();
            m_ccwFingers.resize(m_confParams.fingersSize, m_nodeAddr);
            m_cwFingers.resize(m_confParams.fingersSize, m_nodeAddr);
            m_precessor = m_nodeAddr;
            m_successor = m_nodeAddr;
            m_isAcked.clear();
            goStateIdle();
        }
        else {
            m_ccwFingers.resize(m_confParams.fingersSize, m_confParams.seed.front());
            m_cwFingers.resize(m_confParams.fingersSize, m_confParams.seed.front());
            m_precessor = m_confParams.seed.front();
            m_successor = m_confParams.seed.front();            
            m_isAcked.clear();
            goStateJoin();
        }
    }

    bool low_latency_chord::isMessValid(const chord_message* mess) {        
        if (mess->destNodeID.id == m_nodeAddr.id)
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
                newMess.destNodeID     = node_address(newMess.destination[0]);
                newMess.initiatorID    = m_nodeAddr;
                newMess.srcNodeID      = m_nodeAddr;
                newMess.type           = CHORD_TX_JOIN;
                newMess.appearanceTime = sc_time_stamp();
                newMess.flags.bitMessType = CHORD_BYTE_JOIN;
            
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
        
            //DEBUG
            if (mess->type == CHORD_RX_JOIN)
                (const_cast<chord_message*>(mess))->type = CHORD_RX_FIND_SUCCESSOR;
            //DEBUG
            

            if ((mess->type == CHORD_TX_JOIN) || (mess->type == CHORD_TX_FIND_SUCCESSOR)) {
                sendMessage(*mess);
            }            
            else if (mess->type == CHORD_RX_REPLY_FIND_SUCCESSOR) {
                if ((mess->destNodeID == m_nodeAddr) && (mess->initiatorID == m_nodeAddr))  {
                    cout << "HELLO";
                }
            }
            else {
                ;
            }
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

        if (doResetFlushIfMess(mess) == true)
            return;

        if (mess == nullptr) {
            //Timer update was expired
            
        }
        else {
            //Got message from network: TCP/UDP over IP network
            if (mess->type == CHORD_RX_JOIN) {
                goStateService(mess);
            }
            else {
                 ;
            }
        }
    }


    void low_latency_chord::goStateIndata(const chord_message* mess)  {
        m_state = STATE_INDATA;
        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state), DEBUG_LOG | INTERNAL_LOG);
    }


    void low_latency_chord::goStateService(const chord_message* mess) {
        m_state = STATE_SERVICE;
        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state), DEBUG_LOG | INTERNAL_LOG);

        if (doResetFlushIfMess(mess) == true)
            return;

        if (mess == nullptr) {
            ; //Resume actions of Service State, fingers are good now
        }
        else {
            //Got message from network: TCP/UDP over IP network
            if (mess->type == CHORD_RX_JOIN) {
                node_address forward;
                chord_action a = findSuccessor(mess, forward);

            }
            else
                ;
        }
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
            msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, string("chordMessType2buffIndex ") + LOG_ERROR_NOT_RECOGNIZED, DEBUG_LOG | ERROR_LOG | EXTERNAL_LOG);
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
        m_cwFingers.clear();
        m_ccwFingers.clear();
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
        if (mess.destNodeID.id.iszero()) {
            //ERROR
            m_logText = "sendMessage" + LOG_TAB + LOG_ERROR_NO_ADDR;
            msgLog(name(), LOG_TX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
        }

        m_logText = state2str(m_state) + LOG_TAB + string("sendMessage") + LOG_TAB + const_cast<chord_message&>(mess).toStr();
        msgLog(name(), LOG_TX, LOG_OUT, m_logText, DEBUG_LOG | INTERNAL_LOG);

        trp_port->send_mess(mess);
    }


    bool low_latency_chord::isClockWiseDirection(const uint160& id) {
        
        uint160 first, last;
        if (m_ccwFingers.size() == 0)
            return true;
        if (m_cwFingers.size() == 0)
            return false;
        
        if (id >= m_nodeAddr.id) {
            if (m_cwFingers.at(0).id <= id) //Is situation: 7(id) > 5 AND 0 < 7(id)    values on line: {5, id, 0, ...} 
                return true;

            for (uint i = 0; i < m_cwFingers.size(); ++i) {
                if (m_cwFingers[i].id >= id) //Is situation: 6 > 5(id) ?
                    return true;
                else if (i >= 1) {
                    if ((m_cwFingers[i].id < id) && (id > m_cwFingers[i-1].id)) //Is situation: 0 < 5(id) AND 5(id) > 4  values on line: {4, id, 0, ...}
                        return true;
                }         
            }

            if ((m_cwFingers.back().id() < id) && (id < m_ccwFingers.back().id)) //Is situation: 6 < 7(id) AND 7(id) < 8  values on line: {..., 6} id {8, ...} 
                return true;
        }
        return false;
    }

    uint160& low_latency_chord::closestPrecedingNode(const uint160& id) {
        uint160 found;
        
        if (isClockWiseDirection(id)) {
            for (uint i = m_cwFingers.size()-1; i >= 0; ++i) {
                ;
            }   
        }
        return found;
    }


    chord_action low_latency_chord::findSuccessor(const uint160& id, node_address& found) {
        found.clear();

        if ((m_nodeAddr.isNone() == true)) {
            //ERROR
            m_logText = "findSuccessor " + LOG_TAB + LOG_ERROR_NO_ADDR;
            msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
            return DROP_MESSAGE;
        }

        //My node is first node in p2p network
        //if ((m_cwFingers.size() < 1) && (m_ccwFingers.size() < 1)) {
        //    found = m_nodeAddr;
        //    return DO_REPLY;
        //}

        //Comparising with addrs of seed nodes        
        for (uint i = 0; i < m_seedAddrs.size(); ++i) {
            if (m_seedAddrs[i].isNone() == false) {
                if (id == m_seedAddrs[i].id) {
                    found = m_seedAddrs[i];
                    return DO_REPLY;
                }
            }
        }

        //Chord: Comparising with successor, Chord
        if ((m_precessor.isNone()) && (id > m_precessor.id) && (id <= m_nodeAddr.id)) {
            found = m_nodeAddr;
            return DO_REPLY;
        }
        else if ((id > m_nodeAddr.id ) && (id <= m_successor.id)) {
            found = m_successor;     
            return DO_REPLY;
        }
        else {
            found = closestPrecedingNode(id);
            if (!found.isNone())
                return DO_FORWARD;
                return DROP_MESSAGE;
        }
    }


    string& low_latency_chord::state2str(const finite_state& state) {
        static string res;
        switch (state) {
        case STATE_LOAD:       return res = /*"STATE_*/"LOAD";
        case STATE_INIT:       return res = /*"STATE_*/"INIT";
        case STATE_JOIN:       return res = /*"STATE_*/"JOIN";
        case STATE_IDLE:       return res = /*"STATE_*/"IDLE";
        case STATE_INDATA:     return res = /*"STATE_*/"INDATA";
        case STATE_SERVICE:    return res = /*"STATE_*/"SERVICE";
        case STATE_UPDATE:     return res = /*"STATE_*/"UPDATE";
        case STATE_APPREQUEST: return res = /*"STATE_*/"APPREQUEST";
        case STATE_OFF:        return res = /*"STATE_*/"OFF";
        default:               return res = /*"STATE_*/"UNKNOWN";
        }
    }
}