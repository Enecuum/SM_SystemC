#include "mntr/monitor.h"

namespace P2P_MODEL
{
    monitor::monitor(sc_module_name _name, const uint nodes): sc_module(_name) {        
        createPorts(nodes);

        SC_METHOD(checkFingersPeriodically);
        dont_initialize();
        sensitive << m_eventCheckFingers;

    }


    void monitor::createPorts(const uint nodes) {
        trp_ports.resize(nodes, nullptr);
        for (uint i = 0; i < nodes; ++i) {
            trp_ports[i] = new sc_port<trp_monitor_if>();
        }
    }


    monitor::~monitor() { 
        if (trp_ports.size() > 0) {
            for (uint i = 0; i < trp_ports.size(); ++i) {
                delete trp_ports[i];
                trp_ports[i] = nullptr;
            }
            trp_ports.clear();
        }


    }


    void monitor::setReferenceCwFingers(const uint160 id, const vector<node_address>& fingers) {
        m_id2refCwFingers[id] = fingers;
    }


    void monitor::setReferenceCcwFingers(const uint160 id, const vector<node_address>& fingers) {
        m_id2refCcwFingers[id] = fingers;
    }


    void monitor::setCwFingersUnderTest(const uint160 id, const vector<node_address_latency>* fingers) {
        m_copyCwFingers[id] = fingers;
    }

    void monitor::setCcwFingersUnderTest(const uint160 id, const vector<node_address_latency>* fingers) {
        m_copyCcwFingers[id] = fingers;
    }
    
    
    void monitor::setPeriodCheckFingers(const sc_time period) {
        m_checkFingersPeriod = period;
    }


    void monitor::checkFingersPeriodically() {
        m_eventCheckFingers.notify(m_checkFingersPeriod);
    }

    bool monitor::check_fingers(const uint160 id, vector<node_address_latency>& invalidFingers) {
        invalidFingers.clear();

        auto refCwIt = m_id2refCwFingers.find(id);
        auto refCcwIt = m_id2refCcwFingers.find(id);
        auto cwIt = m_copyCwFingers.find(id);
        auto ccwIt = m_copyCwFingers.find(id);
        if ((refCwIt == m_id2refCwFingers.end()) || (cwIt == m_copyCwFingers.end()) || (refCcwIt == m_id2refCcwFingers.end()) || (ccwIt == m_copyCcwFingers.end())) {
            //ERROR
            m_ssLog << "NOT FOUND FINGERS TO CHECK nodeID " << id.to_string(SC_DEC) << " 0x" << id.to_string(SC_HEX_US);
            msgLog(name(), LOG_TXRX, LOG_ERROR, m_ssLog.str(), ALL_LOG);
            return false;
        }
        
        vector<node_address>* ref; 
        const vector<node_address_latency>* underTest; 

        ref = &(refCwIt->second);
        underTest = cwIt->second;

        if (ref->size() != underTest->size()) {
            //ERROR
            m_ssLog << "SIZE OF REF CW FINGERS NOT MATCHED UNDER TEST CW FINGERS nodeID " << id.to_string(SC_DEC) << " 0x" << id.to_string(SC_HEX_US);
            msgLog(name(), LOG_TXRX, LOG_ERROR, m_ssLog.str(), ALL_LOG);
            return false;
        }
        
        uint i = 0;
        while (i < ref->size()) {
            if (ref->at(i).id != underTest->at(i).id) {
                node_address_latency invalidFinger;
                invalidFinger.setCopy( underTest->at(i) );
                invalidFingers.push_back(invalidFinger);
            }
            ++i;
        }


        ref = &(refCcwIt->second);
        underTest = ccwIt->second;

        if (ref->size() != underTest->size()) {
            //ERROR
            m_ssLog << "SIZE OF REF CCW FINGERS NOT MATCHED UNDER TEST CCW FINGERS nodeID " << id.to_string(SC_DEC) << " 0x" << id.to_string(SC_HEX_US);
            msgLog(name(), LOG_TXRX, LOG_ERROR, m_ssLog.str(), ALL_LOG);
            return false;
        }

        i = 0;
        while (i < ref->size()) {
            if (ref->at(i).id != underTest->at(i).id) {
                node_address_latency invalidFinger;
                invalidFinger.setCopy(underTest->at(i));
                invalidFingers.push_back(invalidFinger);
            }
            ++i;
        }


        if (invalidFingers.size() == 0) {
            auto it = m_isValidFingers.find( id );
            
            if (it == m_isValidFingers.end())
                      ;
            return true;
        }
        return false;
    }
}