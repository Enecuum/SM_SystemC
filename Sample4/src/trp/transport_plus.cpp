#include "trp/transport_plus.h"

namespace P2P_MODEL
{
    transport_plus::transport_plus(sc_module_name name): sc_module(name), m_llchord("low_latency_chord")
    {
        m_llchord.trp_port.bind(*this);
    }


    transport_plus::~transport_plus() {    }





    chord_message& transport_plus::appMess2chordMess(const app_message& mess) {
        static chord_message res;
        res.clear();
        res = mess;

        switch (mess.type) {    
        case APP_HARD_RESET: res.type = CHORD_HARD_RESET; break;
        case APP_SOFT_RESET: res.type = CHORD_SOFT_RESET; break;
        case APP_FLUSH:      res.type = CHORD_FLUSH;      break;
        case APP_SINGLE:    res.type = CHORD_SINGLE;      break;
        case APP_MULTICAST: res.type = CHORD_MULTICAST;   break;
        case APP_BROADCAST: res.type = CHORD_BROADCAST;   break;
        default:
            //ERROR
            msgLog(name(), LOG_TX, LOG_ERROR_INDICATOR, "appMess2chordMess", DEBUG_LOG | ERROR_LOG);
            res.clear();
        }
        return res;
    }


    chord_message& transport_plus::chordByteMess2ChordMess(const chord_byte_message& mess) {
        static chord_message res;
        res.clear();
        res = *(mess.fields);
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