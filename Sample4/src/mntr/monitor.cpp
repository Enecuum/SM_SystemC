#include "mntr/monitor.h"

namespace P2P_MODEL
{
    monitor::monitor(sc_module_name _name, const uint nodes, const uint fingersSize, const bool looksMotive): sc_module(_name) {
        createPorts(nodes);
        setVerifyMode(AUTO_VERIFY);

        m_fingersSize = fingersSize;
        m_nodes = nodes;
        m_looksMotive = looksMotive;
        //SC_METHOD(checkFingersPeriodically);
        //dont_initialize();
        //sensitive << m_eventCheckFingers;
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
        auto it = m_copySnapshot.find(id);
        if (it == m_copySnapshot.end()) {
            node_address addr;
            addr.id = id;
            node_snapshot snapshot(const_cast<vector<node_address_latency>*> (fingers), nullptr, nullptr, addr);
            m_copySnapshot[id] = snapshot;
        }
        else
            m_copySnapshot[id].pCwFingers = const_cast<vector<node_address_latency>*> (fingers);
    }


    void monitor::setCcwFingersUnderTest(const uint160 id, const vector<node_address_latency>* fingers) {
        auto it = m_copySnapshot.find(id);
        if (it == m_copySnapshot.end()) {
            node_address addr;
            addr.id = id;
            node_snapshot snapshot(nullptr, const_cast<vector<node_address_latency>*> (fingers), nullptr, addr);
            m_copySnapshot[id] = snapshot;
        }
        else
            m_copySnapshot[id].pCcwFingers = const_cast<vector<node_address_latency>*> (fingers);       
    }


    void monitor::setFiniteStateUnderTest(const uint160 id, const finite_state* state) {
        auto it = m_copySnapshot.find(id);
        if (it == m_copySnapshot.end()) {
            node_address addr;
            addr.id = id;
            node_snapshot snapshot(nullptr, nullptr, const_cast<finite_state*> (state), addr);
            m_copySnapshot[id] = snapshot;
        }
        else
            m_copySnapshot[id].pFiniteState = const_cast<finite_state*> (state);
    }

    void monitor::setNodeAddrUnderTest(const uint160 id, const node_address& addr) {
        auto it = m_copySnapshot.find(id);
        if (it == m_copySnapshot.end()) {
            node_snapshot snapshot(nullptr, nullptr, nullptr, addr);
            m_copySnapshot[id] = snapshot;
        }
        else
            m_copySnapshot[id].nodeAddr = addr;
    }

    void monitor::setSnapshotUnderTest(const node_snapshot& snapshot) {        
        m_copySnapshot[snapshot.nodeAddr.id] = snapshot;
    }
    
    
    //void monitor::setPeriodCheckFingers(const sc_time period) {
    //    m_checkFingersPeriod = period;
    //}


    void monitor::checkFingersPeriodically() {
        //m_eventCheckFingers.notify(m_checkFingersPeriod);
    }

    vector<node_address> monitor::genRefFingers(const vector<node_address>& addrs, const uint160 id, const bool isClockWise) {
        node_address a; 
        a.inSocket = 999;
        a.outSocket = 999;
        a.id = 999;
        a.ip = "200.200.200.200";
        vector<node_address> newRef(m_fingersSize, a);



        return newRef;
    }

    bool monitor::verifyByRefFingers(const uint160 id, const vector<node_address_latency>& nodeFingers, const vector<node_address>& refFingers, vector<node_address_latency>& invalidFingers) {        
        invalidFingers.clear();
        bool success = true;
        for (uint i = 0; i < nodeFingers.size(); ++i) {
            if (nodeFingers.at(i).id != refFingers.at(i).id) {
                node_address_latency bad;
                bad.setCopy(nodeFingers.at(i));
                invalidFingers.push_back(bad);
                success = false;
            }
        }
        
        return success;
    }


    bool monitor::verifySnapshots(const vector<node_address>& addrs) {
        bool success = false;

        for (auto snapshotIt = m_copySnapshot.begin(); snapshotIt != m_copySnapshot.end(); ++snapshotIt)
        {
            node_snapshot& snapshot = snapshotIt->second;

            if ((snapshot.pFiniteState == nullptr) ||
                (snapshot.pCwFingers == nullptr) ||
                (snapshot.pCcwFingers == nullptr))
                continue;

            if ((snapshot.pCwFingers->size() == 0) ||
                (snapshot.pCcwFingers->size() == 0))
                continue;

            if ((STATE_OFF  <= *(snapshot.pFiniteState)) &&
                (STATE_JOIN > *(snapshot.pFiniteState)))
                continue;


            map<uint160, vector<node_address>>::iterator cwRefIt; 
            map<uint160, vector<node_address>>::iterator ccwRefIt;

            if (m_verifyMode == AUTO_VERIFY) {
                m_id2refCwFingers[snapshot.nodeAddr.id]  = genRefFingers(addrs, snapshot.nodeAddr.id, true);
                m_id2refCcwFingers[snapshot.nodeAddr.id] = genRefFingers(addrs, snapshot.nodeAddr.id, false);
            }
            
            cwRefIt = m_id2refCwFingers.find(snapshot.nodeAddr.id);
            ccwRefIt = m_id2refCcwFingers.find(snapshot.nodeAddr.id);
            


            if (cwRefIt == m_id2refCwFingers.end()) {
                //ERROR
                m_ssLog << "NOT FOUND REF CW FINGERS FOR VERIFICATION, nodeID " << (snapshot.nodeAddr).toStrIDonly();
                msgLog(name(), LOG_TXRX, LOG_ERROR, m_ssLog.str(), ALL_LOG);
                return false;
            }

            if (cwRefIt->second.size() != snapshot.pCwFingers->size()) {
                //ERROR
                m_ssLog << "MISMATCHED SIZES OF REF CW FINGERS AND FINGERS UNDER VERIFICATION, nodeID " << (snapshot.nodeAddr).toStrIDonly();
                msgLog(name(), LOG_TXRX, LOG_ERROR, m_ssLog.str(), ALL_LOG);
                return false;
            }

            if (ccwRefIt == m_id2refCcwFingers.end()) {
                //ERROR
                m_ssLog << "NOT FOUND REF CCW FINGERS FOR VERIFICATION, nodeID " << (snapshot.nodeAddr).toStrIDonly();
                msgLog(name(), LOG_TXRX, LOG_ERROR, m_ssLog.str(), ALL_LOG);
                return false;
            }

            if (ccwRefIt->second.size() != snapshot.pCcwFingers->size()) {
                //ERROR
                m_ssLog << "MISMATCHED SIZES OF REF CCW FINGERS AND FINGERS UNDER VERIFICATION, nodeID " << (snapshot.nodeAddr).toStrIDonly();
                msgLog(name(), LOG_TXRX, LOG_ERROR, m_ssLog.str(), ALL_LOG);
                return false;
            }

            vector<node_address_latency> cwInvalid, ccwInvalid;
            bool cwSuccess = true;
            bool ccwSuccess = true;
            cwSuccess  = verifyByRefFingers(snapshot.nodeAddr.id, *(snapshot.pCwFingers), cwRefIt->second, cwInvalid);
            ccwSuccess = verifyByRefFingers(snapshot.nodeAddr.id, *(snapshot.pCwFingers), ccwRefIt->second, ccwInvalid);
            success = cwSuccess & ccwSuccess;

            if (success == false) {                
                //Õ¿œŒÀÕ»“‹ m_id2invalidFingers

            }
        }
        return success;             
    }


    void monitor::check_fingers(const node_address& addr, vector<node_address_latency>& invalidFingers) {
//DEBUG
string timeStr = sc_time_stamp().to_string();
string nameStr = name();
//DEBUG        
        //sc_time period = SNAPSHOT_PERIOD;

        string str;
        static sc_time lastCallTime = SC_ZERO_TIME;
        static bool doPrint = true;
        uint i = 0;


        if (lastCallTime != sc_time_stamp()) {
            doPrint = true;
        }

        if (doPrint == true) {
            json J;
            string currTime = sc_time_stamp().to_string();                  //to_string(sc_time_stamp().to_seconds()) + string(" s");        //std::replace(currTime.begin(), currTime.end(), ',', '.');            
            J["curr time"] = currTime;

            uint active = 0; 
            vector<node_address> addrs;
            for (auto it = m_copySnapshot.begin(); it != m_copySnapshot.end(); ++it) {
                if (it->second.pFiniteState == nullptr)
                    continue;

                if ((STATE_JOIN       <= *(it->second.pFiniteState)) &&
                    (MAX_FINITE_STATE >  *(it->second.pFiniteState)))
                {
                    ++active;
                    addrs.push_back( it->second.nodeAddr );
                }
            }

            bool isInvalid = verifySnapshots(addrs);            


            J["active"] = active;

            auto mapIt = m_copySnapshot.begin();
            while (mapIt != m_copySnapshot.end())
            {
                node_snapshot& snapshot = mapIt->second;

                if ((STATE_JOIN       <= *(snapshot.pFiniteState)) &&
                    (MAX_FINITE_STATE >  *(snapshot.pFiniteState))) {
                   
                    auto ccwIt = snapshot.pCcwFingers->begin();

                    json j;                    
                    {
                        j["node_id"] = snapshot.nodeAddr.toStrIDonly().erase(0, 1);


                        if (snapshot.pCwFingers->size() == 0) {
                            j["succ"] = "null";
                            j[" cw fing"] = "null";
                        }
                        else {
                            auto cwIt = snapshot.pCwFingers->begin();
                            j["succ"] = cwIt->id.to_uint64();                      //toStrIDonly().erase(0, 1);

                            i = 0;
                            while (cwIt != snapshot.pCwFingers->end()) {
                                str = string(" cw fing[") + to_string(i) + string("]");
                                if (m_looksMotive == true)
                                    j[str] = cwIt->toStrIDmotive();                   ///*toStrFinger()*/toStrIDonly().erase(0,1);
                                else
                                    j[str] = cwIt->id.to_uint64();
                                cwIt++;
                                i++;
                            }
                        }

                        if (snapshot.pCcwFingers->size() == 0) {
                            j["pred"] = "null";
                            j["ccw fing"] = "null";
                        }
                        else {
                            auto ccwIt = snapshot.pCcwFingers->begin();
                            j["pred"] = ccwIt->id.to_uint64();                    //toStrIDonly().erase(0, 1);
                            
                            i = 0;
                            while (ccwIt != snapshot.pCcwFingers->end()) {
                                str = string("ccw fing[") + to_string(i) + string("]");
                                if (m_looksMotive == true)
                                    j[str] = ccwIt->toStrIDmotive();                   ///*toStrFinger()*/toStrIDonly().erase(0,1);
                                else 
                                    j[str] = ccwIt->id.to_uint64();
                                ccwIt++;
                                i++;
                            }
                        }

                        J["nodes"] += j;
                    }
                }
                mapIt++;                
            }
            log::snapshotLogJSON(J);

            doPrint = false;
            lastCallTime = sc_time_stamp();
        }
    }
    
    
    void monitor::check_fingers(const node_snapshot& snapshot, vector<node_address_latency>& invalidFingers) {
        //For future use
    }

    
    void monitor::setVerifyMode(const monitor_mode mode) {
        m_verifyMode = mode;
    }
}












/*bool monitor::check_fingers(const uint160 id, vector<node_address_latency>& invalidFingers) {
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
    }*/

    /*    
    if (ccwIt == m_copyCcwFingers.end()) {
        //ERROR
        m_ssLog << "NOT FOUND CW FINGERS TO VERIFY, nodeID " << (nodeAddrIt->second).toStrIDonly();
        msgLog(name(), LOG_TXRX, LOG_ERROR, m_ssLog.str(), ALL_LOG);
        return;
    }
    else if (ccwIt == m_copyCcwFingers.end()) {
        //ERROR
        m_ssLog << "NOT FOUND CCW FINGERS TO VERIFY, nodeID " << (nodeAddrIt->second).toStrIDonly();
        msgLog(name(), LOG_TXRX, LOG_ERROR, m_ssLog.str(), ALL_LOG);
        return;
    }
    */ 