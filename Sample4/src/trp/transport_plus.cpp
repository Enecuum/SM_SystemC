#include "trp/transport_plus.h"

namespace P2P_MODEL
{
    transport_plus::transport_plus(sc_module_name name): sc_module(name), m_llchord("llchord")
    {
        m_llchord.trp_port.bind(*this);
    }


    transport_plus::~transport_plus() {    }





    chord_message transport_plus::appMess2chordMess(const app_message& mess) {
        chord_message res;
        res.clear();
        res = mess;

        switch (mess.type) {    
        case APP_HARD_RESET: res.type = CHORD_HARD_RESET; break;
        case APP_SOFT_RESET: res.type = CHORD_SOFT_RESET; break;
        case APP_FLUSH:      res.type = CHORD_FLUSH;      break;
        case APP_SINGLE:    res.type = CHORD_SINGLE;      break;
        case APP_MULTICAST: res.type = CHORD_MULTICAST;   break;
        case APP_BROADCAST: res.type = CHORD_BROADCAST;   break;
        case APP_CONF:      res.type = CHORD_CONF;        break;
        case APP_PAUSE:     res.type = CHORD_PAUSE;       break;
        case APP_CONTINUE: res.type = CHORD_CONTINUE;     break;
        default:
            //ERROR
            msgLog(name(), LOG_TX, LOG_ERROR, "appMess2chordMess", ALL_LOG);
            res.clear();
        }
        return res;
    }


    chord_message transport_plus::chordByteMess2ChordMess(const chord_byte_message& mess) {
        chord_message res;
        res = mess.fields;
        return res;
    }


    void transport_plus::config_req(const app_message& mess) {
        m_llchord.pushNewMessage( appMess2chordMess(mess) );
    }


    void transport_plus::mess_req(const app_message& mess) {
      
        m_llchord.pushNewMessage( appMess2chordMess(mess) );
    }


    void transport_plus::send_mess(const chord_byte_message& raw) {                
        network_port->push_into_network(raw);
    }


    void transport_plus::receive_mess(const chord_byte_message& raw) {
        m_llchord.pushNewMessage( chordByteMess2ChordMess(raw) );
    }


    const vector<node_address_latency>* transport_plus::cw_fingers_pointer() const {
        return m_llchord.cw_fingers_pointer();
    }


    const vector<node_address_latency>* transport_plus::ccw_fingers_pointer() const {
        return m_llchord.ccw_fingers_pointer();
    }

    
    node_address transport_plus::node_addr() const {
        return m_llchord.node_addr();
    }


    const finite_state* transport_plus::finite_state_pointer() const {
        return m_llchord.finite_state_pointer();
    }


    node_snapshot transport_plus::snapshot_pointers() {
        return m_llchord.snapshot_pointers();
    }


    void transport_plus::check_fingers(const node_address& addr, vector<node_address_latency>& invalidFingers) {
        monitor_port->check_fingers(addr, invalidFingers);
    }

    void transport_plus::check_fingers(const node_snapshot& snapshot, vector<node_address_latency>& invalidFingers) {
        monitor_port->check_fingers(snapshot, invalidFingers);
    }


    void transport_plus::setNetworkAddress(const network_address& netwAddr) {
        m_llchord.setNetworkAddress(netwAddr);
    }

    network_address& transport_plus::getNetworkAddress() {
        return m_llchord.getNetworkAddress();
    }


    node_address& transport_plus::getNodeAddress() {
        return m_llchord.getNodeAddress();
    }


    void transport_plus::setPathLog(const string& pathLog) {
        log::setPathLog(pathLog);
        m_llchord.setPathLog(pathLog);
    }


    void transport_plus::setSnapshotPathLog(const string& pathLog) {
        m_llchord.setSnapshotPathLog(pathLog);
    }


    void transport_plus::setLogMode(const log_mode& mode) {
        log::setLogMode(mode);
        m_llchord.setLogMode(mode);
    }


    void transport_plus::setDisabledLog() {
        log::setDisabledLog();
        m_llchord.setDisabledLog();
    }


    void transport_plus::setEnabledLog() {
        log::setEnabledLog();
        m_llchord.setEnabledLog();
    }


    void transport_plus::setSeedNodes(const vector<network_address>& seed) {
        m_llchord.setSeedNodes(seed);
    }


    void transport_plus::setConfParameters(const chord_conf_parameters& params) {
        m_llchord.setConfParameters(params);
    }
}