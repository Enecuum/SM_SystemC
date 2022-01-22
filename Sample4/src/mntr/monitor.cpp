#include "mntr/monitor.h"

namespace P2P_MODEL
{
    monitor::monitor(sc_module_name _name, const uint nodes, const uint fingersSize, const bool showMotive, const bool showCcwFingers, const monitor_mode mode): sc_module(_name) {
        createPorts(nodes);
        setVerifyMode(mode);

        m_fingersSize = fingersSize;
        m_nodes = nodes;
        m_showMotive = showMotive;
        m_showCcwFingers = showCcwFingers;
        
        SC_METHOD(makeSnapshotFinal);
        dont_initialize();
        sensitive << m_eventMakeSnapshotFinal;       
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
            node_snapshot snapshot(const_cast<vector<node_address_latency>*> (fingers), nullptr, nullptr, nullptr, nullptr, addr);
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
            node_snapshot snapshot(nullptr, const_cast<vector<node_address_latency>*> (fingers), nullptr, nullptr, nullptr, addr);
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
            node_snapshot snapshot(nullptr, nullptr, const_cast<finite_state*> (state), nullptr, nullptr, addr);
            m_copySnapshot[id] = snapshot;
        }
        else
            m_copySnapshot[id].pFiniteState = const_cast<finite_state*> (state);
    }

    void monitor::setNodeAddrUnderTest(const uint160 id, const node_address& addr) {
        auto it = m_copySnapshot.find(id);
        if (it == m_copySnapshot.end()) {
            node_snapshot snapshot(nullptr, nullptr, nullptr, nullptr, nullptr, addr);
            m_copySnapshot[id] = snapshot;
        }
        else
            m_copySnapshot[id].nodeAddr = addr;
    }

    void monitor::setSnapshotUnderTest(const node_snapshot& snapshot) {        
        m_copySnapshot[snapshot.nodeAddr.id] = snapshot;
    }
    
    
    void monitor::makeSnapshotFinal() {
        node_address tmp;
        vector<node_address_latency> tmpVector;
        check_fingers(tmp, tmpVector, true);
    }


    void monitor::checkFingersPeriodically() {
        //m_eventCheckFingers.notify(m_checkFingersPeriod);
    }

    vector<node_address> monitor::genRefFingers(vector<node_address>& addrs, const node_address& currNodeAddr, const bool isClockWise) {
//DEBUG
if (sc_time_stamp() >= sc_time(13010, SC_MS))
    int herebreakpoint = 0;
//DEBUG           
        sort(addrs.begin(), addrs.end(), [](node_address& a, node_address& b) { return a.id < b.id; });
        
        auto addrIt = addrs.rbegin();
        node_address maxAddr = *addrIt;        
        if ((addrs.size() >= 2) && (maxAddr.id == currNodeAddr.id)) {
            ++addrIt;
            maxAddr = *addrIt;        
        }

        addrIt = addrs.rend()-1;
        node_address minAddr = *addrIt;
        if ((addrs.size() >= 2) && (minAddr.id == currNodeAddr.id)) {
            --addrIt;
            minAddr = *addrIt;
        }


        static vector<uint160> mask;
        if (mask.size() == 0) {
            uint160 value = 1;
            mask.reserve(m_fingersSize);
            for (size_t i = 0; i < m_fingersSize; ++i) {
                mask.push_back(value);
                value *= 2;    
            }
        }                               

        uint160 expected;
        int coeff = -1;
        if (isClockWise)
            coeff = 1;

        vector<node_address> newRef;
        newRef.resize(m_fingersSize, currNodeAddr);
        
        size_t i = newRef.size()-1;
        auto refIt = newRef.rbegin();        
        for (auto maskIt = mask.crbegin(); (maskIt != mask.crend()) && (refIt != newRef.rend()); ++maskIt) {
            expected = currNodeAddr.id + (*maskIt)*coeff;
            
            bool isFound = false;
            for (addrIt = addrs.rbegin(); addrIt != addrs.rend(); ++addrIt) {
                if (addrIt->id == currNodeAddr.id)
                    continue;

                
                if (addrIt->id >= expected) {
                    *refIt = *addrIt; 
                    isFound = true;
                }  
                else
                    break;
            }   

            if (!isFound) {
                if (expected > currNodeAddr.id)
                    *refIt = maxAddr;
                else
                    *refIt = minAddr;
            }

            //cout << "ref[" << i << "] = id " << currNodeAddr.id.to_uint64() << (coeff == 1 ? "+" : "-") << maskIt->to_uint64() << " = " << expected.to_uint64() << " <- " << refIt->id.to_uint64() << endl;

            ++refIt;
            --i;
        }
                
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


    bool monitor::verifySnapshots(vector<node_address>& addrs) {
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
//DEBUG
if (sc_time_stamp() >= sc_time(39000, SC_MS) )
    int herebreakpoint = 0;
//DEBUG
                m_id2refCwFingers[snapshot.nodeAddr.id]  = genRefFingers(addrs, snapshot.nodeAddr, true);
                m_id2refCcwFingers[snapshot.nodeAddr.id] = genRefFingers(addrs, snapshot.nodeAddr, false);
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


    void monitor::check_fingers(const node_address& addr, vector<node_address_latency>& invalidFingers, const bool mustPrint) {
//DEBUG
string timeStr = sc_time_stamp().to_string();
string nameStr = name();
//DEBUG        
        

        string str;
        static sc_time lastCallTime = SC_ZERO_TIME;
        static bool doPrint = true;
        uint i = 0;


        if (lastCallTime != sc_time_stamp()) {
            doPrint = true;
        }

        if (mustPrint)
            doPrint = true;

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

            bool isInvalidVerify = false;
            if (m_verifyMode != NO_VERIFY)
                isInvalidVerify = verifySnapshots(addrs);            


            J["active"] = active;

            auto mapIt = m_copySnapshot.begin();
            while (mapIt != m_copySnapshot.end())
            {
                node_snapshot& snapshot = mapIt->second;

                if ((STATE_JOIN       <= *(snapshot.pFiniteState)) &&
                    (MAX_FINITE_STATE >  *(snapshot.pFiniteState))) {
                   
                    //auto ccwIt = snapshot.pCcwFingers->begin();
                    streamsize sz = 8;
                    json j;                    
                    {
                        j["node_id"] = snapshot.nodeAddr.toStrIDonly().erase(0, 1);


                        if (snapshot.pCwFingers->size() == 0) {
                            j["succ"] = "null";
                            j[" cw fing"] = "null";
                        }
                        else {
                            auto cwIt = snapshot.pCwFingers->begin();
                            if (m_showMotive == true) {
                                j["succ"] = to_string(snapshot.pSuccessor->id.to_uint64()) + LOG_SPACE + snapshot.pSuccessor->motive + LOG_SPACE + snapshot.pSuccessor->updateTime.to_string();
                            }
                            else {
                                j["succ"] = snapshot.pSuccessor->id.to_uint64();
                            }


                            vector<node_address>::iterator refIt;
                            if (m_verifyMode != NO_VERIFY)
                                refIt = m_id2refCwFingers.find(snapshot.nodeAddr.id)->second.begin();
                            

                            uint168 mask = 1;
                            uint160 expected;
                            uint i = 0;
                            while (cwIt != snapshot.pCwFingers->end()) {
                                expected = snapshot.nodeAddr.id + mask;
                                stringstream ss, tmp;
                                tmp << snapshot.nodeAddr.id.to_string(SC_DEC) << "+" << mask.to_string(SC_DEC) << "=" << expected.to_string(SC_DEC);
                                ss << " cw fing[" << i << ": "; ss << setw(sz) << setiosflags(ios::left) << tmp.str(); ss << ("]"); str = ss.str();
                                
                                string str2;
                                if (m_showMotive == true) {
                                    if (m_verifyMode != NO_VERIFY)
                                        str2 = cwIt->id.to_string(SC_DEC) + string(" (") + refIt->id.to_string(SC_DEC) + string(") ") + cwIt->motive + LOG_SPACE + cwIt->updateTime.to_string();
                                    else
                                        str2 = cwIt->id.to_string(SC_DEC) + LOG_SPACE + cwIt->motive + LOG_SPACE + cwIt->updateTime.to_string();
                                    j[str] = str2;                                   
                                }
                                else {
                                    if (m_verifyMode != NO_VERIFY)
                                        str2 = cwIt->id.to_string(SC_DEC) + string(" (") + refIt->id.to_string(SC_DEC) + string(") ");
                                    else 
                                        str2 = cwIt->id.to_string(SC_DEC);
                                    j[str] = str2;                                    
                                }
                                if (m_verifyMode != NO_VERIFY) 
                                    refIt++;
                                cwIt++;
                                mask *= 2;
                                i++;
                            }
                        }

                        if (snapshot.pCcwFingers->size() == 0) {
                            j["pred"] = "null";
                            j["ccw fing"] = "null";
                        }
                        else {
                            auto ccwIt = snapshot.pCcwFingers->begin();
                            if (m_showMotive == true) {
                                j["pred"] = to_string(snapshot.pPredecessor->id.to_uint64()) + LOG_SPACE + snapshot.pPredecessor->motive + LOG_SPACE + snapshot.pPredecessor->updateTime.to_string();
                            }
                            else {
                                j["pred"] = snapshot.pPredecessor->id.to_uint64();
                            }
                            
                            vector<node_address>::iterator refIt;
                            if (m_verifyMode != NO_VERIFY)
                                refIt = m_id2refCcwFingers.find(snapshot.nodeAddr.id)->second.begin();
                            uint168 mask = 1;
                            uint160 expected;
                            uint i = 0;
                            if (m_showCcwFingers) {
                                while (ccwIt != snapshot.pCcwFingers->end()) {
                                    expected = snapshot.nodeAddr.id - mask;
                                    stringstream ss, tmp;
                                    tmp << snapshot.nodeAddr.id.to_string(SC_DEC) << "-" << mask.to_string(SC_DEC) << "=" << expected.to_string(SC_DEC);
                                    ss << "ccw fing[" << i << " : "; ss << setw(sz) << setiosflags(ios::left) << tmp.str(); ss << ("]"); str = ss.str();
                                
                                    string str2;
                                    if (m_showMotive == true) {
                                        if (m_verifyMode != NO_VERIFY)
                                            str2 = ccwIt->id.to_string(SC_DEC) + string(" (") + refIt->id.to_string(SC_DEC) + string(") ") + ccwIt->motive + LOG_SPACE + ccwIt->updateTime.to_string();
                                        else
                                            str2 = ccwIt->id.to_string(SC_DEC) + LOG_SPACE + ccwIt->motive + LOG_SPACE + ccwIt->updateTime.to_string();
                                        j[str] = str2;                                                   
                                    }
                                    else {
                                        if (m_verifyMode != NO_VERIFY)
                                            str2 = ccwIt->id.to_string(SC_DEC) + string(" (") + refIt->id.to_string(SC_DEC) + string(") ");
                                        else 
                                            str2 = ccwIt->id.to_string(SC_DEC);
                                        j[str] = str2;
                                    }
                                    if (m_verifyMode != NO_VERIFY)
                                        refIt++;
                                    ccwIt++;
                                    mask *= 2;
                                    i++;
                                }
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


    void monitor::setSimTime(const sc_time simTime) {
        m_simTime = simTime;
        m_eventMakeSnapshotFinal.notify(simTime-sc_time(1, SC_MS));
    }
}