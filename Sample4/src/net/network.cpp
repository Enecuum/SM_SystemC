#include "net/network.h"

namespace P2P_MODEL
{

    network::network(sc_module_name name, const uint nodes) : sc_module(name) {                        
        setNodeAmount(nodes);

        SC_METHOD(send);
        dont_initialize();
        for (uint i = 0; i < m_eventSend.size(); ++i)
            sensitive << *(m_eventSend[i]);

        SC_METHOD(checkReceive);
        dont_initialize();
        sensitive << m_eventCheckReceive;
        
        initRands();
    }


    network::~network() {
        deleteEventsPorts();
    }


    void network::push_into_network(const chord_byte_message& networkDataUnit) {
        //Needs to parse fields of chord_byte_message into chord_byte_message_fields
        auto it = m_portIndexByNodeID.find(networkDataUnit.fields->destNodeID.id);
        chord_byte_message_fields r = *(networkDataUnit.fields);

        if (it == m_portIndexByNodeID.end()) {
            //ERROR
            m_logText = "push_into_network" + LOG_TAB + string("NOT FOUND OUTPORT ") + r.toStr();
            msgLog(name(), LOG_RX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
        }
        else {
            uint portIndex = it->second;
            if (portIndex >= m_buffMess.size()) {
                //ERROR
                m_logText = "push_into_network" + LOG_TAB + string("NOT FOUND BUFFER for outport") + to_string(portIndex);
                msgLog(name(), LOG_RX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
            }
            else { 
                m_buffMess[portIndex].push_back(networkDataUnit);
                 
                m_eventCheckReceive.notify(0, SC_NS);
                m_hasNewMess = true;

                m_logText = "push_into_network" + LOG_TAB + string("out") + to_string(portIndex) + LOG_SPACE + r.toStr();
                msgLog(name(), LOG_RX, LOG_IN, m_logText, DEBUG_LOG | ERROR_LOG);
            }            
        }
    }


    void network::checkReceive() {        
        int portIndex = CAN_USE;
        if (m_hasNewMess == true) {
            for (uint i = 0; i < m_buffMess.size(); ++i) {
                if ((m_buffMess[i].size() > 0) && (m_wakeUpInfo[i].bufIndex == CAN_USE)) {
                    portIndex = i;
                    auto messIt = m_buffMess[portIndex].begin();

                    auto portIndexIt = m_portIndexByNodeID.find(messIt->fields->srcNodeID.id);
                    if (portIndexIt == m_portIndexByNodeID.end()) {
                        //ERROR
                        m_logText = "checkReceive" + LOG_TAB + string("NOT FOUND INPORT ") + messIt->fields->toStr();
                        msgLog(name(), LOG_TX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
                        return;
                    }
                
                    uint from = portIndexIt->second;

                    portIndexIt = m_portIndexByNodeID.find(messIt->fields->destNodeID.id);
                    if (portIndexIt == m_portIndexByNodeID.end()) {
                        //ERROR
                        m_logText = "checkReceive" + LOG_TAB + string("NOT FOUND OUTPORT ") + messIt->fields->toStr();
                        msgLog(name(), LOG_TX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
                    }
                
                    uint to = portIndexIt->second;
                
                    double millisec = m_latencyTable[from][to].to_seconds() * 1000;
                
                    uint randValue  = 0;
                    if (m_randDesperseMillisec[i] != 0)
                        randValue = rand() % m_randDesperseMillisec[i];

                    uint sign  = rand() % 1;                
                    if (sign == 0)
                        millisec += randValue;
                    else {
                        if (randValue >= millisec)
                            millisec = 1;
                        else
                            millisec -= randValue;
                    }
                    m_wakeUpInfo[portIndex].bufIndex = i;
                    m_wakeUpInfo[portIndex].time = sc_time(sc_time_stamp().to_seconds() + millisec/1000, SC_SEC);
                    m_eventSend[portIndex]->notify(millisec, SC_MS);
                
                    m_logText = "checkReceive" + LOG_TAB + "out" + to_string(to) + LOG_SPACE + "in" + to_string(from) + LOG_SPACE + messIt->fields->toStr();
                    msgLog(name(), LOG_RX, LOG_IN, m_logText, DEBUG_LOG | ERROR_LOG);                
                
                    m_eventCheckReceive.notify(0, SC_NS); 
                }
            }

            if (portIndex == CAN_USE)
                m_hasNewMess = false;
        }
    }

    void network::setNodeAddressList(const vector<network_address>& addrs) {
        static uint portIndex = 0;
        for (uint i = 0; i < addrs.size(); ++i) {
            uint160 nodeID = node_address(addrs[i]).id;
            auto portIndexIt = m_portIndexByNodeID.find(nodeID);
            if (portIndexIt == m_portIndexByNodeID.end()) {
                //Container has no nodeID 
                if (portIndex >= trp_ports.size()) {
                    //ERROR
                    m_logText = "setNodeAddressList" + LOG_TAB + string("NOT FOUND PORT nodeID: ") + nodeID.to_string();
                    msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
                    return;
                }
        
                //Create and put new element into container
                m_portIndexByNodeID.insert(pair<uint160, uint>(nodeID, portIndex));
                portIndex++;
            }
        }
    }



    void network::setRandomLatencyTable(const uint millisecFrom, const uint millisecTo, const uint millisecDesperse) {
        for (int i = 0; i < m_latencyTable.size(); ++i) {
            if (millisecDesperse == 0)   
                m_randDesperseMillisec[i] = 0;
            else {
                m_randDesperseMillisec[i] = 1 + rand()% millisecDesperse;
            }

            for (int j = 0; j < m_latencyTable[i].size(); ++j) {
                if (millisecFrom == millisecTo)
                    m_latencyTable[i][j] = sc_time(millisecFrom, SC_MS);
                else
                    m_latencyTable[i][j] = sc_time(millisecFrom + rand()% millisecTo, SC_MS);                                 
            }
        }
    }


    void network::setNodeAmount(const uint nodes) {
        for (uint i = 0; i < nodes; ++i)
            createNewEventPort();
        
        m_buffMess.resize(nodes, list<chord_byte_message>());
        
        message_wake_up_info w;
        w.bufIndex = CAN_USE;
        w.time = SC_ZERO_TIME;
        m_wakeUpInfo.resize(nodes, w);
        
        vector<sc_time> tmp; 
        tmp.resize(nodes, SC_ZERO_TIME);
        m_latencyTable.resize(nodes, tmp);
        m_randDesperseMillisec.resize(nodes, 0);
        m_hasNewMess = false;
    }


    string& network::latencyTableToStr() {
        static string str;
        for (uint i = 0; i < m_latencyTable.size(); ++i) {
            str += string("in") + to_string(i) + string(" >") + LOG_TAB;
            for (uint j = 0; j < m_latencyTable[i].size(); ++j) {
                uint latency = static_cast<uint>(m_latencyTable[i][j].to_seconds()*1000);
                str += to_string(latency) + string("|") + to_string(m_randDesperseMillisec[i]) + LOG_TAB;
            }
            str += "\n";
        }
        str += string(" ") + LOG_TAB;
        for (uint j = 0; j < m_latencyTable[0].size(); ++j)
            str += "v" + LOG_TAB;
        str += "\n" + string("out") + LOG_TAB;
        for (uint j = 0; j < m_latencyTable[0].size(); ++j)
            str += to_string(j) + LOG_TAB;
        return str;
    }


    void network::deleteEventsPorts() {
        if (m_eventSend.size() > 0) {
            for (int i = 0; i < m_eventSend.size(); ++i) {
                if (m_eventSend[i] != nullptr) {
                    delete m_eventSend[i];
                    m_eventSend[i] = nullptr;
                }
            }
            m_eventSend.clear();
        }

        if (trp_ports.size() > 0) {
            for (int i = 0; i < trp_ports.size(); ++i) {
                if (trp_ports[i] != nullptr) {
                    delete trp_ports[i];
                    trp_ports[i] = nullptr;
                }
            }
            trp_ports.clear();
        }

        m_buffMess.clear();
        m_portIndexByNodeID.clear();
    }


    void network::createNewEventPort() {               
        sc_event* e = nullptr;
        e = new sc_event();
        m_eventSend.push_back(e);
        
        sc_port<trp_network_if>* port = nullptr;
        port = new sc_port<trp_network_if>();
        trp_ports.push_back(port);          
    }


    uint network::specifyNewMessType(const uint type) {
        uint res;
        res = CHORD_UNKNOWN;

        switch (type) {
        case CHORD_TX_JOIN:                res = CHORD_RX_JOIN;                 break;
        case CHORD_TX_NOTIFY:              res = CHORD_RX_NOTIFY;               break;
        case CHORD_TX_ACK:                 res = CHORD_RX_ACK;                  break;
        case CHORD_TX_REPLY_FIND_SUCESSOR: res = CHORD_RX_REPLY_FIND_SUCCESSOR; break;
        case CHORD_TX_FIND_SUCCESSOR:      res = CHORD_RX_FIND_SUCCESSOR;       break;
        case CHORD_TX_FWD_BROADCAST:       res = CHORD_RX_BROADCAST;            break;
        case CHORD_TX_FWD_MULTICAST:       res = CHORD_RX_MULTICAST;            break;
        case CHORD_TX_FWD_SINGLE:          res = CHORD_RX_SINGLE;               break;
        case CHORD_TX_BROADCAST:           res = CHORD_RX_BROADCAST;            break;
        case CHORD_TX_MULTICAST:           res = CHORD_RX_MULTICAST;            break;
        case CHORD_TX_SINGLE:              res = CHORD_RX_SINGLE;               break;
        default:
            //ERROR
            msgLog(name(), LOG_RX, LOG_ERROR_INDICATOR, "specifyNewMessType CHORD_UNKNOWN", DEBUG_LOG | ERROR_LOG);
        }
        return res;
    }


    void network::send() {
        for (int i = 0; i < m_wakeUpInfo.size(); ++i) {
            if ((m_wakeUpInfo[i].bufIndex != CAN_USE) && (m_wakeUpInfo[i].time >= sc_time_stamp())) {
                auto messIt = m_buffMess[i].begin();
                messIt->fields->type = specifyNewMessType(messIt->fields->type);

                m_logText = "send" + LOG_TAB + "out" + to_string(i) + LOG_SPACE + messIt->fields->toStr();
                msgLog(name(), LOG_TX, LOG_OUT, m_logText, DEBUG_LOG | ERROR_LOG);

                (*(trp_ports[i]))->receive_mess(*messIt);
                m_buffMess[i].erase(messIt);

                m_wakeUpInfo[i].bufIndex = CAN_USE;                
                m_eventCheckReceive.notify(0, SC_NS);
                break;
            }                            
        }
    }
}


