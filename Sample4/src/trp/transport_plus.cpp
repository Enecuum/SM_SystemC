#include "trp/transport_plus.h"

namespace P2P_MODEL
{
    transport_plus::transport_plus(sc_module_name name):
    sc_module(name), m_llchord("low_latency_chord") {

    }


    transport_plus::~transport_plus() {    }


    chord_request& transport_plus::appReq2trpReq(const app_request& req) {
        static chord_request res;
        res.destination = req.destination;
        res.payload     = req.payload;
        
        switch (req.type) {
            case APP_HARD_RESET: res.type = CHORD_HARD_RESET; break;
            case APP_SOFT_RESET: res.type = CHORD_SOFT_RESET; break;
            case APP_FLUSH:      res.type = CHORD_FLUSH; break;

            case APP_SINGLE:    res.type = CHORD_SINGLE;  break;
            case APP_MULTICAST: res.type = CHORD_MULTICAST; break;
            case APP_BROADCAST: res.type = CHORD_BROADCAST; break;
            default:
                msgLog(name(), LOG_TX, LOG_ERROR_INDICATOR, "appReq2trpReq", DEBUG_LOG | ERROR_LOG | EXTERNAL_LOG);
                break;
        }

        res.source = getNetworkAddress();
        return res;
    }


    void transport_plus::config_req(const app_request& req) {

        m_llchord.pushNewRequest(appReq2trpReq(req));

    }


    void transport_plus::mess_req(const app_request& req) {
        m_llchord.pushNewRequest(appReq2trpReq(req));
    }


    void transport_plus::send_mess() {

    }


    void transport_plus::receive_mess() {

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


    void transport_plus::setSeedNodes(const vector<network_address> seed) {
        m_llchord.setSeedNodes(seed);
    }
}