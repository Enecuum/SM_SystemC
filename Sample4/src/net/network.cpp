#include "net/network.h"

namespace P2P_MODEL
{

    network::network(sc_module_name name, const uint nodes) : sc_module(name) {                        
        setNodeAmount(nodes);

        SC_METHOD(send);
        dont_initialize();
        for (uint i = 0; i < m_eventSend.size(); ++i)
            sensitive << *(m_eventSend[i]);
    }


    network::~network() {
        deleteEventsPorts();
    }


    void network::push_into_network(const message_info& mess) {
        static uint i = 0;
        cout << "i = " << i++ << endl;

        auto it = m_portIndexByNodeID.find(mess.req.destNodeID);
        if (it == m_portIndexByNodeID.end()) {
            //ERROR
            m_logText = "push_into_network" + LOG_SPACER + string("PORT NOT FOUND for destNodeID ") + mess.req.destNodeID.to_string();
            msgLog(name(), LOG_RX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
        }
        else {
            uint portIndex = it->second;
            if ((portIndex >= m_messages.size()) || (portIndex >= m_eventSend.size())) {
                //ERROR
                m_logText = "push_into_network" + LOG_SPACER + string("BUFFER NOT FOUND for port ") + to_string(portIndex);
                msgLog(name(), LOG_RX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
            }
            else {
                chord_request& r = const_cast<chord_request&>(mess.req);
                r.appearanceTime = sc_time_stamp();

                m_messages[portIndex].push_back(mess);
                m_hasMessageInBuffer[portIndex] = true;
                m_eventSend[portIndex]->notify(0, SC_NS);

                m_logText = "push_into_network" + LOG_SPACER + string("port ") + to_string(portIndex) + string(" ") + r.toStr();
                msgLog(name(), LOG_RX, LOG_IN, m_logText, DEBUG_LOG | ERROR_LOG);
            }            
        }
    }

    void network::send() {        
        int portIndex = -1;
        for (uint i = 0; i < m_messages.size(); ++i) {
            if (m_messages[i].size() > 0) {
                portIndex = i;
                
                auto messIt = m_messages[portIndex].begin();
                m_logText = "send port " + to_string(portIndex) + LOG_SPACER + messIt->req.toStr();
                msgLog(name(), LOG_TX, LOG_OUT, m_logText, DEBUG_LOG | ERROR_LOG);

                (*(trp_ports[portIndex]))->receive_mess(*messIt);
                m_messages[portIndex].erase(messIt);

                //if (m_messages[portIndex].size() == 0)
                //    m_hasMessageInBuffer[portIndex] = false;
            }
        }        
    }


    void network::pushLatency(const uint160 nodeID, const sc_time latency) {
        static uint portIndex = 0;

        auto portIndexIt = m_portIndexByNodeID.find(nodeID);
        if (portIndexIt == m_portIndexByNodeID.end()) {
            //Container has no nodeID and latency
            if (portIndex >= trp_ports.size()) {
                //ERROR
                m_logText = "pushLatency" + LOG_SPACER + string("NOT FOUND PORT for portIndex ") + to_string(portIndex) + LOG_SPACER + string("nodeID: ") + nodeID.to_string();
                msgLog(name(), LOG_TXRX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
                //createNewEventPort();
                return;
            }

            //Create and put new element into container
            m_portIndexByNodeID.insert(pair<uint160, uint>(nodeID, portIndex));
            m_latencyByPortIndex.insert(pair<uint, sc_time>(portIndex, latency));
            portIndex++;
        }
        else {
            //Update latency in container
            auto latencyIt = m_latencyByPortIndex.find(portIndexIt->second);
            latencyIt->second = latency;
        }
    }


    void network::setNodeAmount(const uint nodes) {
        for (uint i = 0; i < nodes; ++i)
            createNewEventPort();
        
        m_messages.resize(nodes, vector<message_info>());
        m_hasMessageInBuffer.resize(nodes, false);
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

        m_messages.clear();
        m_portIndexByNodeID.clear();
        m_latencyByPortIndex.clear();
    }


    void network::createNewEventPort() {               
        sc_event* ev = nullptr;
        ev = new sc_event();
        m_eventSend.push_back(ev);
        
        sc_port<trp_network_if>* port = nullptr;
        port = new sc_port<trp_network_if>();
        trp_ports.push_back(port);          
    }
}