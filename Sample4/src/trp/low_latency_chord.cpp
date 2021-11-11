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
        m_repeatCounter = 0;
        m_state = STATE_OFF;
        m_nodeAddr.clear();                        
        m_confParams.setDefaultTimersCountersFingersSize();
        hardReset();

        //Create buffers
        // 
        //Set priority
        //priority "1" is the highest priority
        //priority "15"  is the lowest priority
        //flag "immediate" allows to handle all messages into buffer without consider max deep limit
        uint priority = 0;
        m_indexLastBufferCall = 0;
        m_howManyBuffers = 14;
        m_buffer.resize(m_howManyBuffers, message_buffer<chord_message>());
        uint buffI = 0;
        m_buffer[buffI++].set(BUFF_CONFIG, MAX_DEEP_BUFF_CONFIG, MAX_SIZE_BUFF_CONFIG, true, priority++);
        m_buffer[buffI++].set(BUFF_RX_MESS, MAX_DEEP_BUFF_RX_MESS, MAX_SIZE_BUFF_RX_MESS, false, priority++);
        m_buffer[buffI++].set(BUFF_TX_JOIN, MAX_DEEP_BUFF_TX_MESS, MAX_SIZE_BUFF_TX_MESS, false, priority++);
        m_buffer[buffI++].set(BUFF_TX_NOTIFY, MAX_DEEP_BUFF_TX_MESS, MAX_SIZE_BUFF_TX_MESS, false, priority++);
        m_buffer[buffI++].set(BUFF_TX_ACK, MAX_DEEP_BUFF_TX_MESS, MAX_SIZE_BUFF_TX_MESS, false, priority++);
        m_buffer[buffI++].set(BUFF_TX_SUCCESSOR, MAX_DEEP_BUFF_TX_MESS, MAX_SIZE_BUFF_TX_MESS, false, priority++);
        m_buffer[buffI++].set(BUFF_TX_FIND_SUCCESSOR, MAX_DEEP_BUFF_TX_MESS, MAX_SIZE_BUFF_TX_MESS, false, priority++);
        m_buffer[buffI++].set(BUFF_TX_PREDECESSOR, MAX_DEEP_BUFF_TX_MESS, MAX_SIZE_BUFF_TX_MESS, false, priority++);
        m_buffer[buffI++].set(BUFF_TX_FIND_PREDECESSOR, MAX_DEEP_BUFF_TX_MESS, MAX_SIZE_BUFF_TX_MESS, false, priority++);
        //m_buffer[buffI++].set(BUFF_TX_FWD_BROADCAST, MAX_DEEP_BUFF_TX_MESS, MAX_SIZE_BUFF_TX_MESS, false, priority++);
        //m_buffer[buffI++].set(BUFF_TX_FWD_MULTICAST, MAX_DEEP_BUFF_TX_MESS, MAX_SIZE_BUFF_TX_MESS, false, priority++);
        //m_buffer[buffI++].set(BUFF_TX_FWD_SINGLE, MAX_DEEP_BUFF_TX_MESS, MAX_SIZE_BUFF_TX_MESS, false, priority++);
        m_buffer[buffI++].set(BUFF_TX_BROADCAST, MAX_DEEP_BUFF_TX_MESS, MAX_SIZE_BUFF_TX_MESS, false, priority++);
        m_buffer[buffI++].set(BUFF_TX_MULTICAST, MAX_DEEP_BUFF_TX_MESS, MAX_SIZE_BUFF_TX_MESS, false, priority++);
        m_buffer[buffI++].set(BUFF_TX_SINGLE, MAX_DEEP_BUFF_TX_MESS, MAX_SIZE_BUFF_TX_MESS, false, priority++);
        m_buffer[buffI++].set(BUFF_APPTXDATA, MAX_DEEP_BUFF_TX_MESS, MAX_SIZE_BUFF_TX_MESS, false, priority++);        
        m_buffer[buffI++].set(BUFF_TIMER, MAX_DEEP_BUFF_TIMER, MAX_SIZE_BUFF_TIMER, true, priority++);

        //Sort buffers by priority and immediate fields of message_buffer class, comparison's operator for sorting is defined in message_buffer class
        std::sort(m_buffer.begin(), m_buffer.end());
        
        //Save index of timer buffer
        m_timerBufferIndex = chordMessType2buffIndex(CHORD_TIMER_RX_ACK);
        if (m_timerBufferIndex == ERROR) {
            //ERROR
            msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, "preinit" + LOG_TAB + LOG_ERROR_NOT_RECOGNIZED, DEBUG_LOG | ERROR_LOG);
        }

        //Print info about buffers (priority...)
        static bool printOnce = false;
        if (printOnce == false) {
            printOnce = true;
            for (int i = 0; i < m_buffer.size(); ++i)
                cout << m_buffer[i].toStr() << endl;
        }        
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
        r.creatingTime = sc_time_stamp();

        m_logText = state2str(m_state) + LOG_TAB + string("pushNewMessage") + LOG_TAB + r.toStr();
        msgLog(name(), LOG_TXRX, LOG_IN, m_logText, DEBUG_LOG | EXTERNAL_LOG);

        //Find buffer index
        int i = chordMessType2buffIndex(r.type);
        if ((i == ERROR) || (i >= MAX_BUFF_TYPE)) {
            //ERROR
            m_logText = state2str(m_state) + LOG_TAB + string("pushNewMessage") + LOG_TAB + LOG_ERROR_NOT_RECOGNIZED;
            msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
            return;
        }

        //Drop rx message, if addr is zero
        if ((r.destNodeIDwithSocket.isNone()) && (r.type > MIN_CHORD_RX_TYPE) && (r.type < MAX_CHORD_RX_TYPE)) {
            m_logText = state2str(m_state) + LOG_TAB + string("pushNewMessage") + LOG_TAB + LOG_ERROR_NO_ADDR;
            msgLog(name(), LOG_TXRX, LOG_INFO, m_logText, DEBUG_LOG | ERROR_LOG);
            return;
        }           

        //Push message into buffer
        if (m_buffer[i].push(r) == false) {
            //OVERFLOW BUFFER
            m_logText = state2str(m_state) + LOG_TAB + string("pushNewMessage") + LOG_TAB + LOG_ERROR_OVERFLOW;
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
                if (m_buffer[i].buffType() != BUFF_TIMER) {
                    p = m_buffer[i].firstMessPointerByImmediate();
                    if (p != nullptr) {                                          
                        m_indexLastBufferCall = i;
                        return p;
                    }                    
                }
            }

            buffersEmpty = true;            
            if (m_indexLastBufferCall == BUFFER_NOT_CHOOSEN) {
                for (uint i = 0; i < m_buffer.size(); ++i) {
                    if (m_buffer[i].size() > 0) {
                        if (m_buffer[i].buffType() != BUFF_TIMER) {
                            buffersEmpty = false;
                            m_buffer[i].clearMessCounter(); //Clear messages counter is necessary to repeat the first cycle and retry find not handled message 
                        }
                    }
                }
            }
        }

        //Check timers after all other buffers with received messages or planned messages for transmitting
        chord_message* p = nullptr;
        m_indexLastBufferCall = BUFFER_NOT_CHOOSEN;
        sc_time minTimerDelay = sc_time(MAX_UINT, SC_SEC);
        bool hasTimer = false;
        for (uint i = 0; i < m_buffer[m_timerBufferIndex].size(); ++i) {
            sc_time timeout;
            p = m_buffer[m_timerBufferIndex].mess(i);
            if (p != nullptr) {
                if (p->type == CHORD_TIMER_RX_ACK)
                    timeout = m_confParams.TrxAck;
                else if (p->type == CHORD_TIMER_RX_SUCCESSOR_ON_JOIN)
                    timeout = m_confParams.TrxSuccOnJoin;
                else if (p->type == CHORD_TIMER_RX_SUCCESSOR)
                    timeout = m_confParams.TrxSucc;
                else if (p->type == CHORD_TIMER_UPDATE)
                    timeout = m_confParams.Tupdate;
            
                sc_time whenTriggered = (p->creatingTime + timeout);
                string whenStr  = whenTriggered.to_string();
                string currTime = sc_time_stamp().to_string();
                if (sc_time_stamp() >= whenTriggered) {
                    m_indexLastBufferCall = m_timerBufferIndex;
                    return p;
                }
                else {
                    sc_time delay = whenTriggered - sc_time_stamp();
                    if (minTimerDelay > delay) {
                        minTimerDelay = delay;
                        hasTimer = true;
                    }
                }
            }
        }

        if (hasTimer)
            m_eventCore.notify(minTimerDelay);
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

        string strName = name();
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
            break;                
            
        case STATE_IDLE:
            goStateIdle(p);
            //eraseFirstMess();
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
        m_fingerMask.resize(m_confParams.fingersSize, 1);
        for (uint i = 1; i < m_fingerMask.size(); ++i) 
            m_fingerMask[i] = m_fingerMask[i-1] << 1;
        
        
        m_latency.clear();
        if (m_confParams.seed.size() == 0) {
            m_logText = "init" + LOG_TAB + string("NO SEED");
            msgLog(name(), LOG_TXRX, LOG_INFO, m_logText, ALL_LOG);
            
            //m_ccwFingers.clear();
            //m_cwFingers.clear();
            //m_predecessor.clear();
            //m_successor.clear();
            m_ccwFingers.resize(m_confParams.fingersSize, node_address_latency(m_nodeAddr));
            m_cwFingers.resize(m_confParams.fingersSize, node_address_latency(m_nodeAddr));
            m_predecessor.clear();
            m_successor = m_nodeAddr;
            m_isAcked.clear();

            if (m_confParams.fillFingersMinQty >= m_cwFingers.size()) {
                m_confParams.fillFingersMinQty = (uint) m_cwFingers.size();
                //ERROR
                msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, state2str(m_state) + LOG_TAB + "confParams.fillFingersMinQty >= m_cwFingers.size()", DEBUG_LOG | ERROR_LOG);                
            }
            goStateIdle();
        }
        else {
            m_ccwFingers.resize(m_confParams.fingersSize, node_address_latency(m_confParams.seed.front()));
            m_cwFingers.resize(m_confParams.fingersSize, node_address_latency(m_confParams.seed.front()));
            m_predecessor.clear();
            m_successor = m_confParams.seed.front();            
            m_isAcked.clear();

            if (m_confParams.fillFingersMinQty >= m_cwFingers.size()) {
                //ERROR
                msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, state2str(m_state) + LOG_TAB + "confParams.fillFingersMinQty >= m_cwFingers.size()", DEBUG_LOG | ERROR_LOG);
                m_confParams.fillFingersMinQty = (uint) m_cwFingers.size();
            }
            goStateJoin();
        }
    }

        
    bool low_latency_chord::isAddrValid(const chord_message& mess) {                
        if (mess.destNodeIDwithSocket.id == m_nodeAddr.id)
            return true;
        return false;
    }


    bool low_latency_chord::isMessageIDvalid(const chord_message& mess, buffer_container::iterator& it) {
        vector<uint> retryMessTypes;
        bool exist;
        if (mess.type == CHORD_RX_ACK) {
            retryMessTypes = {CHORD_TX_JOIN, CHORD_TX_NOTIFY, CHORD_TX_FIND_SUCCESSOR, CHORD_TX_BROADCAST, CHORD_TX_MULTICAST, CHORD_TX_SINGLE, CHORD_TX_FWD_BROADCAST, CHORD_TX_FWD_MULTICAST, CHORD_TX_FWD_SINGLE};
            it = findMessageOnTimersWithRetryParams(exist, CHORD_TIMER_RX_ACK, retryMessTypes, mess.messageID);
            return exist;
        }
        else if (mess.type == CHORD_RX_SUCCESSOR) {
            retryMessTypes = {CHORD_TX_JOIN, CHORD_TX_FIND_SUCCESSOR};
            uint timer = CHORD_TIMER_RX_SUCCESSOR;
            if (m_isSuccessorSet == false)
                timer = CHORD_TIMER_RX_SUCCESSOR_ON_JOIN;
            it = findMessageOnTimersWithRetryParams(exist, timer, retryMessTypes, mess.messageID);
            return exist;
        }       
        
        return true;
    }


    void low_latency_chord::goStateJoin(const chord_message* mess) {
        m_state = STATE_JOIN;
        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state), DEBUG_LOG | INTERNAL_LOG);

        if (doResetFlushIfMess(mess) == true)
            return;
        
        switch (eventType(mess)) {
        case CALLED_BY_ANOTHER_STATE: {
            //First call this state, message wasn't received, timer wasn't expired
           
            //if (m_isSuccessorSet == false) {

            m_currSeed = (m_currSeed + 1) % m_confParams.seed.size();

            m_currCwFinger = 0;            
            chord_message newMess;
            newMess = createJoinMessage(m_confParams.seed.at(m_currSeed));
            pushNewMessage(newMess);

            if (m_confParams.TrxSuccOnJoin != NO_TIMEOUT)
                pushNewTimer(CHORD_TIMER_RX_SUCCESSOR_ON_JOIN, 0, newMess.clone());
            if (m_confParams.needsACK == NEEDS_ACK)
                pushNewTimer(CHORD_TIMER_RX_ACK, 0, newMess.clone());

            eraseFirstMess();
            m_eventCore.notify(0, SC_NS);
            //}
        } break;

        case TX_MESS_SHOULD_SEND: {             //TX message should be sent            
            if ((mess->type == CHORD_TX_JOIN) || (mess->type == CHORD_TX_FIND_SUCCESSOR)) { //if (mess->initiatorNodeIDwithSocket.id == m_nodeAddr.id)d                
                sendMessage(*mess);  
            }
            eraseFirstMess();
        } break;

        case RX_MESS_RECEIVED: {               //RX Message was received 
            if (mess->type == CHORD_RX_SUCCESSOR) {                
                buffer_container::iterator timerIt;
                if (checkMessage(*mess, timerIt, m_errCode) == ERROR)
                    eraseFirstMess();
                else {
                    if (m_isSuccessorSet == false) {    
                        m_successor = mess->srcNodeIDwithSocket;
                        m_currCwFinger = 0;
                        m_cwFingers[m_currCwFinger] = m_successor;
                        m_currCwFinger++;
                        m_isSuccessorSet = true;                        
                        
                        //Remove timers
                        uint timerTypeTmp = (timerIt->type == CHORD_TIMER_RX_SUCCESSOR_ON_JOIN ? CHORD_TIMER_RX_ACK : CHORD_TIMER_RX_SUCCESSOR_ON_JOIN);
                        removeTimer(timerTypeTmp, CHORD_TX_JOIN, mess->messageID);                                                
                        removeTimer(timerIt);
                        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state)+LOG_TAB+string("remove timer TrxAck and TrxSuccOnJoin"), DEBUG_LOG | INTERNAL_LOG);
                        eraseFirstMess();

                        //Create find_successor
                        chord_message newMess;
                        newMess = createFindSuccessorMessage(m_cwFingers[m_currCwFinger - 1], m_nodeAddr, m_nodeAddr.id + m_fingerMask[m_currCwFinger]);
                        pushNewMessage(newMess);

                        if (m_confParams.TrxSucc != NO_TIMEOUT)
                            pushNewTimer(CHORD_TIMER_RX_SUCCESSOR, 0, newMess.clone());
                        if (m_confParams.needsACK == NEEDS_ACK)
                            pushNewTimer(CHORD_TIMER_RX_ACK, 0, newMess.clone());
                    }
                    else {
                        m_cwFingers[m_currCwFinger] = mess->srcNodeIDwithSocket;
                        m_currCwFinger++;  
                        
                        //Remove timers
                        uint timerTmp = (timerIt->type == CHORD_TIMER_RX_SUCCESSOR_ON_JOIN ? CHORD_TIMER_RX_ACK : CHORD_TIMER_RX_SUCCESSOR_ON_JOIN);
                        removeTimer(timerTmp, CHORD_TX_JOIN, mess->messageID);
                        removeTimer(timerIt);
                        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("remove timer TrxAck and TrxSuccOnJoin"), DEBUG_LOG | INTERNAL_LOG);

                        //Send next find_successor
                        if (m_currCwFinger < m_confParams.fillFingersMinQty) {                            
                            chord_message newMess;
                            newMess = createFindSuccessorMessage(m_cwFingers[m_currCwFinger-1], m_nodeAddr, m_nodeAddr.id+m_fingerMask[m_currCwFinger]);
                            pushNewMessage(newMess);

                            if (m_confParams.TrxSucc != NO_TIMEOUT)
                                pushNewTimer(CHORD_TIMER_RX_SUCCESSOR, 0, newMess.clone());
                            if (m_confParams.needsACK == NEEDS_ACK)
                                pushNewTimer(CHORD_TIMER_RX_ACK, 0, newMess.clone());
                            eraseFirstMess();
                        }
                        else {
                            eraseFirstMess();
                            goStateIdle();
                        }
                    }
                }
            }
            else if (mess->type == CHORD_RX_ACK) {
                buffer_container::iterator timerIt;
                if (checkMessage(*mess, timerIt, m_errCode) == ERROR)
                    eraseFirstMess();
                else {
                    //Remove timer
                    removeTimer(timerIt);
                    msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("remove timer ACK"), DEBUG_LOG | INTERNAL_LOG);
                    eraseFirstMess();
                }
            }            
        } break;

        case TIMER_EXPIRED:{                   //Timer was expired, analysing
            if (mess->type == CHORD_TIMER_RX_ACK) {
                if (m_isSuccessorSet == false) {
                    removeTimer(CHORD_TIMER_RX_SUCCESSOR_ON_JOIN, CHORD_TX_JOIN, mess->retryMess->messageID);

                    if (mess->retryCounter < m_confParams.CtxAck) {
                        pushNewMessage(*(mess->retryMess));

                        if (m_confParams.TrxSuccOnJoin != NO_TIMEOUT)
                            pushNewTimer(CHORD_TIMER_RX_SUCCESSOR_ON_JOIN, mess->retryCounter + 1, mess->retryMess->clone());
                        if (m_confParams.needsACK == NEEDS_ACK)
                            pushNewTimer(CHORD_TIMER_RX_ACK, mess->retryCounter + 1, mess->retryMess->clone());
                    }
                    else {
                        m_currSeed = (m_currSeed + 1) % m_confParams.seed.size();
                        chord_message newMess;
                        newMess = createJoinMessage(m_confParams.seed.at(m_currSeed));
                        pushNewMessage(newMess);

                        if (m_confParams.TrxSuccOnJoin != NO_TIMEOUT)
                            pushNewTimer(CHORD_TIMER_RX_SUCCESSOR_ON_JOIN, 0, newMess.clone());
                        if (m_confParams.needsACK == NEEDS_ACK)
                            pushNewTimer(CHORD_TIMER_RX_ACK, 0, newMess.clone());
                    }                   
                }
                else {
                    removeTimer(CHORD_TIMER_RX_SUCCESSOR, CHORD_TX_FIND_SUCCESSOR, mess->retryMess->messageID);
                    if (mess->retryCounter < m_confParams.CtxAck) {
                        pushNewMessage(*(mess->retryMess));

                        if (m_confParams.TrxAck != NO_TIMEOUT)
                            pushNewTimer(CHORD_TIMER_RX_SUCCESSOR, mess->retryCounter+1, mess->retryMess->clone());
                        if (m_confParams.needsACK == NEEDS_ACK)
                            pushNewTimer(CHORD_TIMER_RX_ACK, mess->retryCounter+1, mess->retryMess->clone());                                                
                    }
                }
                m_eventCore.notify(0, SC_NS);
                eraseFirstMess();
            }
            else if (mess->type == CHORD_TIMER_RX_SUCCESSOR_ON_JOIN) {               
                if (m_isSuccessorSet == false) {
                    removeTimer(CHORD_TIMER_RX_ACK, CHORD_TX_JOIN, mess->retryMess->messageID);

                    m_currSeed = (m_currSeed + 1) % m_confParams.seed.size();
                    chord_message newMess;
                    newMess = createJoinMessage(m_confParams.seed.at(m_currSeed));
                    pushNewMessage(newMess);

                    if (m_confParams.TrxSuccOnJoin != NO_TIMEOUT)
                        pushNewTimer(CHORD_TIMER_RX_SUCCESSOR_ON_JOIN, 0, newMess.clone());
                    if (m_confParams.needsACK == NEEDS_ACK)
                        pushNewTimer(CHORD_TIMER_RX_ACK, 0, newMess.clone());
                }               
                m_eventCore.notify(0, SC_NS);
                eraseFirstMess();
            }
            } break;
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
            if ((mess->type > MIN_CHORD_RX_TYPE) && (mess->type < MAX_CHORD_RX_TYPE)) {
                goStateService(mess);
            }
            //else if ((mess->type > MIN_CHORD_TIMER_TYPE) && (mess->type > MAX_CHORD_TIMER_TYPE)) {
            //}
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
        
        eraseFirstMess();
        return;
        
        switch (eventType(mess)) {
        case CALLED_BY_ANOTHER_STATE: {
           
        } break;

        case TX_MESS_SHOULD_SEND: {                        
            if ((mess->type == CHORD_TX_SUCCESSOR) || (mess->type == CHORD_TX_FIND_SUCCESSOR) || (mess->type == CHORD_TX_ACK)) { //if (mess->initiatorNodeIDwithSocket.id == m_nodeAddr.id)d                
                sendMessage(*mess);
            }
            eraseFirstMess();
        } break;

        case RX_MESS_RECEIVED: {           
            if ((mess->type == CHORD_RX_JOIN) || (mess->type == CHORD_RX_FIND_SUCCESSOR)) {
                buffer_container::iterator timerIt;
                if (checkMessage(*mess, timerIt, m_errCode) == ERROR)
                    eraseFirstMess();
                else {
                    chord_message newMess;
                    if (mess->flags.needsACK == NEEDS_ACK) {
                        newMess = createAckMessage(mess->srcNodeIDwithSocket, mess->messageID);
                        pushNewMessage(newMess);
                    }

                    uint160 searchedID = mess->srcNodeIDwithSocket.id+1;
                    node_address fingerAddr;
                    chord_action action = findSuccessor(searchedID, fingerAddr);

                    newMess.clear();
                    if (action == DO_REPLY) {
                        newMess = createSuccessorMessage(mess->srcNodeIDwithSocket, mess->messageID, fingerAddr);
                        pushNewMessage(newMess);
                    }
                    else if (action == DO_FORWARD) {
                        newMess = createFindSuccessorMessage(network_address(fingerAddr.ip, fingerAddr.inSocket), mess->initiatorNodeIDwithSocket, searchedID);
                        pushNewMessage(newMess);

                        if (m_confParams.needsACK == NEEDS_ACK)
                            pushNewTimer(CHORD_TIMER_RX_ACK, 0, newMess.clone());                        
                    }
                    eraseFirstMess();
                }
            }
        } break;

        case TIMER_EXPIRED: {                   //Timer was expired, analysing
            eraseFirstMess();
        } break;
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

        case CHORD_TIMER_RX_ACK:                   
        case CHORD_TIMER_RX_SUCCESSOR:       
        case CHORD_TIMER_RX_SUCCESSOR_ON_JOIN:  
        case CHORD_TIMER_UPDATE: buffType = BUFF_TIMER;   break;

        case CHORD_SINGLE:
        case CHORD_MULTICAST:
        case CHORD_BROADCAST:    buffType = BUFF_APPTXDATA; break;

        case CHORD_RX_JOIN:
        case CHORD_RX_NOTIFY:
        case CHORD_RX_ACK:
        case CHORD_RX_SUCCESSOR:
        case CHORD_RX_FIND_SUCCESSOR:
        case CHORD_RX_BROADCAST:
        case CHORD_RX_MULTICAST:
        case CHORD_RX_SINGLE:    buffType = BUFF_RX_MESS; break;
                                 
        case CHORD_TX_JOIN:      buffType = BUFF_TX_JOIN;   break;
        case CHORD_TX_NOTIFY:    buffType = BUFF_TX_NOTIFY; break;
        case CHORD_TX_ACK:       buffType = BUFF_TX_ACK;    break;
        case CHORD_TX_SUCCESSOR: buffType = BUFF_TX_SUCCESSOR; break;
        case CHORD_TX_FIND_SUCCESSOR: buffType = BUFF_TX_FIND_SUCCESSOR; break;
        case CHORD_TX_PREDECESSOR:    buffType = BUFF_TX_PREDECESSOR; break;
        case CHORD_TX_FIND_PREDECESSOR: buffType = BUFF_TX_FIND_PREDECESSOR; break;
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
        m_repeatCounter = 0;
        
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
        m_predecessor.clear();
        m_currSeed = 0;
        m_isSuccessorSet = false;
        m_isPrecessorSet = false;
        m_currCwFinger = 0;
        m_counterJoin = 0;
        m_messageID = 0;
    }

    
    void low_latency_chord::flush() {
        m_eventCore.cancel();

        //Messages, timers are resetted
        for (uint i = 0; i < m_buffer.size(); ++i)
            m_buffer[i].clearMessages();

        //Fingers, latency, precessor, successor are stored
    }


    void low_latency_chord::sendMessage(const chord_message& mess) {
        if (mess.destNodeIDwithSocket.isNone()) {
            //ERROR
            m_logText = "sendMessage" + LOG_TAB + LOG_ERROR_NO_ADDR;
            msgLog(name(), LOG_TX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
        }


        //LOG
        m_logText = state2str(m_state) + LOG_TAB + string("sendMessage") + LOG_TAB + mess.toStr();
        msgLog(name(), LOG_TX, LOG_OUT, m_logText, DEBUG_LOG | INTERNAL_LOG);

        chord_byte_message raw;
        raw.fields = mess.clone();

        trp_port->send_mess(raw);
    }


    bool low_latency_chord::isClockWiseDirection(const uint160& id) {
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

            if ((m_cwFingers.back().id < id) && (id < m_ccwFingers.back().id)) //Is situation: 6 < 7(id) AND 7(id) < 8  values on line: {..., 6} id {8, ...} 
                return true;
        }
        return false;
    }


    node_address& low_latency_chord::closestPrecedingNode(const uint160& searchedID) {
        node_address_latency found1, found2;       
        uint160 n  = m_nodeAddr.id < searchedID ? m_nodeAddr.id : searchedID;
        uint160 id = m_nodeAddr.id > searchedID ? m_nodeAddr.id : searchedID;
        if (isClockWiseDirection(id)) {
            if (m_cwFingers.size() > 0) {
                for (int i = (int)m_cwFingers.size()-1; i >= 0; --i) {

                    if ((n < m_cwFingers[i].id) && (m_cwFingers[i].id < id)) {
                        found1 = m_cwFingers[i];
                        
                        if (i-1 >= 0) {
                            found2 = m_cwFingers[i-1];
                            if (found1.latency > found2.latency)
                                return found2;                
                        }
                        return found1;
                    }
                }  
            }
            return m_nodeAddr;
        }
        else {
            if (m_ccwFingers.size() > 0) {
                for (int i = (int) m_ccwFingers.size()-1; i >= 0; --i) {
                    if ((n < m_ccwFingers[i].id) && (m_ccwFingers[i].id < id)) {
                        found1 = m_ccwFingers[i];

                        if (i - 1 >= 0) {
                            found2 = m_ccwFingers[i - 1];
                            if (found1.latency > found2.latency)
                                return found2;
                        }
                        return found1;
                    }
                }
            }
            return m_nodeAddr;
        }
        
    }

    
                                          
    chord_action low_latency_chord::findSuccessor(const uint160& searchedID, node_address& found) {
        found.clear();

        //if ((m_nodeAddr.isNone() == true)) {
        //    //ERROR
        //    m_logText = "findSuccessor " + LOG_TAB + LOG_ERROR_NO_ADDR;
        //    msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
        //    return DROP_MESSAGE;
        //}

        //My node is first node in p2p network
        //if ((m_cwFingers.size() < 1) && (m_ccwFingers.size() < 1)) {
        //    found = m_nodeAddr;
        //    return DO_REPLY;
        //}

        //Comparising with addrs of seed nodes        
        for (uint i = 0; i < m_seedAddrs.size(); ++i) {
            if (m_seedAddrs[i].isNone() == false) {
                if (searchedID == m_seedAddrs[i].id) {
                    found = m_seedAddrs[i];
                    return DO_REPLY;
                }
            }
        }

        //Chord: Comparising with successor, Chord
        if ((m_predecessor.isNone() == false) && (isInRange(searchedID, m_predecessor.id, false, m_nodeAddr.id, true))) {
            found = m_nodeAddr;
            return DO_REPLY;
        }
        else if (isInRange(searchedID, m_nodeAddr.id, false, m_successor.id, true)) {
            found = m_successor;     
            return DO_REPLY;
        }
        else {
            found = closestPrecedingNode(searchedID);
            if (found.id == m_nodeAddr.id)
                return DO_REPLY;
            return DO_FORWARD;
        }
    }


    bool low_latency_chord::isInRange(const uint160& id, const uint160& A, const bool includeA, const uint160& B, const bool includeB) {        
        if ((A == B) && (id == A))
            return true;

        uint160 first = A;
        bool includeF = includeA;
        if (B < A) {
            first = B;
            includeF = includeB;
        }

        uint160 last = A;
        bool includeL = includeA;
        if (B > A) {
            last = B;
            includeL = includeB;
        }
        
        if ((includeF == true) && (includeL == false)) {
            if ((first <= id) && (id < last))
                return true;
            return false;
        }
        else if ((includeF == false) && (includeL == true)) {
            if ((first < id) && (id <= last))
                return true;
            return false;
        }
        else if ((includeF == true) && (includeL == true)) {
            if ((first <= id) && (id <= last))
                return true;
            return false;
        }
        return false;
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

    void low_latency_chord::pushNewTimer(const uint type, const uint retryCounter, const chord_byte_message_fields* retryMess) {        
        chord_message timer;
        timer.type = type;
        timer.retryCounter = retryCounter;

        switch (type)
        {
        case CHORD_TIMER_RX_ACK: 
        case CHORD_TIMER_RX_SUCCESSOR_ON_JOIN:
        case CHORD_TIMER_RX_SUCCESSOR:
            if (retryMess == nullptr) {
                //ERROR
                msgLog(name(), LOG_TX, LOG_ERROR_INDICATOR, state2str(m_state) + LOG_TAB + string("pushNewTimer") + LOG_SPACE + LOG_ERROR_NULLPTR, DEBUG_LOG | ERROR_LOG);
                return;
            }            
            timer.retryMess = const_cast<chord_byte_message_fields*>( retryMess );
                            
        case CHORD_TIMER_UPDATE:
            break;

        default:   
            //ERROR
            msgLog(name(), LOG_TX, LOG_ERROR_INDICATOR, state2str(m_state) + LOG_TAB + string("pushNewTimer") + LOG_SPACE + LOG_ERROR_NOT_RECOGNIZED, DEBUG_LOG | ERROR_LOG);
            return;
        }

        pushNewMessage(timer);
    }

    chord_message& low_latency_chord::createMessage(const chord_message& params) {
        static chord_message newMess;
        newMess.clear();

        //switch (params)
        return newMess;
    }
    
    void low_latency_chord::removeTimer(buffer_container::iterator timerIt) {
        if ((m_timerBufferIndex == ERROR) || ((size_t)m_timerBufferIndex >= m_buffer.size())) {
            //ERROR
            msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, "removeTimers" + LOG_TAB + LOG_ERROR_NOT_RECOGNIZED, DEBUG_LOG | ERROR_LOG);
        }

        //for (uint i = 0; im_buffer
        m_buffer[m_timerBufferIndex].eraseMess(timerIt);
    }
    
    void low_latency_chord::removeTimer(const uint timerType, const uint retryMessType, const uint retryMessID) {
        if ((m_timerBufferIndex == ERROR) || ((size_t) m_timerBufferIndex >= m_buffer.size())) {
            //ERROR
            msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, "removeTimers" + LOG_TAB + LOG_ERROR_NOT_RECOGNIZED, DEBUG_LOG | ERROR_LOG);
        }

        //for (uint i = 0; im_buffer
        m_buffer[m_timerBufferIndex].eraseMess(timerType, retryMessType, retryMessID);
    }

    chord_message& low_latency_chord::createJoinMessage(const node_address& dest) {
        static chord_message newMess;
        newMess.clear();
        newMess.destNetwAddrs.push_back(network_address(dest.ip, dest.inSocket));
        newMess.destNodeIDwithSocket = dest;
        newMess.srcNodeIDwithSocket = m_nodeAddr;
        newMess.initiatorNodeIDwithSocket = m_nodeAddr;
        newMess.messageID = m_messageID;
        newMess.type = CHORD_TX_JOIN;
        newMess.flags.bitMessType = CHORD_BYTE_JOIN;
        newMess.flags.needsACK = m_confParams.needsACK;
        m_messageID = nextUniqueMessageID();
        return newMess;
    }

    chord_message& low_latency_chord::createNotifyMessage(const node_address& dest) {
        static chord_message newMess;
        return newMess;
    }

    chord_message& low_latency_chord::createAckMessage(const node_address& dest, const uint messageID) {
        static chord_message newMess;
        newMess.clear();
        newMess.destNetwAddrs.push_back(network_address(dest.ip, dest.inSocket));
        newMess.destNodeIDwithSocket = dest;
        newMess.srcNodeIDwithSocket = m_nodeAddr;
        newMess.type = CHORD_TX_ACK;
        newMess.messageID = messageID;
        newMess.flags.bitMessType = CHORD_BYTE_ACK;
        newMess.flags.needsACK = NO_ACK;
        return newMess;
    }

    chord_message& low_latency_chord::createFindSuccessorMessage(const node_address& dest, const node_address& whoInitiator, const uint160& whatID) {
        static chord_message newMess;
        newMess.clear();
        newMess.destNetwAddrs.push_back(network_address(dest.ip, dest.inSocket));
        newMess.destNodeIDwithSocket = dest;
        newMess.srcNodeIDwithSocket = m_nodeAddr;
        newMess.initiatorNodeIDwithSocket = whoInitiator;
        newMess.type = CHORD_TX_FIND_SUCCESSOR;
        newMess.messageID = m_messageID;
        newMess.searchedNodeIDwithSocket.id = whatID;
        newMess.flags.bitMessType = CHORD_BYTE_FIND_SUCCESSOR;
        newMess.flags.needsACK = m_confParams.needsACK ? NEEDS_ACK : NO_ACK;        
        m_messageID = nextUniqueMessageID();
        return newMess;
    }

    chord_message& low_latency_chord::createSuccessorMessage(const node_address& dest, const uint messageID, const node_address& fingerAddr) {
        static chord_message newMess;
        newMess.clear();
        newMess.destNetwAddrs.push_back(network_address(dest.ip, dest.inSocket));
        newMess.destNodeIDwithSocket = dest;
        newMess.srcNodeIDwithSocket = m_nodeAddr;
        newMess.type = CHORD_TX_SUCCESSOR;
        newMess.messageID = messageID;
        newMess.searchedNodeIDwithSocket = fingerAddr;
        newMess.flags.bitMessType = CHORD_BYTE_SUCCESSOR;
        newMess.flags.needsACK = NO_ACK;
        return newMess;
    }

    chord_message& low_latency_chord::createSingleMessage(const node_address& dest) {
        static chord_message newMess;
        m_messageID = nextUniqueMessageID();
        return newMess;
    }

    buffer_container::iterator low_latency_chord::findMessageOnTimersWithRetryParams(bool& exist, const uint timerType, const vector<uint>& retryMessTypes, const uint messageID) {
        if ((m_timerBufferIndex == ERROR) || ((size_t) m_timerBufferIndex >= m_buffer.size())) {
            //ERROR
            msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, "findMessageOnTimersWithRetryParams" + LOG_TAB + LOG_ERROR_NOT_RECOGNIZED, DEBUG_LOG | ERROR_LOG);
        }

        return m_buffer[m_timerBufferIndex].find1Mess(exist, timerType, retryMessTypes, messageID);
    }

    int low_latency_chord::checkMessage(const chord_message& mess, buffer_container::iterator& timerIt, const string& errCode) {
        
        if (isAddrValid(mess) == false) {
            m_errCode = LOG_ERROR_INVALID_ADDR;
            msgLog(name(), LOG_RX, LOG_INFO, state2str(m_state) + LOG_TAB + m_errCode, DEBUG_LOG | INTERNAL_LOG);
            return ERROR;
        }

        if (isMessageIDvalid(mess, timerIt) == false) {
            m_errCode = LOG_ERROR_INVALID_MESS_ID;
            msgLog(name(), LOG_RX, LOG_INFO, state2str(m_state) + LOG_TAB + m_errCode, DEBUG_LOG | INTERNAL_LOG);
            return ERROR;
        }
        m_errCode.clear();
        return NO_ERROR;
    }


    event_type low_latency_chord::eventType(const chord_message* mess) {
        if (mess == nullptr)
            return CALLED_BY_ANOTHER_STATE;
        
        if ((MIN_CHORD_RX_TYPE < mess->type) && (mess->type < MAX_CHORD_RX_TYPE))
            return RX_MESS_RECEIVED;

        if ((MIN_CHORD_TX_TYPE < mess->type) && (mess->type < MAX_CHORD_TX_TYPE))
            return TX_MESS_SHOULD_SEND;

        if ((MIN_CHORD_TIMER_TYPE < mess->type) && (mess->type < MAX_CHORD_TIMER_TYPE))
            return TIMER_EXPIRED;

        return EVENT_TYPE_UNKNOWN;
    }

    uint low_latency_chord::nextUniqueMessageID() {
        static uint step = 500;
        static uint min = 0;
        static uint max = step;
        static uint limit = 0xFFFF;        
        
        uint randValue;        
        randValue = genRand(min, max);
        min = (min + step) % (limit - step);
        max = min + step;        
        return randValue;
    }
}       