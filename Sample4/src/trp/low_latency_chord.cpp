#include "trp/low_latency_chord.h"

namespace P2P_MODEL
{
    low_latency_chord::low_latency_chord(sc_module_name name) : sc_module(name) {
        SC_METHOD(core);
        dont_initialize();
        sensitive << m_eventCore;

        SC_METHOD(makeSnapshot);
        sensitive << m_eventMakeSnapshot;        

        preinit();  
        
    }


    low_latency_chord::~low_latency_chord() {    }


    void low_latency_chord::preinit() { 
        m_isPaused = false;
        m_state = STATE_OFF;
        m_nodeAddr.clear();                        
        m_confParams.setDefaultTimersCountersFingersSize();
        
        allowableTimers.resize(MAX_FINITE_STATE,  vector<uint>(1, DENIED));        
        allowableRxMess.resize(MAX_FINITE_STATE,  vector<uint>(1, DENIED));
        allowableTxMess.resize(MAX_FINITE_STATE,  vector<uint>(1, DENIED));
        allowableAppreqs.resize(MAX_FINITE_STATE, vector<uint>(1, DENIED));

        allowableTimers[STATE_JOIN] = {CHORD_TIMER_RX_ACK , CHORD_TIMER_RX_SUCCESSOR_ON_JOIN , CHORD_TIMER_RX_SUCCESSOR};
        allowableRxMess[STATE_JOIN] = {CHORD_RX_ACK , CHORD_RX_SUCCESSOR};
        allowableTxMess[STATE_JOIN] = {CHORD_TX_JOIN , CHORD_TX_FIND_SUCCESSOR};

        allowableTimers[STATE_SERVICE] = {CHORD_TIMER_RX_ACK};
        allowableRxMess[STATE_SERVICE] = {CHORD_RX_ACK , CHORD_RX_JOIN , CHORD_RX_FIND_SUCCESSOR , CHORD_RX_FIND_PREDECESSOR};
        allowableTxMess[STATE_SERVICE] = {CHORD_TX_SUCCESSOR , CHORD_TX_FIND_SUCCESSOR , CHORD_TX_ACK , CHORD_TX_PREDECESSOR};

        allowableTimers[STATE_UPDATE] = {CHORD_TIMER_RX_ACK , CHORD_TIMER_RX_SUCCESSOR , CHORD_TIMER_RX_PREDECESSOR , CHORD_TIMER_UPDATE};
        allowableRxMess[STATE_UPDATE] = {CHORD_RX_SUCCESSOR , CHORD_RX_PREDECESSOR , CHORD_RX_ACK , CHORD_RX_NOTIFY};
        allowableTxMess[STATE_UPDATE] = {CHORD_TX_FIND_SUCCESSOR , CHORD_TX_ACK , CHORD_TX_FIND_PREDECESSOR , CHORD_TX_NOTIFY};

        allowableTimers[STATE_INDATA] = {CHORD_TIMER_RX_ACK};
        allowableRxMess[STATE_INDATA] = {CHORD_RX_ACK , CHORD_RX_BROADCAST , CHORD_RX_MULTICAST , CHORD_RX_SINGLE};
        allowableTxMess[STATE_INDATA] = {CHORD_TX_ACK , CHORD_TX_FWD_BROADCAST , CHORD_TX_FWD_MULTICAST , CHORD_TX_FWD_SINGLE};
        allowableAppreqs[STATE_INDATA] = {CHORD_BROADCAST , CHORD_MULTICAST , CHORD_SINGLE};

        allowableTimers[STATE_APPREQUEST] = {CHORD_TIMER_RX_ACK};
        allowableRxMess[STATE_APPREQUEST] = {CHORD_RX_ACK};
        allowableTxMess[STATE_APPREQUEST] = {CHORD_TX_BROADCAST , CHORD_TX_MULTICAST , CHORD_TX_SINGLE};

        m_txMemoryList.set(BUFF_MEMORY, MAX_SIZE_MEMORY_LIST, MAX_SIZE_MEMORY_LIST, true, 0);

        //Create buffers
        // 
        //Set priority
        //priority "0" is the highest priority
        //priority "13"  is the lowest priority
        //flag "immediate" allows to handle all messages into buffer without consider max deep limit
        uint priority = 0;        
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
        
        //Save indexes of timer and tx mess buffers
        m_timerBufferIndex = chordMessType2buffIndex(CHORD_TIMER_UPDATE);        
        if ((m_timerBufferIndex == ERROR) || (m_txMessBufferIndex == ERROR)) {
            //ERROR
            msgLog(name(), LOG_TXRX, LOG_ERROR, "preinit" + LOG_TAB + LOG_ERROR_INVALID_RANGE, ALL_LOG);
        }

        //Print on screen info about buffers (priority...)
        //static bool printOnce = false;
        //if (printOnce == false) {
        //    printOnce = true;
        //    for (int i = 0; i < m_buffer.size(); ++i)
        //        cout << m_buffer[i].toStr() << endl;

        //    for (uint i = MIN_CHORD_BYTE_TYPE + 1; i < MAX_CHORD_BYTE_TYPE; ++i)
        //        cout << chord_bits_flags().type2str(i) << LOG_TAB << i << endl;           
        //    for (uint i = MIN_CHORD_TIMER_TYPE + 1; i < MAX_CHORD_TIMER_TYPE; ++i)
        //        cout << chord_timer_message().type2str(i) << LOG_TAB << i << endl;
        //    for (uint i = MIN_CHORD_APPTX_TYPE + 1; i < MAX_CHORD_APPTX_TYPE; ++i)
        //        cout << chord_apptx_message().type2str(i) << LOG_TAB << i << endl;
        //    for (uint i = MIN_CHORD_RX_TYPE + 1; i < MAX_CHORD_RX_TYPE; ++i)
        //        cout << chord_byte_message_fields().type2str(i) << LOG_TAB << i << endl;
        //    for (uint i = MIN_CHORD_TX_TYPE + 1; i < MAX_CHORD_TX_TYPE; ++i)
        //        cout << chord_byte_message_fields().type2str(i) << LOG_TAB << i << endl;
        //}        
        
        hardReset();
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
        m_seedAddrs.clear();
        for (uint i = 0; i < m_confParams.seed.size(); ++i)
            m_seedAddrs.push_back( node_address(m_confParams.seed.at(i)) );
    }


    void low_latency_chord::setNodeID(const uint160 id) {
        m_nodeAddr.set(id, m_nodeAddr.ip, m_nodeAddr.inSocket, m_nodeAddr.outSocket);
    }


    void low_latency_chord::pushNewMessage(const chord_message& mess, const bool toBack) {
        chord_message r = mess;
        r.creatingTime = sc_time_stamp();
        r.issuedState = m_state;

        if ((r.type > MIN_CHORD_TIMER_TYPE) && (r.type < MAX_CHORD_TIMER_TYPE))
            msgLog(name(), LOG_RX, LOG_IN, state2str(m_state) + LOG_TAB + string("set ") + r.toStr(), DEBUG_LOG | INTERNAL_LOG);
        else
            msgLog(name(), LOG_RX, LOG_IN, state2str(m_state) + LOG_TAB + string("pushNewMessage ") + r.toStr(), DEBUG_LOG | EXTERNAL_LOG);
        

        //Find buffer index
        int i = chordMessType2buffIndex(r.type);
        if ((i == ERROR) || (i >= MAX_BUFF_TYPE)) {
            //ERROR
            m_logText = state2str(m_state) + LOG_TAB + string("pushNewMessage") + LOG_TAB + LOG_ERROR_INVALID_RANGE;
            msgLog(name(), LOG_TXRX, LOG_ERROR, m_logText, ALL_LOG);
            return;
        }

        //Drop rx message, if addr is zero
        if ((r.destNodeIDwithSocket.isNone()) && (r.type > MIN_CHORD_RX_TYPE) && (r.type < MAX_CHORD_RX_TYPE)) {
            m_logText = state2str(m_state) + LOG_TAB + string("pushNewMessage") + LOG_TAB + LOG_ERROR_NO_ADDR;
            msgLog(name(), LOG_TXRX, LOG_INFO, m_logText, ALL_LOG);
            return;
        }           

        //Push message into buffer
        if (m_buffer[i].push(r, toBack) == false) {
            //OVERFLOW BUFFER
            m_logText = state2str(m_state) + LOG_TAB + string("pushNewMessage") + LOG_TAB + LOG_ERROR_OVERFLOW;
            msgLog(name(), LOG_TXRX, LOG_WARNING, m_logText, ALL_LOG);
        }
            
        m_eventCore.notify(0, SC_NS);
    }


    chord_message low_latency_chord::firstMessByPriority(bool& exist) {
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
                        exist = true;
                        return *p;
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
            p = m_buffer[m_timerBufferIndex].messPointer(i);
            if (p != nullptr) {                
                if (p->type == CHORD_TIMER_RX_ACK)
                    timeout = m_confParams.TrxAck;
                else if (p->type == CHORD_TIMER_RX_SUCCESSOR_ON_JOIN)
                    timeout = m_confParams.TrxSuccOnJoin;
                else if (p->type == CHORD_TIMER_RX_SUCCESSOR)
                    timeout = m_confParams.TrxSucc;
                else if (p->type == CHORD_TIMER_RX_PREDECESSOR)
                    timeout = m_confParams.TrxSucc;
                else if (p->type == CHORD_TIMER_UPDATE)
                    timeout = m_confParams.Tupdate;
            
                sc_time whenTriggered = (p->creatingTime + timeout);
                string whenStr  = whenTriggered.to_string();
                string currTime = sc_time_stamp().to_string();
                if (sc_time_stamp() >= whenTriggered) {                    
                    if (m_canTakeDelayedTimers == true) {
                        m_indexLastBufferCall = m_timerBufferIndex;
                        m_timerItForRemove = m_buffer[m_timerBufferIndex].messIterator(i);
                        exist = true;
                        return *p;
                    }
                    else {
                        if (p->isDelayed == false) {
                            m_indexLastBufferCall = m_timerBufferIndex;
                            m_timerItForRemove = m_buffer[m_timerBufferIndex].messIterator(i);
                            exist = true;
                            return *p;
                        }
                    }
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

        exist = false;
        return chord_message();
    }


    void low_latency_chord::eraseFirstMess() {
        if ((m_indexLastBufferCall != BUFFER_NOT_CHOOSEN) && (m_indexLastBufferCall < m_buffer.size())) {
            if (m_indexLastBufferCall == m_timerBufferIndex)
                m_buffer[m_indexLastBufferCall].eraseMess(m_timerItForRemove);
            else
                m_buffer[m_indexLastBufferCall].eraseFirstMess();
            m_indexLastBufferCall = BUFFER_NOT_CHOOSEN;
        }
    }


    void low_latency_chord::eraseTxMess(const chord_tx_message_type type) {
        if ((m_txMessBufferIndex >= 0) && (m_txMessBufferIndex < m_buffer.size()))
            m_buffer[m_txMessBufferIndex].eraseMess(type);
    }


    void low_latency_chord::core() {
        if ((m_nodeAddr.isNone()) && (m_state != STATE_LOAD)) {
            //ERROR
            m_logText = "core" + LOG_TAB + LOG_ERROR_NO_ADDR;
            msgLog(name(), LOG_TXRX, LOG_ERROR, m_logText, ALL_LOG);
            m_eventCore.cancel();
            return;
        }

        string strName = name();
        string currTime = sc_time_stamp().to_string();
        
        bool exist;
        chord_message mess = firstMessByPriority(exist);        
        if (exist == false)
            return;                                  //Nothing to do
        eraseFirstMess();              

        m_logText = "core" + LOG_TAB + mess.toStr();

        switch (m_state)
        {
        case STATE_OFF:
            if (mess.type == CHORD_HARD_RESET) {                
                hardReset();
                goStateLoad();
            }
            else if (mess.type == CHORD_CONTINUE) {
                m_isPaused = false;
                setNextState(STATE_IDLE);
            }
            break;

        case STATE_INIT:
            goStateInit(mess, true);
            break;
            
        case STATE_JOIN:
            goStateJoin(mess, true);
            break;                
            
        case STATE_IDLE:
            goStateIdle(mess, true);
            break;
            
        case STATE_INDATA:
            goStateIndata(mess, true);
            break;
            
        case STATE_SERVICE:
            goStateService(mess, true);
            break;
            
        case STATE_UPDATE:
            goStateUpdate(mess, true);
            break;
            
        case STATE_APPREQUEST:
            goStateApprequest(mess, true);
            break;

        default:
            //ERROR
            m_logText = "core" + LOG_TAB + LOG_ERROR_NOT_RECOGNIZED + state2str(m_state);
            msgLog(name(), LOG_TXRX, LOG_ERROR, m_logText, ALL_LOG);
            return;
        }
            
        m_eventCore.notify(0, SC_NS);
    }

    
    void low_latency_chord::goStateLoad() {
        m_state = STATE_LOAD;
        //TODO Needs loading new config parameters
        msgLog(name(), LOG_TXRX, LOG_INFO, logHeadStateString(chord_message(), false), DEBUG_LOG|INTERNAL_LOG);
        
        goStateInit(chord_message(), false);
    }


    void low_latency_chord::goStateInit(const chord_message& mess, const bool existMess) {
        m_state = STATE_INIT;
        msgLog(name(), LOG_TXRX, LOG_INFO, logHeadStateString(mess, existMess), DEBUG_LOG|INTERNAL_LOG);

        if (doResetFlushPauseIfMess(mess, existMess) == true)
            return;

        m_messageID = nextUniqueMessageID();
        //Initializing of fingers, successor, precessor, latency by default values
        m_currSeed = 0;
        m_fingerMask.resize(m_confParams.fingersSize, 1);
        for (uint i = 1; i < m_fingerMask.size(); ++i) {
            m_fingerMask[i] = m_fingerMask.at(i-1) << 1;
        }

        if (m_confParams.fingersSize < 1) {
            //ERROR
            msgLog(name(), LOG_TXRX, LOG_ERROR, state2str(m_state) + LOG_TAB + LOG_ERROR_INVALID_RANGE, ALL_LOG);
        }
        
        cout << "Run ... node " << setprecision(0) << fixed << sc_time_stamp().to_seconds() << " sec" << LOG_SPACE << m_nodeAddr.toStrIDonly() << endl;

        msgLog(name(), LOG_TXRX, LOG_INFO, m_confParams.netwAddr.toStr(), ALL_LOG);
        msgLog(name(), LOG_TXRX, LOG_INFO, m_nodeAddr.toStr(), ALL_LOG);
        
        
        if (m_confParams.seed.size() == 0) {
            m_logText = state2str(m_state) + LOG_TAB + string("NO SEED");
            msgLog(name(), LOG_TXRX, LOG_INFO, m_logText, ALL_LOG);
            
            m_ccwFingers.resize(m_confParams.fingersSize, node_address_latency(m_nodeAddr));
            for (uint i = 0; i < m_ccwFingers.size(); ++i) {
                if (i == 0)
                    m_ccwFingers[i].from = m_nodeAddr.id - 1;
                else
                    m_ccwFingers[i].from = m_ccwFingers[i - 1].to - 1;
                m_ccwFingers[i].to = m_nodeAddr.id - m_fingerMask[i];
                

                m_ccwFingers[i].fingerIndex = i;
                m_ccwFingers[i].isClockWise = false;
                m_ccwFingers[i].isUpdated = false;
            }
            
            m_cwFingers.resize(m_confParams.fingersSize, node_address_latency(m_nodeAddr));
            for (uint i = 0; i < m_cwFingers.size(); ++i) {
                if (i == 0) 
                    m_cwFingers[i].from = m_nodeAddr.id + 1;
                else
                    m_cwFingers[i].from = m_cwFingers[i-1].to+1;
                m_cwFingers[i].to = m_nodeAddr.id+m_fingerMask[i];

                m_cwFingers[i].fingerIndex = i;
                m_cwFingers[i].isClockWise = true;
                m_cwFingers[i].isUpdated = false;
            }

            m_predecessor.clear();
            m_predecessor = m_nodeAddr;
            m_predecessor.isClockWise = false;
            m_predecessor.from = m_nodeAddr.id-1;
            m_predecessor.to = m_nodeAddr.id-1;
            m_predecessor.fingerIndex = 0;
            m_successor = m_nodeAddr;
            m_successor.fingerIndex = 0;
            m_successor.from = m_nodeAddr.id+1;
            m_successor.to = m_nodeAddr.id+1;
            m_isAcked.clear();

            if ((m_confParams.fillFingersMinQty > m_cwFingers.size()) || (m_cwFingers.size() == 0)) {
                m_confParams.fillFingersMinQty = (uint) m_cwFingers.size();
                //ERROR
                msgLog(name(), LOG_TXRX, LOG_ERROR, state2str(m_state) + LOG_TAB + "confParams.fillFingersMinQty >= m_cwFingers.size()", ALL_LOG);                
            }
            setNextState(STATE_IDLE);
            
        }
        else {
            m_ccwFingers.resize(m_confParams.fingersSize, node_address_latency(m_nodeAddr));
            for (uint i = 0; i < m_ccwFingers.size(); ++i) {
                if (i == 0)
                    m_ccwFingers[i].from = m_nodeAddr.id - 1;
                else
                    m_ccwFingers[i].from = m_ccwFingers[i - 1].to - 1;
                m_ccwFingers[i].to = m_nodeAddr.id - m_fingerMask[i];                

                m_ccwFingers[i].fingerIndex = i;
                m_ccwFingers[i].isClockWise = false;
                m_ccwFingers[i].isUpdated = false;               
            }

            m_cwFingers.resize(m_confParams.fingersSize, node_address_latency(m_nodeAddr));
            for (uint i = 0; i < m_cwFingers.size(); ++i) {
                if (i == 0)
                    m_cwFingers[i].from = m_nodeAddr.id + 1;
                else
                    m_cwFingers[i].from = m_cwFingers[i - 1].to + 1;
                m_cwFingers[i].to = m_nodeAddr.id + m_fingerMask[i];

                m_cwFingers[i].fingerIndex = i;
                m_cwFingers[i].isClockWise = true;
                m_cwFingers[i].isUpdated = false;
            }

            m_predecessor.clear();
            m_predecessor = m_nodeAddr;
            m_predecessor.isClockWise = false;
            m_predecessor.from = m_nodeAddr.id - 1;
            m_predecessor.to = m_nodeAddr.id - 1;
            m_predecessor.fingerIndex = 0;            
            m_successor = m_nodeAddr;
            m_successor.fingerIndex = 0;
            m_successor.from = m_nodeAddr.id + 1;
            m_successor.to = m_nodeAddr.id + 1;
            m_isAcked.clear();

            if ((m_confParams.fillFingersMinQty > m_cwFingers.size()) || (m_ccwFingers.size() == 0)) {
                //ERROR
                msgLog(name(), LOG_TXRX, LOG_ERROR, state2str(m_state) + LOG_TAB + "confParams.fillFingersMinQty >= m_cwFingers.size()", ALL_LOG);
                m_confParams.fillFingersMinQty = (uint) m_cwFingers.size();
            }            
            goStateJoin(chord_message(), false);
        }

        if (CCW_FINGERS_TURN_ON == false)
            m_ccwFingers.resize(1);
    }

        
    bool low_latency_chord::isAddrValid(const chord_message& mess) {                
        //NEW NEW NEW
        if (mess.retransmitCounter > 1.5 * m_cwFingers.size()) {
            //cout << LOG_ERROR_TOO_BIG_RETRANSMIT << getchar();
            m_errCode = LOG_ERROR_TOO_BIG_RETRANSMIT;
            msgLog(name(), LOG_RX, LOG_INFO, state2str(m_state) + LOG_TAB + m_errCode, DEBUG_LOG | INTERNAL_LOG);
            return false;
        }
        //NEW NEW NEW

        if (mess.destNodeIDwithSocket.id == m_nodeAddr.id)
            return true;

        return false;
    }


    bool low_latency_chord::isMessageIDvalid(const chord_message& mess, buffer_container::iterator& it) {
        static buffer_container stub;
        if (stub.size() == 0)
            stub.push_back(chord_message());

        it = stub.begin();
        vector<chord_tx_message_type> retryMessTypes;
        chord_timer_message_type timer;
        bool exist;
        if (mess.type == CHORD_RX_ACK) {            
            retryMessTypes = {CHORD_TX_JOIN, CHORD_TX_NOTIFY, CHORD_TX_FIND_SUCCESSOR, CHORD_TX_FIND_PREDECESSOR, CHORD_TX_BROADCAST, CHORD_TX_MULTICAST, CHORD_TX_SINGLE, CHORD_TX_FWD_BROADCAST, CHORD_TX_FWD_MULTICAST, CHORD_TX_FWD_SINGLE};
            timer = CHORD_TIMER_RX_ACK;
            if (m_confParams.TrxAck == NO_TIMEOUT) {
                return true;                //There are no `ACK` timers
            }
            
            it = findMessageOnTimersWithRetryParams(exist, timer, retryMessTypes, mess.messageID);
            return exist;
        }
        else if (mess.type == CHORD_RX_SUCCESSOR) {
            retryMessTypes = {CHORD_TX_JOIN, CHORD_TX_FIND_SUCCESSOR};
            timer = CHORD_TIMER_RX_SUCCESSOR;
            if (m_confParams.TrxSucc == NO_TIMEOUT) {
                return true;                 //There are no `Successor` timers            
            }
                
            if ((m_isSuccessorSet == false) && (m_state == STATE_JOIN)) {
                timer = CHORD_TIMER_RX_SUCCESSOR_ON_JOIN;
                if (m_confParams.TrxSuccOnJoin == NO_TIMEOUT) {
                    return true;            //There are no `Successor on join` timers                                                        
                }
            }
            
            it = findMessageOnTimersWithRetryParams(exist, timer, retryMessTypes, mess.messageID);
            return exist;
        }   
        else if (mess.type == CHORD_RX_PREDECESSOR) {
            retryMessTypes = { CHORD_TX_FIND_PREDECESSOR };
            timer = CHORD_TIMER_RX_PREDECESSOR;
            if (m_confParams.TrxPred == NO_TIMEOUT) {
                return true;                //There are no `Predecessor` timers  
            }
            it = findMessageOnTimersWithRetryParams(exist, timer, retryMessTypes, mess.messageID);
            return exist;
        }
        
        return true;
    }


    void low_latency_chord::goStateJoin(const chord_message& mess, const bool existMess) {
        m_state = STATE_JOIN;
        msgLog(name(), LOG_TXRX, LOG_INFO, logHeadStateString(mess, existMess), DEBUG_LOG|INTERNAL_LOG);

        if (doResetFlushPauseIfMess(mess, existMess) == true)
            return;
        
        if (m_isPaused == false) {

            chord_timer_message timer;
            bool isIssueSuccess;
            bool isRepeatSuccess;

            event_type evType = eventType(mess, existMess);
            switch (evType) {
                case CALLED_BY_ANOTHER_STATE: {           
                    issueMessagePushTimers(CHORD_TX_JOIN, false, 0);                                         //First call this state, message wasn't received, timer wasn't expired               
                }
                break;

                case TX_MESS_SHOULD_SEND: {
                    if ((mess.type == CHORD_TX_JOIN) || (mess.type == CHORD_TX_FIND_SUCCESSOR))              //TX message should be sent            
                        sendMessage(mess);    
                }
                break;

                case RX_MESS_RECEIVED: {               //RX Message was received 
                    if (mess.type == CHORD_RX_SUCCESSOR) {                
                        if (checkMessage(mess, timer, m_errCode) != ERROR) {                                                      
                            if (m_isSuccessorSet == false) {
                                //DEBUG
                                if ((sc_time_stamp() >= sc_time(48020, SC_MS)) && (name() == string("trp10.llchord")))
                                    int tmp = mess.messageID;
                                //DEBUG
                                
                                chord_message m = mess;
                                m.searchedNodeIDwithSocket = mess.serviceAddr;
                                setPredecessor(m, chord_timer_message(), true);

                                setSuccessorRemoveTimers(mess, timer); 
                                m_updateType = setNextFingerToUpdate();
                            }
                            else {
                                setFingerRemoveTimers(mess, timer);
                                m_updateType = setNextFingerToUpdate();
                            }
    
                            if ((m_cwFingerIndex < m_confParams.fillFingersMinQty) && /*(m_isPredecessorSet == false) &&*/ (m_cwFingers.at(m_cwFingerIndex).updateTime < sc_time_stamp()))
                                issueMessagePushTimers(CHORD_TX_FIND_SUCCESSOR, false, 0);                  
                            else {
                                setNextState(STATE_IDLE);
                                pushNewTimer(CHORD_TIMER_UPDATE, 0, 0, chord_byte_message_fields());
                            }
                        }
                    }
                    else if (mess.type == CHORD_RX_ACK) {
                        if ((checkMessage(mess, timer, m_errCode) != ERROR) && (m_confParams.needsACK == NEEDS_ACK)) {
                            removeTimer((chord_timer_message_type)timer.type, (chord_tx_message_type)timer.retryMess.type, timer.retryMess.messageID);                        
                        }                    
                    }            
                }
                break;

                case TIMER_EXPIRED:{                   //Timer was expired, analysing
                    timer = mess;
                    if (mess.type == CHORD_TIMER_RX_ACK) {
                        if (mess.retryMess.type == CHORD_TX_JOIN) {                                            
                            removeTimer(CHORD_TIMER_RX_SUCCESSOR_ON_JOIN, CHORD_TX_JOIN, mess.retryMess.messageID);                    //Join message was sent early
                            isIssueSuccess = issueMessagePushTimers(CHORD_TX_JOIN, true, timer.requestCounter, mess.retryMess, timer);
                            if (isIssueSuccess == false) {
                                isRepeatSuccess = repeatMessage(CHORD_TX_JOIN, mess.retryMess, timer);
                                if (isRepeatSuccess == false)
                                    issueMessagePushTimers(CHORD_TX_JOIN, false, 0);                        
                            }
                        }
                        else if (mess.retryMess.type == CHORD_TX_FIND_SUCCESSOR) {
                            removeTimer(CHORD_TIMER_RX_SUCCESSOR, CHORD_TX_FIND_SUCCESSOR, mess.retryMess.messageID);
                            isIssueSuccess = issueMessagePushTimers(CHORD_TX_FIND_SUCCESSOR, true, timer.requestCounter, mess.retryMess, timer);
                            if (isIssueSuccess == false) {
                                isRepeatSuccess = repeatMessage(CHORD_TX_FIND_SUCCESSOR, mess.retryMess, timer);
                                if (isRepeatSuccess == false) {
                                    if (setCopyPreviousAliveFinger() == false) {
                                        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("NO alive fingers. Do reset"), ALL_LOG);
                                        chord_conf_message reset;
                                        reset.type = CHORD_HARD_RESET;
                                        pushNewMessage(reset);
                                    }
                                    else {
                                        m_updateType = setNextFingerToUpdate();
                                        if (m_cwFingerIndex < m_confParams.fillFingersMinQty)
                                            issueMessagePushTimers(CHORD_TX_FIND_SUCCESSOR, false, 0);
                                        else {
                                            setNextState(STATE_IDLE);
                                            pushNewTimer(CHORD_TIMER_UPDATE, 0, 0, chord_byte_message_fields());
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else if (mess.type == CHORD_TIMER_RX_SUCCESSOR_ON_JOIN) {
                        if (mess.retryMess.type == CHORD_TX_JOIN) {
                            removeTimer(CHORD_TIMER_RX_ACK, CHORD_TX_JOIN, mess.retryMess.messageID);
                            isRepeatSuccess = repeatMessage(CHORD_TX_JOIN, mess.retryMess, timer);
                            if (isRepeatSuccess == false)
                                issueMessagePushTimers(CHORD_TX_JOIN, false, 0);                            
                        }
                    }
                    else if (mess.type == CHORD_TIMER_RX_SUCCESSOR) {
                        if (mess.retryMess.type == CHORD_TX_FIND_SUCCESSOR) {
                            removeTimer(CHORD_TIMER_RX_ACK, CHORD_TX_FIND_SUCCESSOR, mess.retryMess.messageID);
                            isRepeatSuccess = repeatMessage(CHORD_TX_FIND_SUCCESSOR, mess.retryMess, timer);
                            if (isRepeatSuccess == false) {                                
                                if (setCopyPreviousAliveFinger() == false) {
                                    msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("NO alive fingers. Do reset"), ALL_LOG);
                                    chord_conf_message reset;
                                    reset.type = CHORD_HARD_RESET;
                                    pushNewMessage(reset);
                                }
                                else {
                                    m_updateType = setNextFingerToUpdate();
                                    if (m_cwFingerIndex < m_confParams.fillFingersMinQty)
                                        issueMessagePushTimers(CHORD_TX_FIND_SUCCESSOR, false, 0);
                                    else {
                                        setNextState(STATE_IDLE);
                                        pushNewTimer(CHORD_TIMER_UPDATE, 0, 0, chord_byte_message_fields());
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }        
            setNextState(STATE_IDLE);
        }
    }


    void low_latency_chord::goStateIdle(const chord_message& mess, const bool existMess) {
        m_state = STATE_IDLE;

        if (doResetFlushPauseIfMess(mess, existMess) == true)
            return;

        if (m_isPaused == false) {
            
            chord_timer_message timer;

            low_latency_chord::FP funcPointer = nullptr;
            event_type evType = eventType(mess, existMess);

            switch (evType) {
                case CALLED_BY_ANOTHER_STATE: {      
                    //ERROR
                    msgLog(name(), LOG_TXRX, LOG_ERROR, state2str(m_state) + LOG_TAB + "Who call me?", ALL_LOG);
                }
                break;

                case TX_MESS_SHOULD_SEND: {
                    sendMessage(mess);
                }
                break;

                case RX_MESS_RECEIVED: {               //RX Message was received                       
                    funcPointer = mess2state(evType, mess);
                    if (funcPointer != nullptr)
                        (this->*funcPointer)(mess, existMess);
                    else
                        msgLog(name(), LOG_TXRX, LOG_WARNING, state2str(m_state) + LOG_TAB + "Drop message " + mess.toStr(), ALL_LOG);
                }
                break;

                case TIMER_EXPIRED:{                     //Timer was expired, analysing                

                    funcPointer = mess2state(evType, mess);
                    if (funcPointer != nullptr)
                        (this->*funcPointer)(mess, existMess);
                    else
                        msgLog(name(), LOG_TXRX, LOG_WARNING, state2str(m_state) + LOG_TAB + "Drop message " + mess.toStr(), ALL_LOG);
                }
                break;
            }       
        }
    }

    void low_latency_chord::goStateService(const chord_message& mess, const bool existMess) {
        m_state = STATE_SERVICE;
        msgLog(name(), LOG_TXRX, LOG_INFO, logHeadStateString(mess, existMess), DEBUG_LOG|INTERNAL_LOG);

        if (doResetFlushPauseIfMess(mess, existMess) == true)
            return;
        
        if (m_isPaused == false) {        
           
            chord_message newMess;
            chord_timer_message timer;
            node_address lookupAddr;
            bool isIssueSuccess;


            switch (eventType(mess, existMess)) {
                case CALLED_BY_ANOTHER_STATE: {
                    msgLog(name(), LOG_TXRX, LOG_INFO, logHeadStateString(mess, existMess), DEBUG_LOG | INTERNAL_LOG);
                }
                break;

                case TX_MESS_SHOULD_SEND: {                        
                    if ((mess.type == CHORD_TX_SUCCESSOR) || (mess.type == CHORD_TX_FIND_SUCCESSOR) || (mess.type == CHORD_TX_ACK) || (mess.type == CHORD_TX_PREDECESSOR)) 
                        sendMessage(mess);                
                }
                break;

                case RX_MESS_RECEIVED: {           
                    if ((mess.type == CHORD_RX_JOIN) || (mess.type == CHORD_RX_FIND_SUCCESSOR)) {
                        if (checkMessage(mess, timer, m_errCode) != ERROR) {

                            if ((m_seedAddrs.size() == 0) && (mess.type == CHORD_RX_JOIN)) {
                                m_seedAddrs.push_back(mess.srcNodeIDwithSocket);
                                setNextState(STATE_IDLE);
                                pushNewTimer(CHORD_TIMER_UPDATE, 0, 0, chord_byte_message_fields());
                                setNextState(STATE_SERVICE);                                                                                                                              
                            }


                            issueMessagePushTimers(CHORD_TX_ACK, false, 0, mess);            

                            chord_action action = findSuccessor(mess.searchedNodeIDwithSocket.id, mess.srcNodeIDwithSocket.id, mess.initiatorNodeIDwithSocket.id, lookupAddr, mess.isJoin);

                                                                                          
                                                        
                            if (action == DO_REPLY) {           
                                issueMessagePushTimers(CHORD_TX_SUCCESSOR, false, 0, mess, chord_timer_message(), DO_REPLY, lookupAddr);


                                //New node was connected. We got join as fwd_find_successor from boost node
                                if (mess.isJoin) {
                                    m_isNowStabilize = false;

                                    chord_message m = mess;
                                    m.searchedNodeIDwithSocket = mess.initiatorNodeIDwithSocket;
                                    setPredecessor(m, chord_timer_message(), false);

                                    if (m_successor.id == m_nodeAddr.id) //If node is alone in a network
                                        setSuccessorRemoveTimers(m, chord_timer_message());

                                    if (MAKE_SNAPSHOT_ALWAYS) makeSnapshot();
                                }

                                if ((lookupAddr.id == m_nodeAddr.id) || (mess.type == CHORD_RX_JOIN)) {
                                    forceUpdateFingerTable(mess);
                                }
                            }
                            else if (action == DO_FORWARD) {
                                forceUpdateFingerTable(mess);
                                issueMessagePushTimers(CHORD_TX_FIND_SUCCESSOR, false, 0, mess, chord_timer_message(), DO_FORWARD, lookupAddr);                                
                            }
                        }
                    }
                    else if (mess.type == CHORD_RX_FIND_PREDECESSOR) {
                        if (checkMessage(mess, timer, m_errCode) != ERROR) {
                            issueMessagePushTimers(CHORD_TX_ACK, false, 0, mess);

                            chord_action action = findPredecessor(mess.searchedNodeIDwithSocket.id, lookupAddr);
                            issueMessagePushTimers(CHORD_TX_PREDECESSOR, false, 0, mess, chord_timer_message(), action, lookupAddr);
                        }
                    }
                    else if (mess.type == CHORD_RX_ACK) {
                        if ((checkMessage(mess, timer, m_errCode) != ERROR) && (m_confParams.needsACK == NEEDS_ACK)) {
                            removeTimer((chord_timer_message_type)timer.type, (chord_tx_message_type)timer.retryMess.type, timer.retryMess.messageID);
                        }
                    }
                }
                break;

                case TIMER_EXPIRED: {                   //Timer was expired, analysing               
                    timer = mess;
                    if (mess.type == CHORD_TIMER_RX_ACK) {
                        if (mess.retryMess.type == CHORD_TX_FIND_SUCCESSOR) {
                            removeTimer(CHORD_TIMER_RX_SUCCESSOR, CHORD_TX_FIND_SUCCESSOR, mess.retryMess.messageID);
                            isIssueSuccess = issueMessagePushTimers(CHORD_TX_FIND_SUCCESSOR, true, timer.requestCounter, mess.retryMess, timer, DO_FORWARD, mess.retryMess.searchedNodeIDwithSocket);
                            if (isIssueSuccess == false) {
                                timer.isDelayed = true;
                                pushNewTimer(timer, false);
                                goStateUpdate(timer, true);
                            }
                        }                    
                    }
                }
                break;
            }
            setNextState(STATE_IDLE);
        }
    }


    void low_latency_chord::goStateUpdate(const chord_message& mess, const bool existMess)  {
        m_state = STATE_UPDATE;
        msgLog(name(), LOG_TXRX, LOG_INFO, logHeadStateString(mess, existMess), DEBUG_LOG | INTERNAL_LOG);        

        if (doResetFlushPauseIfMess(mess, existMess) == true)
            return;
        
        if (m_isPaused == false) {

            msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("isNowUpdate = ") + LOG_BOOL(m_isNowUpdate), DEBUG_LOG | INTERNAL_LOG);

            chord_message newMess;
            node_address lookupAddr;
            bool isIssueSuccess;
            bool isRepeatSuccess;
            chord_timer_message timer;

            //m_canTakeDelayedTimers = false;
            event_type evType = eventType(mess, existMess);
            switch (evType) {
                case INACCESSIBLE_NODE: {
                    pushInaccessibleFinger(mess);

                    if (m_isNowUpdate == false) {
                        m_isNowUpdate = true;
                        node_address_latency& badFinger = m_inaccessibleFingers.at(0).badFinger;
                        m_updateType = setNextFingerToUpdate(false, badFinger);
                        if (m_updateType == INACCESSIBLE_FINGER)
                            issueMessagePushTimers(CHORD_TX_FIND_SUCCESSOR); //issueMessagePushTimers(CHORD_TX_FIND_PREDECESSOR);
                    }
                }
                break;                       

                case TX_MESS_SHOULD_SEND: {                        
                    if ((mess.type == CHORD_TX_FIND_SUCCESSOR) ||
                        (mess.type == CHORD_TX_ACK) ||
                        (mess.type == CHORD_TX_FIND_PREDECESSOR) ||
                        (mess.type == CHORD_TX_NOTIFY))
                        sendMessage(mess);                
                }
                break;

                case RX_MESS_RECEIVED: {           
                    if (mess.type == CHORD_RX_SUCCESSOR) {
                        if (checkMessage(mess, timer, m_errCode) != ERROR) {  
                            m_isNowUpdate = false;
                            setFingerRemoveTimers(mess, timer);                        
                        }
                    }
                    else if (mess.type == CHORD_RX_PREDECESSOR) {
                        if (checkMessage(mess, timer, m_errCode) != ERROR) {   
                            if (m_confParams.needsACK == NO_ACK)
                                m_isNowUpdate = false;
                            setSuccessorStabilize(mess, timer);                        
                            setNextFingerToUpdate();
                            issueMessagePushTimers(CHORD_TX_NOTIFY);                        
                        }
                    }
                    else if (mess.type == CHORD_RX_ACK) {
                        if ((checkMessage(mess, timer, m_errCode) != ERROR) && (m_confParams.needsACK == NEEDS_ACK)) {
                            if (timer.retryMess.type == CHORD_TX_NOTIFY)
                                m_isNowUpdate = false;
                            removeTimer((chord_timer_message_type)timer.type, (chord_tx_message_type)timer.retryMess.type, timer.retryMess.messageID);
                        }
                    }
                    else if (mess.type == CHORD_RX_NOTIFY) {
                        if ((checkMessage(mess, timer, m_errCode) != ERROR) && (m_confParams.needsACK == NEEDS_ACK)) {                        
                            issueMessagePushTimers(CHORD_TX_ACK, false, 0, mess);
                            setPredecessor(mess, timer);
                        }
                    }
                }
                break;

                case TIMER_EXPIRED: {                   //Timer was expired, analysing
                    timer = mess;

                    if (mess.type == CHORD_TIMER_RX_ACK) {
                        if (mess.retryMess.type == CHORD_TX_FIND_SUCCESSOR) {

                            removeTimer(CHORD_TIMER_RX_SUCCESSOR, CHORD_TX_FIND_SUCCESSOR, mess.retryMess.messageID);                        
                            isIssueSuccess = issueMessagePushTimers(CHORD_TX_FIND_SUCCESSOR, true, timer.requestCounter, mess.retryMess, timer, DO_REQUEST, mess.retryMess.searchedNodeIDwithSocket);
                            if (isIssueSuccess == false) {
                                isRepeatSuccess = repeatMessage(CHORD_TX_FIND_SUCCESSOR, mess.retryMess, timer);
                                if (isRepeatSuccess == false) {
                                    m_isNowUpdate = false;

                                    if (mess.retryMess.retransmitCounter == 0)
                                        if (setCopyPreviousAliveFinger() == false) {
                                            msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("NO alive fingers. Do reset"), ALL_LOG);
                                            chord_conf_message reset;
                                            reset.type = CHORD_HARD_RESET;
                                            pushNewMessage(reset);
                                        }                                    
                                }     
                            }
                        }
                        else if (mess.retryMess.type == CHORD_TX_FIND_PREDECESSOR) {
                            removeTimer(CHORD_TIMER_RX_PREDECESSOR, CHORD_TX_FIND_PREDECESSOR, mess.retryMess.messageID);
                            isIssueSuccess = issueMessagePushTimers(CHORD_TX_FIND_PREDECESSOR, true, timer.requestCounter, mess.retryMess, timer, DO_REQUEST, mess.retryMess.searchedNodeIDwithSocket);
                            if (isIssueSuccess == false) {
                                isRepeatSuccess = repeatMessage(CHORD_TX_FIND_PREDECESSOR, mess.retryMess, timer);
                                if (isRepeatSuccess == false) {
                                    m_isNowUpdate = false;
                                    msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("ACK wasn't received on TX_FIND_PREDECESSOR ") + to_string(m_updateType), ALL_LOG);
                                }
                            }
                        }
                        else if (mess.retryMess.type == CHORD_TX_NOTIFY) {
                            //removeTimer(CHORD_TIMER_RX_ACK, CHORD_TX_NOTIFY, mess.retryMess.messageID);
                            isIssueSuccess = issueMessagePushTimers(CHORD_TX_NOTIFY, true, timer.requestCounter, mess.retryMess, timer, DO_REQUEST, mess.retryMess.searchedNodeIDwithSocket);
                            if (isIssueSuccess == false) {
                                m_isNowUpdate = false;
                                msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("ACK wasn't received on TX_NOTIFY ") + to_string(m_updateType), ALL_LOG);
                            }
                        }
                    }
                    else if (mess.type == CHORD_TIMER_RX_SUCCESSOR) {
                        isRepeatSuccess = repeatMessage(CHORD_TX_FIND_SUCCESSOR, mess.retryMess, timer);
                        if (isRepeatSuccess == false) {
                            m_isNowUpdate = false;
                            
                            //NEW NEW NEW
                            m_isNowStabilize = false;
                            //NEW NEW NEW

                            //if (setCopyPreviousAliveFinger() == false) {
                            //    msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("NO alive fingers. Do reset"), ALL_LOG);
                            //    chord_conf_message reset;
                            //    reset.type = CHORD_HARD_RESET;
                            //    pushNewMessage(reset);
                            //} 
                        }
                    }
                    else if (mess.type == CHORD_TIMER_RX_PREDECESSOR) {
                        isRepeatSuccess = repeatMessage(CHORD_TX_FIND_PREDECESSOR, mess.retryMess, timer);
                        if (isRepeatSuccess == false) {
                            m_isNowUpdate = false;
                            
                            if (setCopyPreviousAliveFinger() == false) {
                                msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("NO alive fingers. Do reset"), ALL_LOG);
                                chord_conf_message reset;
                                reset.type = CHORD_HARD_RESET;
                                pushNewMessage(reset);
                            }                            
                        }
                    }
                    else if (mess.type == CHORD_TIMER_UPDATE) {                    
                        pushNewTimer(CHORD_TIMER_UPDATE, 0, 0, chord_byte_message_fields());                                    

                        if (m_isNowUpdate == false) {
                            m_isNowUpdate = true;

                            m_updateType = setNextFingerToUpdate(true);
                            if (m_updateType == STABILIZE_SUCCESSOR) {

                                issueMessagePushTimers(CHORD_TX_FIND_PREDECESSOR, false, 0);                    
                            }
                            else if (m_updateType == FIX_FINGER)
                                issueMessagePushTimers(CHORD_TX_FIND_SUCCESSOR, false, 0);
                            else {
                                //ERROR
                                msgLog(name(), LOG_TXRX, LOG_ERROR, state2str(m_state) + LOG_TAB + string("update type ") + to_string(m_updateType), ALL_LOG);
                            }
                        }     
                    
                    }
                }
                break;
            }        

            msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("isNowUpdate = ") + LOG_BOOL(m_isNowUpdate), DEBUG_LOG | INTERNAL_LOG);
            setNextState(STATE_IDLE);    
            
            //if ((mess.type != CHORD_RX_ACK) && (evType != TX_MESS_SHOULD_SEND))
            //    makeSnapshot();
        }        
    }

    
    void low_latency_chord::goStateApprequest(const chord_message& mess, const bool existMess) { 
        m_state = STATE_INDATA;
        msgLog(name(), LOG_TXRX, LOG_INFO, logHeadStateString(mess, existMess), DEBUG_LOG | INTERNAL_LOG);

        if (doResetFlushPauseIfMess(mess, existMess) == true)
            return;

        if (m_isPaused == false) {
            setNextState(STATE_IDLE);
        }
    }


    void low_latency_chord::goStateIndata(const chord_message& mess, const bool existMess) {
        m_state = STATE_INDATA;
        msgLog(name(), LOG_TXRX, LOG_INFO, logHeadStateString(mess, existMess), DEBUG_LOG | INTERNAL_LOG);

        if (doResetFlushPauseIfMess(mess, existMess) == true)
            return;

        if (m_isPaused == false) {
            setNextState(STATE_IDLE);
        }
    }


    int low_latency_chord::chordMessType2buffIndex(const uint type) {
        static map<uint, uint> mapBuffIndex;
        uint buffType = 0;
        switch (type) {            
        case CHORD_HARD_RESET:
        case CHORD_SOFT_RESET:
        case CHORD_FLUSH:        
        case CHORD_PAUSE:        
        case CHORD_CONTINUE:     
        case CHORD_CONF:        buffType = BUFF_CONFIG; break;

        case CHORD_TIMER_RX_ACK:                   
        case CHORD_TIMER_RX_SUCCESSOR:       
        case CHORD_TIMER_RX_SUCCESSOR_ON_JOIN:  
        case CHORD_TIMER_RX_PREDECESSOR:
        case CHORD_TIMER_UPDATE: buffType = BUFF_TIMER;   break;

        case CHORD_SINGLE:
        case CHORD_MULTICAST:
        case CHORD_BROADCAST:    buffType = BUFF_APPTXDATA; break;

        case CHORD_RX_JOIN:
        case CHORD_RX_NOTIFY:
        case CHORD_RX_ACK:
        case CHORD_RX_SUCCESSOR:
        case CHORD_RX_FIND_SUCCESSOR:
        case CHORD_RX_PREDECESSOR:
        case CHORD_RX_FIND_PREDECESSOR:
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
        case CHORD_TX_FWD_BROADCAST:  buffType = /*BUFF_TX_FWD_BROADCAST;*/ BUFF_TX_BROADCAST; break;
        case CHORD_TX_FWD_MULTICAST:  buffType = /*BUFF_TX_FWD_MULTICAST;*/ BUFF_TX_MULTICAST; break;
        case CHORD_TX_FWD_SINGLE:     buffType = /*BUFF_TX_FWD_SINGLE;   */ BUFF_TX_SINGLE;    break;
        case CHORD_TX_BROADCAST:      buffType = BUFF_TX_BROADCAST;      break;
        case CHORD_TX_MULTICAST:      buffType = BUFF_TX_MULTICAST;      break;
        case CHORD_TX_SINGLE:         buffType = BUFF_TX_SINGLE;         break;

        default:
            //ERROR
            msgLog(name(), LOG_TXRX, LOG_ERROR, string("chordMessType2buffIndex ") + LOG_ERROR_NOT_RECOGNIZED, DEBUG_LOG | ERROR_LOG | EXTERNAL_LOG);
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
        m_isPaused = false;

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
        m_isAcked.clear();
        m_successor.clear();
        m_predecessor.clear();
        m_currSeed = 0;
        m_isSuccessorSet = false;
        m_isPredecessorSet = false;
        m_cwFingerIndex = 0;
        m_ccwFingerIndex = 0;
        m_cwFingerIndexToUpdate = 0;
        m_ccwFingerIndexToUpdate = 0;
        m_isClockWise = true;
        m_messageID = 0;
    }

    
    void low_latency_chord::flush() {
        m_eventCore.cancel();

        //Messages, timers are resetted
        for (uint i = 0; i < m_buffer.size(); ++i)
            m_buffer[i].clearMessages();
        m_txMemoryList.clearMessages();
        m_isNowUpdate = false;
        m_canTakeDelayedTimers = false;
        m_indexLastBufferCall = 0;
        m_isNowStabilize = false;
        //Fingers, latency, precessor, successor are stored
    }


    void low_latency_chord::sendMessage(const chord_message& mess) {
        if ((mess.destNodeIDwithSocket.isNone()) || (mess.destNodeIDwithSocket.id == m_nodeAddr.id)) {
            //ERROR
            m_logText = "sendMessage" + LOG_TAB + LOG_ERROR_NO_ADDR + LOG_SPACE + mess.toStr();
            msgLog(name(), LOG_TX, LOG_ERROR, m_logText, ALL_LOG);
            return;
        }        

        //LOG
        m_logText = state2str(m_state) + LOG_TAB + string("sendMessage") + LOG_TAB + mess.toStr();
        msgLog(name(), LOG_TX, LOG_OUT, m_logText, DEBUG_LOG | INTERNAL_LOG);

        chord_byte_message raw;
        raw.fields = mess;



        pushTxMessageMemoryList(mess);

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
                if (m_cwFingers.at(i).id >= id) //Is situation: 6 > 5(id) ?
                    return true;
                else if (i >= 1) {
                    if ((m_cwFingers.at(i).id < id) && (id > m_cwFingers.at(i-1).id)) //Is situation: 0 < 5(id) AND 5(id) > 4  values on line: {4, id, 0, ...}
                        return true;
                }         
            }

            if ((m_cwFingers.back().id < id) && (id < m_ccwFingers.back().id)) //Is situation: 6 < 7(id) AND 7(id) < 8  values on line: {..., 6} id here {8, ...} 
                return true;
        }
        
        return false;
    }

    
    void low_latency_chord::findBigFinger(const vector<node_address_latency>& fingers, const vector<uint160>& forbidden, node_address_latency& result) {
        node_address_latency big;
        big.id = 0;
        for (const auto& i : fingers) {
            bool accept = true;
            for (const auto& j : forbidden) {
                if (i.id == j) {
                    accept = false;
                    break;
                }
            }
            
            if ((i.id > big.id) && (accept))
                big.setCopy(i);
            
        }
        result.setCopy(big);
    }


    void low_latency_chord::findAlittleBigFinger(const uint160 searchedID, const vector<node_address_latency>& fingers, const vector<uint160>& forbidden, node_address_latency& result) {
        static node_address_latency result_;
        result.id = 0;

        for (size_t i = 1; i < fingers.size(); ++i) {
            bool accept = true;
            for (const auto& j : forbidden) {
                if (fingers.at(i).id == j) {
                    accept = false;
                    break;
                }
            }

            if ((fingers.at(i-1).id <= searchedID) && (searchedID <= fingers.at(i).id) && (accept)) {
                result.setCopy(fingers.at(i));
                break;
            }                
        }
        result.setCopy(result_);
    }


    void low_latency_chord::find1stFingerNotForbidden(const vector<node_address_latency>& fingers, const vector<uint160>& forbidden, node_address_latency& result) {
        node_address_latency result_;
        result_.id = 0;

        for (const auto& i : fingers) {
            bool accept = true;
            for (const auto& j : forbidden) {
                if (i.id == j) {
                    accept = false;
                    break;
                }
            }

            if (accept) {
                result_.setCopy(i);
                break;
            }
        }
        result.setCopy(result_);
    }

    
    chord_action low_latency_chord::findSuccessor(const uint160& searchedID, const uint160& senderID, const uint160& initiatorID, node_address& found, const bool isJoin) {
        //LOG
        m_ssLog << state2str(m_state) << LOG_TAB << "findSuccessor " << "........" << " sID " << searchedID.to_string(SC_DEC) << " ?? [pred " << m_predecessor.id.to_string(SC_DEC) << "; " << m_nodeAddr.id.to_string(SC_DEC) << ") node ";
        m_ssLog << m_nodeAddr.id.to_string(SC_DEC) << " (" << m_nodeAddr.id.to_string(SC_DEC) << "; succ " << m_successor.id.to_string(SC_DEC) << "]";
        msgLog(name(), LOG_RX, LOG_INFO, m_ssLog.str(), DEBUG_LOG | INTERNAL_LOG);

        for (const auto& finger : m_cwFingers) {            
            m_ssLog << state2str(m_state) << LOG_TAB << "findSuccessor " << "........" << " sID " << searchedID.to_string(SC_DEC) << " ?? cw fing" << finger.fingerIndex << " " << finger.toStrIDonly();
            msgLog(name(), LOG_RX, LOG_INFO, m_ssLog.str(), DEBUG_LOG | INTERNAL_LOG);
        }
        //LOG

        
        found.clear();
        chord_action action = DO_REPLY;
        node_address_latency foundFinger, foundFinger2;

        //Check curr node
        if (searchedID == m_nodeAddr.id) {
            action = DO_REPLY;
            foundFinger.setCopy(m_nodeAddr);
            found = foundFinger.toNodeAddress();
            
            //LOG
            m_ssLog << state2str(m_state) << LOG_TAB << "findSuccessor " << action2str(action) << " sID " << searchedID.to_string(SC_DEC) << " = node " << m_nodeAddr.id.to_string(SC_DEC) << " => node " << foundFinger.toStrIDonly();
            msgLog(name(), LOG_RX, LOG_INFO, m_ssLog.str(), DEBUG_LOG | INTERNAL_LOG);
        }

        //Check predecessor
        else if ((searchedID == m_predecessor.id) && (!m_predecessor.isNone())) {
            action = DO_REPLY;
            foundFinger.setCopy(m_predecessor);
            found = foundFinger.toNodeAddress();

            //LOG
            m_ssLog << state2str(m_state) << LOG_TAB << "findSuccessor " << action2str(action) << " sID " << searchedID.to_string(SC_DEC) << " = pred  " << m_predecessor.id.to_string(SC_DEC) << " => pred " << foundFinger.toStrIDonly();
            msgLog(name(), LOG_RX, LOG_INFO, m_ssLog.str(), DEBUG_LOG | INTERNAL_LOG);
        }
        
        //Check behind curr node
        else if ((isInRangeOverZero(searchedID, m_predecessor.id, m_nodeAddr.id)) && (!m_predecessor.isNone())) {
            action = DO_REPLY;
            foundFinger.setCopy(m_nodeAddr);                        
            found = foundFinger.toNodeAddress();
                                 
            //LOG
            m_ssLog << state2str(m_state) << LOG_TAB << "findSuccessor " << action2str(action) << " sID " << searchedID.to_string(SC_DEC) << " = [pred  " << m_predecessor.id.to_string(SC_DEC) << "; node " << m_nodeAddr.id.to_string(SC_DEC) << "] => node " << foundFinger.toStrIDonly();
            msgLog(name(), LOG_RX, LOG_INFO, m_ssLog.str(), DEBUG_LOG | INTERNAL_LOG);
        }



        //Check forward curr node
        else if ((isInRangeOverZero(searchedID, m_nodeAddr.id, m_successor.id)) && (!m_successor.isNone())) {
            action = DO_REPLY;

            foundFinger.setCopy(m_successor);
            found = foundFinger.toNodeAddress();

            //LOG
            m_ssLog << state2str(m_state) << LOG_TAB << "findSuccessor " << action2str(action) << " sID " << searchedID.to_string(SC_DEC) << " = [node  " << m_nodeAddr.id.to_string(SC_DEC) << "; succ " << m_successor.id.to_string(SC_DEC) << "] => succ " << foundFinger.toStrIDonly();
            msgLog(name(), LOG_RX, LOG_INFO, m_ssLog.str(), DEBUG_LOG | INTERNAL_LOG);
        }

        //Check fingers
        else 
        {
            //closest preceding  1
            for (auto fingerIt = m_cwFingers.rbegin(); fingerIt != m_cwFingers.rend(); ++fingerIt) {
                
                bool biggerThanLimit = searchedID >= fingerIt->id;
                bool notInitiator = fingerIt->id != initiatorID;
                bool notSender = fingerIt->id != senderID;
                bool inRangeFromTo = isInRangeOverZero(searchedID, fingerIt->from, fingerIt->to);
                bool closestToTarget = isInRangeOverZero(fingerIt->id, m_nodeAddr.id, searchedID);
                
                if ((closestToTarget) && (notInitiator) && (notSender) && (fingerIt->id != m_nodeAddr.id))
                {                                                            
                    foundFinger.setCopy(*fingerIt);                                     

                    if (fingerIt+1 != m_cwFingers.rend()) {
                        if (foundFinger.latency > (fingerIt+1)->latency) {
                            foundFinger.setCopy(*(fingerIt+1));
                        }
                    }
                    break;
                }
            }
            
            found = foundFinger.toNodeAddress();
           
            
            //We tried to find good finger, now the next steps will be
            if (found.isNone()) {
                
                find1stFingerNotForbidden(m_cwFingers, {initiatorID, senderID, m_nodeAddr.id}, foundFinger);      //Try to send to successor via ring
                if (foundFinger.isNone()) {   
                
                    //Try to send back to predecessor, because above we have already checked successor without success
                    if ((m_predecessor.id != senderID) && (m_predecessor.id != initiatorID) && (m_predecessor.id != m_nodeAddr.id)) {
                        foundFinger.setCopy(m_predecessor);
                        found = foundFinger.toNodeAddress();
                        action = DO_FORWARD;

                        //LOG
                        m_ssLog << state2str(m_state) << LOG_TAB << "findSuccessor " << action2str(action) << " sID " << searchedID.to_string(SC_DEC) << " = Not found finger => go to predecessor back " << found.toStrIDonly();
                        msgLog(name(), LOG_RX, LOG_INFO, m_ssLog.str(), DEBUG_LOG | INTERNAL_LOG);
                    }
                    else {
                        //Go home
                        foundFinger.setCopy(m_nodeAddr);                    
                        found = foundFinger.toNodeAddress();
                        action = DO_REPLY;

                        //LOG
                        m_ssLog << state2str(m_state) << LOG_TAB << "findSuccessor " << action2str(action) << " sID " << searchedID.to_string(SC_DEC) << " = Not found finger => node " << found.toStrIDonly();
                        msgLog(name(), LOG_RX, LOG_INFO, m_ssLog.str(), DEBUG_LOG | INTERNAL_LOG);
                    }
                }
                else { //Go through the ring
                    found = foundFinger.toNodeAddress();
                    action = DO_FORWARD;

                    //LOG
                    m_ssLog << state2str(m_state) << LOG_TAB << "findSuccessor " << action2str(action) << " sID " << searchedID.to_string(SC_DEC) << " = Not found finger => go to succ ring " << found.toStrIDonly();
                    msgLog(name(), LOG_RX, LOG_INFO, m_ssLog.str(), DEBUG_LOG | INTERNAL_LOG);
                }
                
            }
            else if ((found.id == senderID) || (found.id == initiatorID) || (found.id == m_nodeAddr.id)) {
                node_address_latency closerToTarget; 
                findAlittleBigFinger(searchedID, m_cwFingers, {senderID, initiatorID, m_nodeAddr.id}, closerToTarget);

                if (closerToTarget.isNone()) {                    
                    action = DO_REPLY;

                    //LOG
                    m_ssLog << state2str(m_state) << LOG_TAB << "findSuccessor " << action2str(action) << " sID " << searchedID.to_string(SC_DEC) << " >= cwfinger" << foundFinger.fingerIndex << " " << foundFinger.toStrIDonly();
                    msgLog(name(), LOG_RX, LOG_INFO, m_ssLog.str(), DEBUG_LOG | INTERNAL_LOG);
                }
                else {                   
                    foundFinger.setCopy(closerToTarget);
                    found = foundFinger.toNodeAddress();
                    action = DO_FORWARD;

                    //LOG
                    m_ssLog << state2str(m_state) << LOG_TAB << "findSuccessor " << action2str(action) << " sID " << searchedID.to_string(SC_DEC) << " => go via ring closer " << foundFinger.fingerIndex << " " << foundFinger.toStrIDonly();
                    msgLog(name(), LOG_RX, LOG_INFO, m_ssLog.str(), DEBUG_LOG | INTERNAL_LOG);
                }
                
            }
            else {            
                action = DO_FORWARD;

                //LOG
                m_ssLog << state2str(m_state) << LOG_TAB << "findSuccessor " << action2str(action) << " sID " << searchedID.to_string(SC_DEC) << " => cwfinger" << foundFinger.fingerIndex << " " << foundFinger.toStrIDonly();
                msgLog(name(), LOG_RX, LOG_INFO, m_ssLog.str(), DEBUG_LOG | INTERNAL_LOG);
            }            
        } 
        
        if ((isJoin) && (action == DO_REPLY) && (foundFinger.id != m_nodeAddr.id)) {

            action = DO_FORWARD;
            //LOG                     
            m_ssLog << state2str(m_state) << LOG_TAB << "findSuccessor " << action2str(action) << " sID " << searchedID.to_string(SC_DEC) << " => " << foundFinger.toStrIDonly();
            msgLog(name(), LOG_RX, LOG_INFO, m_ssLog.str(), DEBUG_LOG | INTERNAL_LOG);            
        }      

        return action;
    }


    chord_action low_latency_chord::findPredecessor(const uint160& searchedID, node_address& found) {
        found.clear();
        found = m_predecessor;
        if ((m_predecessor.isNone())) {
            found = m_nodeAddr;
        }
        
        return DO_REPLY;
    }


    string low_latency_chord::state2str(const finite_state& state) const {
        string res;
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


    void low_latency_chord::pushNewTimer(const chord_timer_message& timer, const bool toBack) {
        pushNewMessage(timer, toBack);
    }


    void low_latency_chord::pushNewTimer(const chord_timer_message_type type, const uint retryCounter, const uint requestCounter, const chord_byte_message_fields& retryMess, const bool toBack) {
        chord_message timer;
        timer.type = type;
        timer.retryCounter = retryCounter;
        timer.requestCounter = requestCounter;
        timer.issuedState = m_state;

        switch (type)
        {
        case CHORD_TIMER_RX_ACK: 
        case CHORD_TIMER_RX_SUCCESSOR_ON_JOIN:
        case CHORD_TIMER_RX_SUCCESSOR:
        case CHORD_TIMER_RX_PREDECESSOR:
            timer.retryMess = retryMess;
                            
        case CHORD_TIMER_UPDATE:
            break;

        default:   
            //ERROR
            msgLog(name(), LOG_RX, LOG_ERROR, state2str(m_state) + LOG_TAB + string("pushNewTimer") + LOG_SPACE + LOG_ERROR_NOT_RECOGNIZED, ALL_LOG);
            return;
        }

        pushNewMessage(timer, toBack);
    }

    chord_message low_latency_chord::createMessage(const chord_message& params) {
        chord_message newMess;
        newMess.clear();

        //switch (params)
        return newMess;
    }
    
    void low_latency_chord::removeTimer(buffer_container::iterator timerIt) {  
        if ((m_timerBufferIndex >= 0) && (m_timerBufferIndex < m_buffer.size())) {
            if ((timerIt->type > MIN_CHORD_TIMER_TYPE) && (timerIt->type < MAX_CHORD_TIMER_TYPE)) {
                if (true == m_buffer[m_timerBufferIndex].eraseMess(timerIt))
                    msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("remove ") + (*timerIt).toStr(), DEBUG_LOG | INTERNAL_LOG);
            }
        }
    }
    
    void low_latency_chord::removeTimer(const chord_timer_message_type timerType, const chord_tx_message_type retryMessType, const uint retryMessID) {
        if ((m_timerBufferIndex >= 0) && (m_timerBufferIndex < m_buffer.size())) {
            if ((timerType > MIN_CHORD_TIMER_TYPE) && (timerType < MAX_CHORD_TIMER_TYPE)) {
                if (true == m_buffer[m_timerBufferIndex].eraseMess(timerType, retryMessType, retryMessID))
                    msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("remove ") + chord_timer_message().type2str(timerType) + string(" mID ") + to_string(retryMessID), DEBUG_LOG | INTERNAL_LOG);
            }
        }
    }

    void low_latency_chord::removeTimers(const vector<chord_timer_message_type>& timerTypes, const vector<chord_tx_message_type>& retryMessTypes, const uint retryMessID) {
        vector<uint> messageIDs(1, retryMessID);
        
        if ((m_timerBufferIndex >= 0) && (m_timerBufferIndex < m_buffer.size())) {
            if (true == m_buffer[m_timerBufferIndex].eraseAllMess((vector<uint>&)timerTypes, (vector<uint>&)retryMessTypes, messageIDs)) {

                m_logText.clear();
                for (uint i = 0; i < timerTypes.size(); ++i)
                    m_logText += chord_timer_message().type2str(timerTypes[i]) + LOG_SPACE;
                msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("remove ") + m_logText + string(" mID ") + to_string(retryMessID), DEBUG_LOG | INTERNAL_LOG);
            }
        }
    }

    chord_message low_latency_chord::createJoinMessage(const node_address& dest) {
        chord_message newMess;
        newMess.clear();
        newMess.destNetwAddrs.push_back(network_address(dest.ip, dest.inSocket));
        newMess.destNodeIDwithSocket = dest;
        newMess.srcNodeIDwithSocket = m_nodeAddr;
        newMess.initiatorNodeIDwithSocket = m_nodeAddr;
        newMess.searchedNodeIDwithSocket = m_nodeAddr;
        newMess.searchedNodeIDwithSocket.id += 1;
        newMess.messageID = m_messageID;
        newMess.type = CHORD_TX_JOIN;
        newMess.flags.bitMessType = CHORD_BYTE_JOIN;
        newMess.flags.needsACK = m_confParams.needsACK ? NEEDS_ACK : NO_ACK;
        newMess.isJoin = true;
        m_messageID = nextUniqueMessageID();
        return newMess;
    }

    chord_message low_latency_chord::createNotifyMessage(const node_address& dest) {
        chord_message newMess;
        newMess.clear();
        newMess.destNetwAddrs.push_back(network_address(dest.ip, dest.inSocket));
        newMess.destNodeIDwithSocket = dest;
        newMess.srcNodeIDwithSocket = m_nodeAddr;
        newMess.initiatorNodeIDwithSocket = m_nodeAddr;
        newMess.searchedNodeIDwithSocket = m_nodeAddr;
        newMess.messageID = m_messageID;
        newMess.type = CHORD_TX_NOTIFY;
        newMess.flags.bitMessType = CHORD_BYTE_NOTIFY;
        newMess.flags.needsACK = m_confParams.needsACK ? NEEDS_ACK : NO_ACK;
        m_messageID = nextUniqueMessageID();
        return newMess;
    }

    chord_message low_latency_chord::createAckMessage(const node_address& dest, const uint messageID) {
        chord_message newMess;
        newMess.clear();
        newMess.destNetwAddrs.push_back(network_address(dest.ip, dest.inSocket));
        newMess.destNodeIDwithSocket = dest;
        newMess.srcNodeIDwithSocket = m_nodeAddr;
        newMess.initiatorNodeIDwithSocket = m_nodeAddr;
        newMess.type = CHORD_TX_ACK;
        newMess.messageID = messageID;
        newMess.flags.bitMessType = CHORD_BYTE_ACK;
        newMess.flags.needsACK = NO_ACK;
        return newMess;
    }

    chord_message low_latency_chord::createFindSuccessorMessage(const node_address& dest, const node_address& whoInitiator, const uint160& whatID, const bool isJoin, const int retransmitCounter, const int initialMessageID) {
        chord_message newMess;
        newMess.clear();
        newMess.destNetwAddrs.push_back(network_address(dest.ip, dest.inSocket));
        newMess.destNodeIDwithSocket = dest;
        newMess.srcNodeIDwithSocket = m_nodeAddr;
        newMess.initiatorNodeIDwithSocket = whoInitiator;
        newMess.type = CHORD_TX_FIND_SUCCESSOR;
        
        newMess.searchedNodeIDwithSocket.id = whatID;
        newMess.flags.bitMessType = CHORD_BYTE_FIND_SUCCESSOR;
        newMess.flags.needsACK = m_confParams.needsACK ? NEEDS_ACK : NO_ACK;        
        if (initialMessageID == NONE) {
            newMess.messageID = m_messageID;
            m_messageID = nextUniqueMessageID();
        }
        else 
            newMess.messageID = initialMessageID;
        newMess.isJoin = isJoin; 
        newMess.retransmitCounter = retransmitCounter;
        return newMess;
    }

    chord_message low_latency_chord::createSuccessorMessage(const node_address& dest, const uint messageID, const node_address& fingerAddr) {
        chord_message newMess;
        newMess.clear();
        newMess.destNetwAddrs.push_back(network_address(dest.ip, dest.inSocket));
        newMess.destNodeIDwithSocket = dest;
        newMess.srcNodeIDwithSocket = m_nodeAddr;
        newMess.initiatorNodeIDwithSocket = m_nodeAddr;
        newMess.type = CHORD_TX_SUCCESSOR;
        newMess.messageID = messageID;
        newMess.searchedNodeIDwithSocket = fingerAddr;
        newMess.flags.bitMessType = CHORD_BYTE_SUCCESSOR;
        newMess.flags.needsACK = NO_ACK;        
        newMess.serviceAddr = m_predecessor;
        return newMess;
    }

    chord_message low_latency_chord::createFindPredecessorMessage(const node_address& dest, const node_address& whoInitiator, const uint160& whatID) {
        chord_message newMess;
        newMess.clear();
        newMess.destNetwAddrs.push_back(network_address(dest.ip, dest.inSocket));
        newMess.destNodeIDwithSocket = dest;
        newMess.srcNodeIDwithSocket = m_nodeAddr;
        newMess.initiatorNodeIDwithSocket = whoInitiator;
        newMess.type = CHORD_TX_FIND_PREDECESSOR;
        newMess.messageID = m_messageID;
        newMess.searchedNodeIDwithSocket.id = whatID;
        newMess.flags.bitMessType = CHORD_BYTE_FIND_PREDECESSOR;
        newMess.flags.needsACK = m_confParams.needsACK ? NEEDS_ACK : NO_ACK;
        m_messageID = nextUniqueMessageID();
        return newMess;
    }

    chord_message low_latency_chord::createPredecessorMessage(const node_address& dest, const uint messageID, const node_address& fingerAddr) {
        chord_message newMess;
        newMess.clear();
        newMess.destNetwAddrs.push_back(network_address(dest.ip, dest.inSocket));
        newMess.destNodeIDwithSocket = dest;
        newMess.srcNodeIDwithSocket = m_nodeAddr;
        newMess.initiatorNodeIDwithSocket = m_nodeAddr;
        newMess.type = CHORD_TX_PREDECESSOR;
        newMess.messageID = messageID;
        newMess.searchedNodeIDwithSocket = fingerAddr;
        newMess.flags.bitMessType = CHORD_BYTE_PREDECESSOR;
        newMess.flags.needsACK = NO_ACK;
        return newMess;
    }

    chord_message low_latency_chord::createSingleMessage(const node_address& dest) {
        chord_message newMess;
        m_messageID = nextUniqueMessageID();
        return newMess;
    }

    buffer_container::iterator low_latency_chord::findMessageOnTimersWithRetryParams(bool& exist, const chord_timer_message_type timerType, const vector<chord_tx_message_type>& possibleRetryMessTypes, const uint messageID) {
        buffer_container::iterator it;
        
        if ((m_timerBufferIndex >= 0) && (m_timerBufferIndex < m_buffer.size()))
            it = m_buffer[m_timerBufferIndex].find1Mess(exist, (uint) timerType, (vector<uint>&) possibleRetryMessTypes, messageID);
        return it;
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


    int low_latency_chord::checkMessage(const chord_message& mess, chord_timer_message& timer, const string& errCode) {
        buffer_container::iterator timerIt;
        int res = checkMessage(mess, timerIt, errCode);        
        if (res != ERROR)
            timer = *timerIt;
        return res;
    }


    event_type low_latency_chord::eventType(const chord_message& mess, const bool existMess) {                
        if (existMess == false) 
            return CALLED_BY_ANOTHER_STATE;

        if (mess.isDelayed)
            return INACCESSIBLE_NODE;

        if ((MIN_CHORD_RX_TYPE < mess.type) && (mess.type < MAX_CHORD_RX_TYPE))
            return RX_MESS_RECEIVED; 
                                     
        if ((MIN_CHORD_TX_TYPE < mess.type) && (mess.type < MAX_CHORD_TX_TYPE))
            return TX_MESS_SHOULD_SEND;

        if ((MIN_CHORD_TIMER_TYPE < mess.type) && (mess.type < MAX_CHORD_TIMER_TYPE))
            return TIMER_EXPIRED;

        if ((MIN_CHORD_APPTX_TYPE < mess.type) && (mess.type < MAX_CHORD_APPTX_TYPE))
            return APPTXREQUEST;

        //ERROR
        msgLog(name(), LOG_TXRX, LOG_ERROR, state2str(m_state) + LOG_TAB + string("eventType") + LOG_SPACE + LOG_ERROR_NOT_RECOGNIZED, DEBUG_LOG | INTERNAL_LOG);
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
        return (m_nodeAddr.id.to_uint() << 16) + randValue;
    }


    bool low_latency_chord::issueMessagePushTimers(const chord_tx_message_type& type, const bool isRetry, const uint requestCounter,
                                                   const chord_byte_message_fields rxMess, const chord_timer_message expiredTimer,
                                                   const chord_action action, const node_address lookupAddr) {
        chord_message newMess;

        switch (type) {
            case CHORD_TX_JOIN: {
                if ((isRetry == false) /* || ((isRetry == true) && (expiredTimer.retryCounter >= m_confParams.CtxRetry) && (m_confParams.needsACK == NEEDS_ACK))*/) {
                    m_currSeed = (m_currSeed + 1) % (uint)(m_confParams.seed.size());

                    newMess = createJoinMessage(m_confParams.seed.at(m_currSeed));

                    msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("issueMessagePushTimers ") + newMess.chord_byte_message_fields::type2str() + string(" mID ") + to_string(newMess.messageID) + string(" retryC ") + to_string(0) + string("(") + to_string(m_confParams.CtxRetry) + string(")"), DEBUG_LOG | INTERNAL_LOG);

                    pushNewMessage(newMess);
                    if (m_confParams.TrxSuccOnJoin != NO_TIMEOUT)
                        pushNewTimer(CHORD_TIMER_RX_SUCCESSOR_ON_JOIN, 0, requestCounter, newMess);
                    if ((m_confParams.TrxAck != NO_TIMEOUT) && (m_confParams.needsACK == NEEDS_ACK))
                        pushNewTimer(CHORD_TIMER_RX_ACK, 0, requestCounter, newMess);
                }
                else {
                    if (expiredTimer.retryCounter < m_confParams.CtxRetry) {                        
                        newMess = createJoinMessage(m_confParams.seed.at(m_currSeed));
                        
                        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("issueMessagePushTimers ") + newMess.chord_byte_message_fields::type2str() + string(" mID ") + to_string(newMess.messageID) + string(" retryC ") + to_string(expiredTimer.retryCounter + 1) + string("(") + to_string(m_confParams.CtxRetry) + string(")"), DEBUG_LOG | INTERNAL_LOG);

                        pushNewMessage(newMess);
                        if (m_confParams.TrxSuccOnJoin != NO_TIMEOUT)
                            pushNewTimer(CHORD_TIMER_RX_SUCCESSOR_ON_JOIN, expiredTimer.retryCounter, requestCounter, newMess);
                        if ((m_confParams.TrxAck != NO_TIMEOUT) && (m_confParams.needsACK == NEEDS_ACK))
                            pushNewTimer(CHORD_TIMER_RX_ACK, expiredTimer.retryCounter+1, requestCounter, newMess);  
                    }
                    else
                        return false;   //No possible to retry tx_join
                }       
                return true;            //First transmisstion and retry further
            }
            break;

            case CHORD_TX_FIND_SUCCESSOR: {     
                
                uint currFinger;
                node_address_latency fingerPast;
                node_address_latency finger;
                uint160 whatId = m_nodeAddr.id;
                if (action == DO_REQUEST) {
                    //fingerPast = findPrevAliveFinger(m_isClockWise,0);



                    if (m_isClockWise) {
                        currFinger = m_cwFingerIndex;

                        if (currFinger >= m_cwFingers.size()) {
                            //ERROR
                            msgLog(name(), LOG_TXRX, LOG_ERROR, state2str(m_state) + LOG_TAB + string("issueMessagePushTimers currFinger >= cwFingers.size() ") + LOG_ERROR_INVALID_RANGE, DEBUG_LOG | INTERNAL_LOG);
                            return false;
                        }


                            
                        finger.setCopy(m_cwFingers[currFinger]);
                        whatId = whatId+m_fingerMask[currFinger];

                        if ((m_cwFingerIndex == 0)) {
                            fingerPast.setCopy(m_cwFingers.at(0));
                                                           
                            //if (m_currSeed < m_seedAddrs.size()) {
                            //    node_address_latency addr;        
                            //    addr.set(m_seedAddrs.at(m_currSeed));
                            //    fingerPast.setCopy(addr);
                            //}
                        }
                        else {
                            int i = -1;
                            for (i = (int)currFinger-1; i >= 0; --i) {
                                if (m_cwFingers.at(i).id != m_nodeAddr.id)
                                    break;
                            }
                            if (i >= 0) 
                                fingerPast.setCopy(m_cwFingers.at(i));
                            else {
                                node_address_latency addr;
                                addr.set(m_seedAddrs.at(m_currSeed));
                                fingerPast.setCopy(addr);
                            }
                        }                                                           
                    }
                    else {
                        currFinger = m_ccwFingerIndex;

                        if (currFinger >= m_ccwFingers.size()) {
                            //ERROR
                            msgLog(name(), LOG_TXRX, LOG_ERROR, state2str(m_state) + LOG_TAB + string("issueMessagePushTimers currFinger >= ccwFingers.size() ") + LOG_ERROR_INVALID_RANGE, DEBUG_LOG | INTERNAL_LOG);
                            return false;
                        }

                        finger.setCopy(m_ccwFingers[currFinger]);
                        whatId = whatId-m_fingerMask[currFinger];                    

                        if (m_ccwFingerIndex == 0) {                        
                            fingerPast.setCopy(m_ccwFingers.at(0));

                            //if (m_currSeed < m_seedAddrs.size()) {
                            //    node_address_latency addr;
                            //    addr.set(m_seedAddrs.at(m_currSeed));
                            //    fingerPast.setCopy(addr);
                            //}
                        }
                        else {                        
                            int i = -1;
                            for (i = (int)currFinger - 1; i >= 0; --i) {
                                if (m_ccwFingers.at(i).id != m_nodeAddr.id)
                                    break;
                            }
                            if (i > 0)
                                fingerPast.setCopy(m_ccwFingers.at(i));
                            else {
                                node_address_latency addr;
                                addr.set(m_seedAddrs.at(m_currSeed));
                                fingerPast.setCopy(addr);
                            }
                        }                                        
                    }
                }
            
                if (isRetry == false) {
                    if (action == DO_FORWARD) {
                        newMess = createFindSuccessorMessage(lookupAddr, rxMess.initiatorNodeIDwithSocket, rxMess.searchedNodeIDwithSocket.id, rxMess.isJoin, rxMess.retransmitCounter+1, rxMess.messageID);
                    }
                    else if (action == DO_REQUEST) {                        
                        newMess = createFindSuccessorMessage(fingerPast, m_nodeAddr, whatId, false, 0);
                    }

                    msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("issueMessagePushTimers ") + (action == DO_FORWARD ? string("FWD_") : string("")) + newMess.chord_byte_message_fields::type2str() + string(" mID ") +
                        to_string(newMess.messageID) + string(" retryC ") + to_string(expiredTimer.retryCounter + 1) + string("(") + to_string(m_confParams.CtxRetry) + string(")"), DEBUG_LOG | INTERNAL_LOG);
                    
                    
                    if (newMess.destNodeIDwithSocket.id == m_nodeAddr.id) {
                        
                        msgLog(name(), LOG_TXRX, LOG_WARNING, state2str(m_state) + LOG_TAB + string("issueMessagePushTimers ") + newMess.chord_byte_message_fields::type2str() +
                            string(" d ") + node_address(fingerPast).toStr() + string(" dest addr = myAddr ") + node_address(m_nodeAddr).toStr() + string(" CANCELLED"), DEBUG_LOG | INTERNAL_LOG);
                        
                        if (m_isNowUpdate)
                            m_isNowUpdate = false;
                        return false;
                    }


                    pushNewMessage(newMess);
                    if ((m_confParams.TrxSucc != NO_TIMEOUT) && (action == DO_REQUEST))
                        pushNewTimer(CHORD_TIMER_RX_SUCCESSOR, 0, requestCounter, newMess);
                    if ((m_confParams.TrxAck != NO_TIMEOUT) && (m_confParams.needsACK == NEEDS_ACK))
                        pushNewTimer(CHORD_TIMER_RX_ACK, 0, requestCounter, newMess);
                    return true;          //First try
                }
                else {
                    if ((expiredTimer.retryCounter < m_confParams.CtxRetry) && (m_confParams.needsACK == NEEDS_ACK)) {
                        if (action == DO_FORWARD) {
                            newMess = createFindSuccessorMessage(lookupAddr, rxMess.initiatorNodeIDwithSocket, rxMess.searchedNodeIDwithSocket.id, rxMess.isJoin, rxMess.retransmitCounter+1, rxMess.messageID);
                        }                        
                        else if (action == DO_REQUEST) {                            
                            newMess = createFindSuccessorMessage(fingerPast, m_nodeAddr, m_nodeAddr.id + m_fingerMask[currFinger], false, 0);                            
                        }

                        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("issueMessagePushTimers ") + (action == DO_FORWARD ? string("FWD_") : string("")) + newMess.chord_byte_message_fields::type2str() + string(" mID ") +
                            to_string(newMess.messageID) + string(" retryC ") + to_string(expiredTimer.retryCounter + 1) + string("(") + to_string(m_confParams.CtxRetry) + string(")"), DEBUG_LOG | INTERNAL_LOG);


                        if (newMess.destNodeIDwithSocket.id == m_nodeAddr.id) {
                            msgLog(name(), LOG_TXRX, LOG_WARNING, state2str(m_state) + LOG_TAB + string("issueMessagePushTimers ") + newMess.chord_byte_message_fields::type2str() +
                                string(" d ") + node_address(fingerPast).toStr() + string(" dest addr = myAddr ") + node_address(m_nodeAddr).toStr() + string(" CANCELLED"), DEBUG_LOG | INTERNAL_LOG);

                            if (m_isNowUpdate)
                                m_isNowUpdate = false;
                            return false;
                        }

                        //if ((m_isNowUpdate) && (action == DO_REQUEST))
                        //     m_isNowUpdate = false;                        

                        pushNewMessage(newMess);
                        if ((m_confParams.TrxSucc != NO_TIMEOUT) && (action == DO_REQUEST))
                            pushNewTimer(CHORD_TIMER_RX_SUCCESSOR, expiredTimer.retryCounter, requestCounter, newMess);
                        if ((m_confParams.TrxAck != NO_TIMEOUT) && (m_confParams.needsACK == NEEDS_ACK))
                            pushNewTimer(CHORD_TIMER_RX_ACK, expiredTimer.retryCounter+1, requestCounter, newMess);
                        return true;      //Try retry
                    }
                    else {                   
                        return false;     //No possible to retry find_successor on this finger
                    }             
                }
            }   
            break;

            case CHORD_TX_FIND_PREDECESSOR: {

                if (isRetry == false) {                   
                    newMess = createFindPredecessorMessage(m_successor, m_nodeAddr, m_successor.id-1);

                    msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("issueMessagePushTimers ") + newMess.chord_byte_message_fields::type2str() + string(" mID ") + to_string(newMess.messageID) +
                           string(" retryC ") + to_string(0) + string("(") + to_string(m_confParams.CtxRetry) + string(")"), DEBUG_LOG | INTERNAL_LOG);

                    pushNewMessage(newMess);
                    if ((m_confParams.TrxPred != NO_TIMEOUT) && (action == DO_REQUEST))
                        pushNewTimer(CHORD_TIMER_RX_PREDECESSOR, 0, requestCounter, newMess);
                    if ((m_confParams.TrxAck != NO_TIMEOUT) && (m_confParams.needsACK == NEEDS_ACK))
                        pushNewTimer(CHORD_TIMER_RX_ACK, 0, requestCounter, newMess);
                    return true;          //First try
                }
                else {
                    if ((expiredTimer.retryCounter < m_confParams.CtxRetry) && (m_confParams.needsACK == NEEDS_ACK)) {
                        m_isClockWise = true;
                        //m_ccwFingerIndex = 0;
                        newMess = createFindPredecessorMessage(m_successor, m_nodeAddr, m_successor.id-1);

                        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("issueMessagePushTimers ") + newMess.chord_byte_message_fields::type2str() + string(" mID ") + to_string(newMess.messageID) + string(" retryC ") + to_string(expiredTimer.retryCounter + 1) + string("(") + to_string(m_confParams.CtxRetry) + string(")"), DEBUG_LOG | INTERNAL_LOG);

                        pushNewMessage(newMess);
                        if ((m_confParams.TrxPred != NO_TIMEOUT) && (action == DO_REQUEST))
                            pushNewTimer(CHORD_TIMER_RX_PREDECESSOR, expiredTimer.retryCounter, requestCounter, newMess);
                        if ((m_confParams.TrxAck != NO_TIMEOUT) && (m_confParams.needsACK == NEEDS_ACK))
                            pushNewTimer(CHORD_TIMER_RX_ACK, expiredTimer.retryCounter+1, requestCounter, newMess);
                        return true;      //Try retry
                    }
                    else {
                        return false;     //No possible to retry find_predecessor
                    }
                }
            }
            break;


            case CHORD_TX_NOTIFY: {
                if (isRetry == false) {
                    newMess = createNotifyMessage(m_successor);

                    msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("issueMessagePushTimers ") + newMess.chord_byte_message_fields::type2str() + string(" mID ") + to_string(newMess.messageID) + string(" retryC ") + to_string(0) + string("(") + to_string(m_confParams.CtxRetry) + string(")"), DEBUG_LOG | INTERNAL_LOG);

                    pushNewMessage(newMess);
                    if ((m_confParams.TrxAck != NO_TIMEOUT) && (m_confParams.needsACK == NEEDS_ACK))
                        pushNewTimer(CHORD_TIMER_RX_ACK, 0, requestCounter, newMess);
                    return true;
                }
                else {
                    if ((expiredTimer.retryCounter < m_confParams.CtxRetry) && (m_confParams.needsACK == NEEDS_ACK)) {
                        newMess = createNotifyMessage(m_successor);

                        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("issueMessagePushTimers ") + newMess.chord_byte_message_fields::type2str() + string(" mID ") + to_string(newMess.messageID) + string(" retryC ") + to_string(0) + string("(") + to_string(m_confParams.CtxRetry) + string(")"), DEBUG_LOG | INTERNAL_LOG);

                        pushNewMessage(newMess);
                        if ((m_confParams.TrxAck != NO_TIMEOUT) && (m_confParams.needsACK == NEEDS_ACK))
                            pushNewTimer(CHORD_TIMER_RX_ACK, 0, requestCounter, newMess);
                        return true;    //Try retry
                    }
                    else
                        return false;   //No possible to retry find_predecessor
                }
            }
            break;

            case CHORD_TX_ACK: {
                if (rxMess.flags.needsACK == NEEDS_ACK) {
                    newMess = createAckMessage(rxMess.srcNodeIDwithSocket, rxMess.messageID);
                    pushNewMessage(newMess);
                }      
                return true;
            }
            break;

            case CHORD_TX_SUCCESSOR: { 
                newMess = createSuccessorMessage(rxMess.initiatorNodeIDwithSocket, rxMess.messageID, lookupAddr);
                pushNewMessage(newMess);   
                return true;
            }
            break;

            case CHORD_TX_PREDECESSOR: {
                newMess = createPredecessorMessage(rxMess.initiatorNodeIDwithSocket, rxMess.messageID, lookupAddr);
                pushNewMessage(newMess);
                return true;
            }
            break;

            default:
                //ERROR
                msgLog(name(), LOG_TXRX, LOG_ERROR, state2str(m_state) + LOG_TAB + string("issueMessagePushTimers") + LOG_SPACE + LOG_ERROR_NOT_RECOGNIZED, ALL_LOG);
        }
        return false;
    }


    bool low_latency_chord::setPredecessorThanSuccessor(const chord_byte_message_fields& rxMess, const chord_timer_message& timer, const string& motive) {

        if ((rxMess.searchedNodeIDwithSocket.id < m_nodeAddr.id) && (m_cwFingerIndex == 0)) {
            
            //oberon5962's modificated actions for valid Chord operations
            m_predecessor = rxMess.searchedNodeIDwithSocket;
            m_predecessor.isUpdated = true;
            m_predecessor.updateTime = sc_time_stamp();
            m_predecessor.motive = motive + LOG_SPACE + MOTIVE_PRED_SUCC;

            if (m_ccwFingers.size() > 0)
                m_ccwFingers[0] = m_predecessor;
            m_isPredecessorSet = true;

            //LOG
            m_ssLog << state2str(m_state) + LOG_TAB + string("setPredecessorThanSuccessor sID ") << rxMess.searchedNodeIDwithSocket.toStrIDonly() << " < " << m_nodeAddr.toStrIDonly() << " => predecessor ccwfinger0 " << rxMess.searchedNodeIDwithSocket.toStrIDonly();
            msgLog(name(), LOG_TXRX, LOG_INFO, m_ssLog.str(), DEBUG_LOG | INTERNAL_LOG);
            //LOG

            m_successor = timer.retryMess.destNodeIDwithSocket;
            m_successor.isUpdated = true;
            m_successor.updateTime = sc_time_stamp();
            m_successor.motive = motive + LOG_SPACE + MOTIVE_PRED_SUCC;
            
            if (m_cwFingers.size() > 0)
                m_cwFingers[0] = m_successor;
            m_isSuccessorSet = true;            

            //LOG
            m_ssLog << state2str(m_state) + LOG_TAB + string("setPredecessorThanSuccessor sID ") << rxMess.searchedNodeIDwithSocket.toStrIDonly() << " < " << m_nodeAddr.toStrIDonly() << " => successor ccfinger0 " << timer.retryMess.destNodeIDwithSocket.toStrIDonly();
            msgLog(name(), LOG_TXRX, LOG_INFO, m_ssLog.str(), DEBUG_LOG | INTERNAL_LOG);
            //LOG

            return true;
        }
        else if (rxMess.searchedNodeIDwithSocket.id > m_nodeAddr.id) {
            return false;
        }
        else { 
            if (rxMess.searchedNodeIDwithSocket.id == m_nodeAddr.id) {
                //ERROR
                m_ssLog << state2str(m_state) << LOG_TAB << "sID " << timer.retryMess.searchedNodeIDwithSocket.toStrIDonly() << " = node " << m_nodeAddr.toStrIDonly() << LOG_SPACE << LOG_ERROR_INVALID_SEARCHED_ID;
                msgLog(name(), LOG_TXRX, LOG_ERROR, m_ssLog.str(), ALL_LOG);                            
            }
            return false;
        }
    }


    bool low_latency_chord::setSuccessorRemoveTimers(const chord_byte_message_fields& rxMess, const chord_timer_message& timer) {
        bool needsFillFingersMinQty = false;
        if (m_isSuccessorSet == false) {
            //Set successor via classical Chord algorithm
            m_successor = rxMess.searchedNodeIDwithSocket;
            m_successor.isUpdated = true;
            m_successor.updateTime = sc_time_stamp();
            m_successor.motive = MOTIVE_JOIN;
            m_cwFingerIndex++;
            if (m_cwFingers.size() > 0)
                m_cwFingers[0] = m_successor;
            m_isSuccessorSet = true;

            //Remove timers        
            vector<chord_timer_message_type> timerTypes = {CHORD_TIMER_RX_ACK, CHORD_TIMER_RX_SUCCESSOR_ON_JOIN};
            vector<chord_tx_message_type> retryMessTypes = {CHORD_TX_JOIN};
            removeTimers(timerTypes, retryMessTypes, rxMess.messageID);                
        
            //Remove possible retry message
            if ((sc_time_stamp() >= timer.creatingTime) && (timer.retryMess.type == CHORD_TX_JOIN))
                eraseTxMess(CHORD_TX_JOIN);

            msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("setSuccessor successor cwfinger0 ") + m_successor.toStrIDonly(), DEBUG_LOG | INTERNAL_LOG);
            
            if ((m_cwFingerIndex < m_confParams.fillFingersMinQty) && (m_state == STATE_JOIN))
                needsFillFingersMinQty = true;
        }
        else {
            //Set successor via classical Chord algorithm
            m_successor = rxMess.searchedNodeIDwithSocket;
            m_successor.isUpdated = true;
            m_successor.updateTime = sc_time_stamp();
            m_successor.motive = MOTIVE_JOIN;
            m_cwFingerIndex++;
            if (m_cwFingers.size() > 0)
                m_cwFingers[0] = m_successor;
            m_isSuccessorSet = true;
            

            //Remove timers        
            vector<chord_timer_message_type> timerTypes = { CHORD_TIMER_RX_ACK, CHORD_TIMER_RX_SUCCESSOR };
            vector<chord_tx_message_type> retryMessTypes = { CHORD_TX_FIND_SUCCESSOR };
            removeTimers(timerTypes, retryMessTypes, rxMess.messageID);

            //Remove message
            //if (timer.retryMess.type == CHORD_TX_FIND_SUCCESSOR)
            if ((sc_time_stamp() >= timer.creatingTime) && (timer.retryMess.type == CHORD_TX_FIND_SUCCESSOR))
                eraseTxMess(CHORD_TX_FIND_SUCCESSOR);

            msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("setSuccessor successor cwfinger0 ") + m_successor.toStrIDonly(), DEBUG_LOG | INTERNAL_LOG);
            needsFillFingersMinQty = false;
        }   

        //tryAddNewSeed(m_successor);
        if (MAKE_SNAPSHOT_ALWAYS) makeSnapshot();
    
        if (m_inaccessibleFingers.size() > 0) {
            if (timer.retryMess.destNodeIDwithSocket.id == m_inaccessibleFingers.at(0).timer.retryMess.destNodeIDwithSocket.id) {
                msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("Restore timer ") + chord_timer_message().type2str(m_inaccessibleFingers[0].timer.type) + LOG_SPACE + state2str(m_inaccessibleFingers[0].timer.issuedState), DEBUG_LOG | INTERNAL_LOG);
                m_inaccessibleFingers[0].timer.isDelayed = false;
                pushNewTimer(m_inaccessibleFingers[0].timer, false);
                eraseFirstInaccessFingersIssueNewMessage();
            }
        }
        return needsFillFingersMinQty;
    }

    bool low_latency_chord::setPredecessor(const chord_byte_message_fields& rxMess, const chord_timer_message& timer, const bool onJoin) {
        bool isSuccess = false;
        m_predecessor.isUpdated = true;
        m_predecessor.updateTime = sc_time_stamp();        

        if ((m_predecessor.isNone() == true) || (m_predecessor.id == m_nodeAddr.id) || (isInRangeOverZeroNotInc(rxMess.searchedNodeIDwithSocket.id, m_predecessor.id, m_nodeAddr.id))) {
            m_predecessor = rxMess.searchedNodeIDwithSocket;
            m_predecessor.motive = MOTIVE_NOTIFY + LOG_SPACE + "mID " + to_string(rxMess.messageID);;
            m_ccwFingers[0] = m_predecessor;
            isSuccess = true;

            //tryAddNewSeed(m_predecessor);

            if (MAKE_SNAPSHOT_ALWAYS) makeSnapshot();
        }   

        if (isSuccess) {
            if (onJoin)
                msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("on Join setPredecessor new predecessor = ") + m_predecessor.toStrIDonly(), DEBUG_LOG | INTERNAL_LOG);
            else
                msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("Notify setPredecessor new predecessor = ") + m_predecessor.toStrIDonly(), DEBUG_LOG | INTERNAL_LOG);
        }
        else {
            stringstream ss;

            if (onJoin)
                ss << state2str(m_state) << LOG_TAB << ("on Join setPredecessor not changed: addr ") + rxMess.searchedNodeIDwithSocket.toStrIDonly() << " != (pred " << m_predecessor.toStrIDonly() << "; node " << m_nodeAddr.toStrIDonly() << ")";
            else
                ss << state2str(m_state) << LOG_TAB << ("Notify setPredecessor not changed: addr ") + rxMess.searchedNodeIDwithSocket.toStrIDonly() << " != (pred " << m_predecessor.toStrIDonly() << "; node " << m_nodeAddr.toStrIDonly() << ")";
            msgLog(name(), LOG_TXRX, LOG_INFO, ss.str().c_str(), DEBUG_LOG | INTERNAL_LOG);
        }

        if ((m_inaccessibleFingers.size() > 0) && (onJoin == false)) {
            if (timer.retryMess.destNodeIDwithSocket.id == m_inaccessibleFingers.at(0).timer.retryMess.destNodeIDwithSocket.id) {
                msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("Restore timer ") + chord_timer_message().type2str(m_inaccessibleFingers[0].timer.type) + LOG_SPACE + state2str(m_inaccessibleFingers[0].timer.issuedState), DEBUG_LOG | INTERNAL_LOG);
                m_inaccessibleFingers[0].timer.isDelayed = false;
                pushNewTimer(m_inaccessibleFingers[0].timer, false);
                eraseFirstInaccessFingersIssueNewMessage();
            }
        }

        return isSuccess;
    }





    bool low_latency_chord::setSuccessorStabilize(const chord_byte_message_fields& rxMess, const chord_timer_message& timer) {
        //m_isNowStabilize = false;
        bool isSuccess = false;
        m_successor.isUpdated = true;
        m_successor.updateTime = sc_time_stamp();                
        
        if ((rxMess.searchedNodeIDwithSocket.isNone() == false) && (isInRangeOverZeroNotInc(rxMess.searchedNodeIDwithSocket.id, m_nodeAddr.id, m_successor.id))) {
            m_successor = rxMess.searchedNodeIDwithSocket;
            m_successor.motive = MOTIVE_STABILIZE + LOG_SPACE + "mID " + to_string(rxMess.messageID);;
            m_cwFingers[0] = m_successor;
            isSuccess = true;
            //tryAddNewSeed(m_successor);
            if (MAKE_SNAPSHOT_ALWAYS) makeSnapshot();
        }
        m_isSuccessorSet = true;
        

        //Remove timers        
        vector<chord_timer_message_type> timerTypes = {CHORD_TIMER_RX_ACK, CHORD_TIMER_RX_PREDECESSOR};
        vector<chord_tx_message_type> retryMessTypes = {CHORD_TX_FIND_PREDECESSOR};
        removeTimers(timerTypes, retryMessTypes, rxMess.messageID);

        //Remove message        
        if ((sc_time_stamp() >= timer.creatingTime) && (timer.retryMess.type == CHORD_TX_FIND_PREDECESSOR))
            eraseTxMess(CHORD_TX_FIND_PREDECESSOR);

        if (isSuccess) 
            msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("Stabilize setSuccessor new successor = cwfinger0 ") + m_successor.toStrIDonly(), DEBUG_LOG | INTERNAL_LOG);
        else {
            stringstream ss;
            ss << state2str(m_state) << LOG_TAB << ("Stabilize setSuccessor not changed: addr ") << rxMess.searchedNodeIDwithSocket.toStrIDonly() + (" != (node ") << m_nodeAddr.toStrIDonly() << ("; succ ") << m_successor.toStrIDonly() << ")";
            msgLog(name(), LOG_TXRX, LOG_INFO, ss.str().c_str(), DEBUG_LOG | INTERNAL_LOG);
        }
        
        if (m_inaccessibleFingers.size() > 0) {
            if (timer.retryMess.destNodeIDwithSocket.id == m_inaccessibleFingers.at(0).timer.retryMess.destNodeIDwithSocket.id) {
                msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("Restore timer ") + chord_timer_message().type2str(m_inaccessibleFingers[0].timer.type) + LOG_SPACE + state2str(m_inaccessibleFingers[0].timer.issuedState), DEBUG_LOG | INTERNAL_LOG);
                m_inaccessibleFingers[0].timer.isDelayed = false;
                pushNewTimer(m_inaccessibleFingers[0].timer, false);
                eraseFirstInaccessFingersIssueNewMessage();
            }
        }
        
        return isSuccess;
    }


    bool low_latency_chord::forceUpdateFingerTable(const chord_message& rxMess) {
    return true;

        int i = ((int)m_cwFingers.size())-1;
        int prevI = i;
        

        bool isSuccess = false;
        uint160 ccwExpectedIDi, cwExpectedIDi;
        uint160 transitID = rxMess.initiatorNodeIDwithSocket.id;

        while (i >= 0) {
            cwExpectedIDi = m_nodeAddr.id + m_fingerMask.at(i);            
            bool isFingerDefaultANDtransitBiggerExpected = (m_cwFingers.at(i).id == m_nodeAddr.id) && (transitID >= cwExpectedIDi) && (transitID < m_nodeAddr.id);
            bool isFingerNotEqualedExpected = cwExpectedIDi != m_cwFingers.at(i).id;
            bool isTransitBiggerExpectedAndFingerLessExpectedANDexpectedNotFound = (transitID >= cwExpectedIDi) && (m_cwFingers[i].id < cwExpectedIDi) && (isFingerNotEqualedExpected);

            //New code
            bool needsForceUpdate = false;
            if (isFingerDefaultANDtransitBiggerExpected)
                needsForceUpdate = true;
            
            if (isTransitBiggerExpectedAndFingerLessExpectedANDexpectedNotFound)
                needsForceUpdate = true;

            if (needsForceUpdate)
            {
                //if (m_nodeAddr.id > transitID)
                {                    
                    m_ssLog << state2str(m_state) + LOG_TAB + string("forceUpdateFingerTable cwfinger") << i << LOG_SPACE << m_cwFingers.at(i).toStrIDonly() << " => " << rxMess.initiatorNodeIDwithSocket.toStrIDonly();
                    msgLog(name(), LOG_TXRX, LOG_INFO, m_ssLog.str(), DEBUG_LOG | INTERNAL_LOG);

                
                    m_cwFingers[i] = rxMess.initiatorNodeIDwithSocket;
                    m_cwFingers[i].isUpdated = true;
                    m_cwFingers[i].updateTime = sc_time_stamp();
                    m_cwFingers[i].motive = MOTIVE_FORCE_UPDATE;
                    if (i == 0) {
                        m_successor = m_cwFingers[i];
                    }
                    isSuccess = true;
                }
            }

            --i;            
        }

        return isSuccess;
    }



    void low_latency_chord::setFingerRemoveTimers(const chord_byte_message_fields& rxMess, const chord_timer_message& timer) {  

        node_address_latency updatedFinger;
        if (m_isClockWise) {
            if (m_cwFingerIndex >= m_cwFingers.size()) {
                cout << sc_time_stamp().to_string() << " !!!! Press enter" << endl;
                //ERROR
                msgLog(name(), LOG_TXRX, LOG_ERROR, state2str(m_state) + LOG_TAB + string("setFinger") + LOG_SPACE + LOG_ERROR_INVALID_RANGE, ALL_LOG);
                return;
            }
            
            //Set successor via classical Chord algorithm
            m_cwFingers[m_cwFingerIndex] = rxMess.searchedNodeIDwithSocket;
            m_cwFingers[m_cwFingerIndex].isUpdated = true;
            m_cwFingers[m_cwFingerIndex].updateTime = sc_time_stamp();
            m_cwFingers[m_cwFingerIndex].motive = MOTIVE_PERIOD_RET_SUCC + LOG_SPACE + "mID " + to_string(rxMess.messageID);
            if ((m_cwFingerIndex == 0) && (m_cwFingers[m_cwFingerIndex].id != m_nodeAddr.id)) {
                m_successor = m_cwFingers[m_cwFingerIndex];                    
            }                    
            
            updatedFinger.setCopy(m_cwFingers.at(m_cwFingerIndex));
        }
        else {
            if (m_ccwFingerIndex >= m_ccwFingers.size()) {
                cout << sc_time_stamp().to_string() << " !!!! Press enter" << endl;
                //ERROR
                msgLog(name(), LOG_TXRX, LOG_ERROR, state2str(m_state) + LOG_TAB + string("setFinger") + LOG_SPACE + LOG_ERROR_INVALID_RANGE, ALL_LOG);
                return;
            }
            
            m_ccwFingers[m_ccwFingerIndex] = rxMess.searchedNodeIDwithSocket;
            m_ccwFingers[m_ccwFingerIndex].isUpdated = true;
            m_ccwFingers[m_ccwFingerIndex].updateTime = sc_time_stamp();
            m_ccwFingers[m_ccwFingerIndex].motive = MOTIVE_PERIOD_RET_SUCC + LOG_SPACE + "mID " + to_string(rxMess.messageID);

            if ((m_ccwFingerIndex == 0) && (m_ccwFingers[m_ccwFingerIndex].id != m_nodeAddr.id)) {
                m_predecessor = m_ccwFingers[m_ccwFingerIndex];                
            }                        
            updatedFinger.setCopy(m_ccwFingers.at(m_ccwFingerIndex));
        } 
        
        if (MAKE_SNAPSHOT_ALWAYS) makeSnapshot();
       

        //Remove timers        
        vector<chord_timer_message_type> timerTypes = {CHORD_TIMER_RX_ACK, CHORD_TIMER_RX_SUCCESSOR};
        vector<chord_tx_message_type> retryMessTypes = {CHORD_TX_FIND_SUCCESSOR};
        removeTimers(timerTypes, retryMessTypes, rxMess.messageID);

        //Remove possible retry message
        if ((sc_time_stamp() >= timer.creatingTime) && (timer.retryMess.type == CHORD_TX_FIND_SUCCESSOR))
            eraseTxMess(CHORD_TX_FIND_SUCCESSOR);

        m_ssLog << state2str(m_state) << LOG_TAB << "setFinger " << (updatedFinger.isClockWise ? string("cwfinger") : string("ccwfinger"))
                << updatedFinger.fingerIndex << ((updatedFinger.fingerIndex == 0) && (updatedFinger.isClockWise) ? string(" successor") :
                   ((updatedFinger.fingerIndex == 0) && (updatedFinger.isClockWise == false) ? string(" predecessor") : string(""))) << LOG_SPACE << updatedFinger.toStrIDonly();
        msgLog(name(), LOG_TXRX, LOG_INFO, m_ssLog.str(), DEBUG_LOG | INTERNAL_LOG);
        

        //tryAddNewSeed(rxMess.searchedNodeIDwithSocket);

        if (m_inaccessibleFingers.size() > 0) {
            if (timer.retryMess.destNodeIDwithSocket.id == m_inaccessibleFingers.at(0).timer.retryMess.destNodeIDwithSocket.id) {
                msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("Restore timer ") + chord_timer_message().type2str(m_inaccessibleFingers[0].timer.type) + LOG_SPACE + state2str(m_inaccessibleFingers[0].timer.issuedState), DEBUG_LOG | INTERNAL_LOG);
                m_inaccessibleFingers[0].timer.isDelayed = false;
                pushNewTimer(m_inaccessibleFingers[0].timer, false);
                eraseFirstInaccessFingersIssueNewMessage();
            }
        }
    }


    void low_latency_chord::setNextState(const finite_state& state) {
        m_state = state;
        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state), DEBUG_LOG | INTERNAL_LOG);               
    }


    bool low_latency_chord::setCopyPreviousAliveFinger() {
        uint fingerIndex = 0;
        node_address newAddr;

        node_address_latency aliveFinger;
       
        
        if (m_isClockWise) { 
            findPrevAliveFinger(m_isClockWise, m_cwFingerIndex, aliveFinger);
            if (aliveFinger.isNone())
                return false;
            
            m_cwFingers[m_cwFingerIndex].set(aliveFinger.toNodeAddress());
            m_cwFingers[m_cwFingerIndex].isUpdated = true;
            m_cwFingers[m_cwFingerIndex].updateTime = sc_time_stamp();
            m_cwFingers[m_cwFingerIndex].motive = MOTIVE_COPY;
            if (m_cwFingerIndex == 0)
                m_successor = m_cwFingers[m_cwFingerIndex];            
            newAddr = m_cwFingers[m_cwFingerIndex];
            fingerIndex = m_cwFingerIndex;
            m_cwFingerIndex++;           
        }
        else {
            findPrevAliveFinger(m_isClockWise, m_ccwFingerIndex, aliveFinger);
            if (aliveFinger.isNone())
                return false;
            
            m_ccwFingers[m_cwFingerIndex].set(aliveFinger.toNodeAddress());
            m_ccwFingers[m_ccwFingerIndex].isUpdated = true;
            m_ccwFingers[m_ccwFingerIndex].updateTime = sc_time_stamp();
            m_ccwFingers[m_ccwFingerIndex].motive = MOTIVE_COPY;
            if (m_ccwFingerIndex == 0)
                m_predecessor = m_ccwFingers[m_ccwFingerIndex];
            newAddr = m_ccwFingers[m_ccwFingerIndex];
            fingerIndex = m_ccwFingerIndex;
            m_ccwFingerIndex++;
        }

        msgLog(name(), LOG_RX, LOG_INFO, state2str(m_state) + LOG_TAB + string("setCopyPreviousAliveFinger") + LOG_SPACE +
            (m_isClockWise ? m_cwFingers.at(m_cwFingerIndex-1).toStrFinger() : m_ccwFingers.at(m_ccwFingerIndex-1).toStrFinger(true)) + string(" = ") + aliveFinger.toStrFinger(true), DEBUG_LOG | INTERNAL_LOG);

        if (m_inaccessibleFingers.size() > 0) {
            if ((fingerIndex == m_inaccessibleFingers.at(0).badFinger.fingerIndex) && (m_isClockWise == m_inaccessibleFingers.at(0).badFinger.isClockWise)) {
                msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("Restore timer ") + chord_timer_message().type2str(m_inaccessibleFingers[0].timer.type) + LOG_SPACE + state2str(m_inaccessibleFingers[0].timer.issuedState), DEBUG_LOG | INTERNAL_LOG);
                m_inaccessibleFingers[0].timer.isDelayed = false;
                pushNewTimer(m_inaccessibleFingers[0].timer, false);
                eraseFirstInaccessFingersIssueNewMessage();
            }
        }
        return true;
    }


    bool low_latency_chord::repeatMessage(const chord_tx_message_type& type, const chord_byte_message_fields& rxMess, const chord_timer_message& expiredTimer) {
        switch (type) {
            case CHORD_TX_JOIN: {
                if (expiredTimer.requestCounter < m_confParams.CtxJoin) {
                    msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("repeatMessage ") + rxMess.type2str() + string(" mID ") + to_string(rxMess.messageID) + string(" reqC ") + to_string(expiredTimer.requestCounter + 1) + string("(") + to_string(m_confParams.CtxJoin) + string(")"), DEBUG_LOG | INTERNAL_LOG);
                    issueMessagePushTimers(CHORD_TX_JOIN, false, expiredTimer.requestCounter+1, rxMess, expiredTimer);
                    return true;
                }
                else {
                    return false;
                }
            }
            break;

            case CHORD_TX_FIND_SUCCESSOR: {
                if (expiredTimer.requestCounter < m_confParams.CtxFindSucc) {
                    msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("repeatMessage ") + rxMess.type2str() + string(" mID ") + to_string(rxMess.messageID) + string(" reqC ") + to_string(expiredTimer.requestCounter + 1) + string("(") + to_string(m_confParams.CtxFindSucc) + string(")"), DEBUG_LOG | INTERNAL_LOG);
                    issueMessagePushTimers(CHORD_TX_FIND_SUCCESSOR, false, expiredTimer.requestCounter+1, rxMess, expiredTimer);
                    return true;
                }
                else {
                    return false;
                }
            }
            break;

            case CHORD_TX_FIND_PREDECESSOR: {
                if (expiredTimer.requestCounter < m_confParams.CtxFindSucc) {
                    msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("repeatMessage ") + rxMess.type2str() + string(" mID ") + to_string(rxMess.messageID) + string(" reqC ") + to_string(expiredTimer.requestCounter + 1) + string("(") + to_string(m_confParams.CtxFindSucc) + string(")"), DEBUG_LOG | INTERNAL_LOG);
                    issueMessagePushTimers(CHORD_TX_FIND_PREDECESSOR, false, expiredTimer.requestCounter+1, rxMess, expiredTimer);
                    return true;
                }
                else {
                    return false;
                }
            }
            break;

            //case CHORD_TX_NOTIFY: {
            //    if (expiredTimer.requestCounter < m_confParams.CtxJoin) {
            //        msgLog(name(), LOG_TXRX, LOG_INFO, state2str(m_state) + LOG_TAB + string("repeatMessage ") + rxMess.type2str() + string(" mID ") + to_string(rxMess.messageID) + string(" reqC ") + to_string(expiredTimer.requestCounter + 1) + string("(") + to_string(m_confParams.CtxJoin) + string(")"), DEBUG_LOG | INTERNAL_LOG);
            //        issueMessagePushTimers(CHORD_TX_JOIN, false, expiredTimer.requestCounter + 1, rxMess, expiredTimer);
            //        return true;
            //    }
            //    else {
            //        return false;
            //    }
            //}
            //break;

            default:
                //ERROR
                msgLog(name(), LOG_TXRX, LOG_ERROR, state2str(m_state) + LOG_TAB + string("issueMessagePushTimers") + LOG_SPACE + LOG_ERROR_NOT_RECOGNIZED, ALL_LOG);   
        } 

        return false;
    }


    string low_latency_chord::logHeadStateString(const chord_message& mess, const bool existMess) {
        return state2str(m_state) + LOG_TAB + (existMess ? string("_# ")+mess.toStr() : string("_# Without") );
    }


    update_type low_latency_chord::setNextFingerToUpdate(const bool isTimerUpdateExpired, const node_address_latency badFinger) {
        if (isTimerUpdateExpired == false) {
            if (badFinger.isNone() == false) {
                m_isClockWise = badFinger.isClockWise;
                if (badFinger.isClockWise) {
                    m_cwFingerIndex = badFinger.fingerIndex;
                    m_cwFingers[m_cwFingerIndex].setCopy(badFinger);
                    return INACCESSIBLE_FINGER;
                }
                else {
                    m_ccwFingerIndex = badFinger.fingerIndex;
                    m_ccwFingers[m_ccwFingerIndex].setCopy(badFinger);
                    return INACCESSIBLE_FINGER;
                }
            }
        }

        if (m_state != STATE_JOIN) {
            if (isTimerUpdateExpired) {
                if ((m_isNowStabilize == false)) {
                    m_isNowStabilize = true;
                    return STABILIZE_SUCCESSOR;
                }
            }
        }

        //Lookup into cwFingers
        uint initialI = min((uint) m_cwFingers.size(), m_confParams.fillFingersMinQty);
        string currTime = sc_time_stamp().to_string();
        for (uint i = 0; i < initialI; ++i) {
            if (m_cwFingers[i].isUpdated == false) {  // (sc_time_stamp() >= m_cwFingers[i].updateTime))
                m_cwFingerIndex = i;
                m_isClockWise = true;
                return FIX_FINGER;
            }
        }

        for (uint i = initialI; i < m_cwFingers.size(); ++i) {
            if (m_cwFingers[i].isUpdated == false) {
                m_cwFingerIndex = i;
                m_isClockWise = true;
                return FIX_FINGER;
            }
        }
        
        //Lookup into ccwFingers
        if (CCW_FINGERS_TURN_ON) {
            for (uint i = 0; i < m_ccwFingers.size(); ++i) {
                if (m_ccwFingers[i].isUpdated == false) {
                    m_ccwFingerIndex = i;
                    m_isClockWise = false;
                    return FIX_FINGER;
                }
            }
        }



        //Not found finger to update. Set default values for call after STABILIZE_SUCCESSOR         
        for (uint i = initialI; i < m_cwFingers.size(); ++i) {
            m_cwFingers[i].isUpdated = false;
        }

        if (CCW_FINGERS_TURN_ON) {
            for (uint i = 0; i < m_ccwFingers.size(); ++i) {
                m_ccwFingers[i].isUpdated = false;
            }
        }
        m_isClockWise = true;
        m_cwFingerIndex = 0;
        m_ccwFingerIndex = 0;
        m_predecessor.isUpdated = false;
        m_successor.isUpdated = false;
        m_isNowStabilize = true;
        
        //LOG        
        m_ssLog << state2str(m_state) << LOG_TAB << "setNextFingerToUpdate" << " fingers are out of date, restart update";
        msgLog(name(), LOG_TXRX, LOG_INFO, m_ssLog.str(), DEBUG_LOG | INTERNAL_LOG);
        //return FIX_FINGER;
        return STABILIZE_SUCCESSOR;
    }


    bool low_latency_chord::doResetFlushPauseIfMess(const chord_message& mess, const bool existMess) {
        if (existMess == true) {
            if (mess.type == CHORD_HARD_RESET) {
                hardReset();
                goStateLoad();
                return true;
            }
            else if (mess.type == CHORD_SOFT_RESET) {
                softReset();
                goStateInit(mess, false);
                return true;
            }
            else if (mess.type == CHORD_FLUSH) {
                flush();
                goStateIdle(mess, false);
                return true;
            }
            else if (mess.type == CHORD_PAUSE) {
                m_isPaused = true;
                setNextState(STATE_OFF);
                return true;
            }
            else if (mess.type == CHORD_CONTINUE) {
                m_isPaused = false;
                setNextState(STATE_IDLE);
                //goStateJoin(chord_message(), false);
                //hardReset();
                //goStateLoad();
                return true;
            }
        }
        return false;
    }


    bool low_latency_chord::pushTxMessageMemoryList(const chord_message& mess) {
        bool res = false;
        if (mess.flags.needsACK == NEEDS_ACK) {           
            chord_message* firstMess = m_txMemoryList.messPointer(0);
            uint firstMessID = 0;
            if (firstMess != nullptr)
                firstMessID = firstMess->messageID;

            //Push message into buffer
            if (m_txMemoryList.push(mess, true) == true) {
                res =  true;
            }
            else {
                //OVERFLOW BUFFER
                m_logText = state2str(m_state) + LOG_TAB + string("pushTxMessageMemoryList") + LOG_TAB + LOG_ERROR_OVERFLOW + LOG_SPACE + string("erase first mess, id") + to_string(firstMessID);
                msgLog(name(), LOG_TXRX, LOG_INFO, m_logText, ALL_LOG);
                res = true;
            }                   
        }

        if ((mess.type == CHORD_TX_JOIN) || (mess.type == CHORD_TX_FIND_SUCCESSOR) || (mess.type == CHORD_TX_FIND_PREDECESSOR)) {
            uint firstMessID = m_txMemoryList.mess(0).messageID;
            //Push message into buffer
            if (m_txMemoryList.push(mess, true) == true) {
                res = true;
            }
            else {
                //OVERFLOW BUFFER
                m_logText = state2str(m_state) + LOG_TAB + string("pushTxMessageMemoryList") + LOG_TAB + LOG_ERROR_OVERFLOW + LOG_SPACE + string("erase first mess, id") + to_string(firstMessID);
                msgLog(name(), LOG_TXRX, LOG_WARNING, m_logText, ALL_LOG);
                res = true;
            }    
        }

        return res;
        
        //if (m_txMemoryList.size() < MAX_SIZE_MEMORY_LIST) {
        //    m_txMemoryList.push(mess);            
        //}
        //else {
        //    buffer_container::iterator minIt = m_txMemoryList.messIterator(0);
        //    buffer_container::iterator nextMess = minIt;
        //    uint counter = 1;
        //    nextMess++;
        //    while (counter < m_txMemoryList.size()) {
        //        if (minIt->creatingTime > nextMess->creatingTime) {
        //            minIt = nextMess;
        //        }
        //        nextMess++;
        //        counter++;
        //    }
        //    m_txMemoryList.eraseMess(minIt);
        //    m_txMemoryList.push(mess);
        //}
    }


    bool low_latency_chord::eraseTxMessageMemoryList(const chord_message& mess) {
        buffer_container::iterator memMessIt = m_txMemoryList.messIterator(0);
        for (uint i = 0; i < m_txMemoryList.size(); ++i) {
            chord_message memMess = *memMessIt;

            if ((memMess.messageID == mess.messageID) && (memMess.type == mess.type) && (memMess.issuedState == mess.issuedState)) {
                m_txMemoryList.eraseMess(memMessIt);
                return true;
            }
            memMessIt++;
        }
        return false;
    }


    buffer_container::iterator low_latency_chord::findTxMessageMemoryList(bool& exist, const uint messageID, const uint type, const finite_state state) {
        exist = false;
        buffer_container::iterator memMessIt = m_txMemoryList.messIterator(0);
        if (state == STATE_UNKNOWN) {
            for (uint i = 0; i < m_txMemoryList.size(); ++i) {            
                if ((memMessIt->messageID == messageID) && (memMessIt->type == type)) {
                    exist = true;
                    return memMessIt;
                }
                memMessIt++;
            }
        }
        else {
            for (uint i = 0; i < m_txMemoryList.size(); ++i) {
                if ((memMessIt->messageID == messageID) && (memMessIt->type == type) && (memMessIt->issuedState == state)) {
                    exist = true;
                    return memMessIt;
                }
                memMessIt++;
            }
        }
        return memMessIt;
    }


    finite_state low_latency_chord::findStateOnRxReplyMess(const chord_message& rxMess) {
        bool exist = false;
        buffer_container::iterator it;
        uint possibleTxType = CHORD_UNKNOWN;

        if (rxMess.type == CHORD_RX_SUCCESSOR) {
            vector<uint> possibleTxType = {CHORD_TX_JOIN, CHORD_TX_FIND_SUCCESSOR};
            for (uint i = 0; i < possibleTxType.size(); ++i) {
                for (uint stateI = 0; stateI < MAX_FINITE_STATE; ++stateI) {
                    for (uint typeI = 0; typeI < allowableTxMess[stateI].size(); ++typeI) {
                        string stateStr = state2str((finite_state)stateI);
                        if (possibleTxType[i] == allowableTxMess[stateI][typeI]) {
                            it = findTxMessageMemoryList(exist, rxMess.messageID, possibleTxType[i], (finite_state)stateI);
                            if (exist) {
                                m_txMemoryList.eraseMess(it);
                                return (finite_state)stateI;
                            }
                        }
                    }
                }
            }
        }
        else if (rxMess.type == CHORD_RX_PREDECESSOR) {
            vector<uint> possibleTxType = {CHORD_TX_FIND_PREDECESSOR};
            for (uint i = 0; i < possibleTxType.size(); ++i) {
                for (uint stateI = 0; stateI < MAX_FINITE_STATE; ++stateI) {
                    for (uint typeI = 0; typeI < allowableTxMess[stateI].size(); ++typeI) {
                        string stateStr = state2str((finite_state)stateI);
                        if (possibleTxType[i] == allowableTxMess[stateI][typeI]) {
                            it = findTxMessageMemoryList(exist, rxMess.messageID, possibleTxType[i], (finite_state)stateI);
                            if (exist) {
                                m_txMemoryList.eraseMess(it);
                                return (finite_state)stateI;
                            }
                        }
                    }
                }
            }
        }       
        else if (rxMess.type == CHORD_RX_ACK) {
            if (m_confParams.needsACK == NEEDS_ACK) {
                vector<uint> possibleTxType = {CHORD_TX_JOIN, CHORD_TX_FIND_SUCCESSOR, CHORD_TX_FIND_PREDECESSOR, CHORD_TX_NOTIFY, CHORD_TX_BROADCAST, CHORD_TX_MULTICAST, CHORD_TX_SINGLE, CHORD_TX_FWD_BROADCAST, CHORD_TX_FWD_MULTICAST, CHORD_TX_FWD_SINGLE};
                for (uint i = 0; i < possibleTxType.size(); ++i) {
                    for (uint stateI = 0; stateI < MAX_FINITE_STATE; ++stateI) {
                        for (uint typeI = 0; typeI < allowableTxMess[stateI].size(); ++typeI) {
                            string stateStr = state2str((finite_state)stateI);
                            if (possibleTxType[i] == allowableTxMess[stateI][typeI]) {
                                it = findTxMessageMemoryList(exist, rxMess.messageID, possibleTxType[i], (finite_state)stateI);
                                if (exist) {
                                    m_txMemoryList.eraseMess(it);
                                    return (finite_state)stateI;
                                }
                            }
                        }
                    }
                }
            }
        }

        return STATE_UNKNOWN;
    }




    low_latency_chord::FP low_latency_chord::mess2state(const event_type eventType, const chord_message& rxMess) {       
        if (arrFunctions.size() == 0) {
            arrFunctions.resize(MAX_FINITE_STATE, nullptr);
            arrFunctions[STATE_JOIN] = &low_latency_chord::goStateJoin;
            arrFunctions[STATE_IDLE] = &low_latency_chord::goStateIdle;
            arrFunctions[STATE_SERVICE] = &low_latency_chord::goStateService;
            arrFunctions[STATE_UPDATE] = &low_latency_chord::goStateUpdate;
            arrFunctions[STATE_INDATA] = &low_latency_chord::goStateIndata;
            arrFunctions[STATE_APPREQUEST] = &low_latency_chord::goStateApprequest;
        }

        if (eventType == RX_MESS_RECEIVED) {
            if ((isAddrValid(rxMess) == false))
                return nullptr;   //Drop message

            finite_state state = findStateOnRxReplyMess(rxMess);
            if ((state != STATE_UNKNOWN) && (STATE_JOIN <= state) && (state <= STATE_APPREQUEST)) {
                return arrFunctions[state];
            }
            else {
                switch (rxMess.type) {
                    case CHORD_RX_JOIN:  
                        return arrFunctions[STATE_SERVICE];

                    case CHORD_RX_NOTIFY:
                        return arrFunctions[STATE_UPDATE];

                    case CHORD_RX_FIND_SUCCESSOR: 
                        return arrFunctions[STATE_SERVICE];                

                    case CHORD_RX_FIND_PREDECESSOR:
                        return arrFunctions[STATE_SERVICE];

                    case CHORD_RX_ACK:
                        return nullptr; //Drop message                    

                    case CHORD_RX_SUCCESSOR: 
                        return nullptr; //Drop message 
                                               
                    case CHORD_RX_PREDECESSOR:
                        return nullptr; //Drop message 

                    case CHORD_RX_BROADCAST:
                        return arrFunctions[STATE_INDATA];

                    case CHORD_RX_MULTICAST:
                        return arrFunctions[STATE_INDATA];

                    case CHORD_RX_SINGLE:
                        return arrFunctions[STATE_INDATA];

                    default:
                        return nullptr;
                }
            }            
        }
        else if (eventType == TIMER_EXPIRED) {            
            if (rxMess.type == CHORD_TIMER_UPDATE)
                return arrFunctions[STATE_UPDATE];
            else if ((rxMess.issuedState >= STATE_JOIN) && (rxMess.issuedState  <= STATE_APPREQUEST))
                return arrFunctions[rxMess.issuedState];
            else
                return nullptr;
        }

        return nullptr;        
    }


    void low_latency_chord::pushInaccessibleFinger(const chord_timer_message& timer) {
        uint160 nodeID = timer.retryMess.destNodeIDwithSocket.id;
        bad_finger_with_retry_timer tmp;
        for (uint i = 0; i < m_cwFingers.size(); ++i) {
            if (nodeID == m_cwFingers[i].id) {
                m_cwFingers[i].isUpdated = false;
                tmp.badFinger.setCopy(m_cwFingers[i]);
                tmp.timer = timer;                
                m_inaccessibleFingers.push_back(tmp);
                return;
            }
        }

        for (uint i = 0; i < m_ccwFingers.size(); ++i) {
            if (nodeID == m_ccwFingers[i].id) {
                m_cwFingers[i].isUpdated = false;
                tmp.badFinger.setCopy(m_ccwFingers[i]);
                tmp.timer = timer;
                m_inaccessibleFingers.push_back(tmp);
                return;
            }
        }
    }

    void low_latency_chord::eraseFirstInaccessFingersIssueNewMessage() {
        if (m_inaccessibleFingers.size() > 0)
            m_inaccessibleFingers.erase(m_inaccessibleFingers.begin());
        
        if (m_inaccessibleFingers.size() == 0)
            m_isNowUpdate = false;
        else {
            node_address_latency& badFinger = m_inaccessibleFingers.at(0).badFinger;
            m_updateType = setNextFingerToUpdate(false, badFinger);
            if (m_updateType == INACCESSIBLE_FINGER)
                issueMessagePushTimers(CHORD_TX_FIND_SUCCESSOR);
        }
    }



    bool low_latency_chord::findPrevAliveFinger(const bool isClockWise, const uint forThisFinger, node_address_latency& prevFinger) {
        vector<node_address_latency>* myFingers;
        vector<node_address_latency>* neighbourFingers;

        if (isClockWise == true) {
            myFingers = &m_cwFingers;
            neighbourFingers = &m_ccwFingers;
        }
        else {
            myFingers = &m_ccwFingers;
            neighbourFingers = &m_cwFingers;
        }
        
        for (int i = forThisFinger-1; (i >= 0) && (i < myFingers->size()); --i) {
            if ((myFingers->at(i).isUpdated) && (myFingers->at(i).isNone() == false) && (myFingers->at(i).id != m_nodeAddr.id)) {
                prevFinger.setCopy(myFingers->at(i));
                return true;
            }
        }

        for (uint i = 0; i < neighbourFingers->size(); ++i) {
            if ((neighbourFingers->at(i).isUpdated) && (neighbourFingers->at(i).isNone() == false) && (neighbourFingers->at(i).id != m_nodeAddr.id)) {
                prevFinger.setCopy(neighbourFingers->at(i));
                return true;
            }
        }

        prevFinger.clear();
        return false;
    }


    void low_latency_chord::tryAddNewSeed(const node_address& newSeed) {
        network_address a;
        a.ip = newSeed.ip;
        a.inSocket = newSeed.inSocket;
        a.outSocket = newSeed.outSocket;
        for (uint i = 0; i < m_confParams.seed.size(); ++i) {
            if ((a.ip == m_confParams.seed[i].ip) &&
                (a.outSocket == m_confParams.seed[i].outSocket) &&
                (a.inSocket == m_confParams.seed[i].inSocket)) {
                return;             //This addr has already been added (early)
            }                
        }
       
        m_confParams.seed.push_back(a);
    }


    const vector<node_address_latency>* low_latency_chord::cw_fingers_pointer() const {
        return &m_cwFingers;    
    }


    const vector<node_address_latency>* low_latency_chord::ccw_fingers_pointer() const {
        return &m_ccwFingers;
    }


    node_address low_latency_chord::node_addr() const {
        return m_nodeAddr.toNodeAddress();
    }


    const finite_state* low_latency_chord::finite_state_pointer() const {
        return &m_state;
    }


    node_snapshot low_latency_chord::snapshot_pointers() {
        node_snapshot snapshot(&m_cwFingers, &m_ccwFingers, &m_state, &m_successor, &m_predecessor, m_nodeAddr.toNodeAddress());
        return snapshot;
    }


    void low_latency_chord::makeSnapshot() {
        vector<node_address_latency> invalidFingers;
        trp_port->check_fingers(m_nodeAddr.toNodeAddress(), invalidFingers);        

        sc_time period = TRP_PERIOD_SNAPSHOTS;
        m_eventMakeSnapshot.notify(period);
    }
}
