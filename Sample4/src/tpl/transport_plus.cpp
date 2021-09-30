#include "tpl/transport_plus.h"

namespace P2P_MODEL
{
    transport_plus::transport_plus(sc_module_name _name): sc_module(_name) {
        
    }


    transport_plus::~transport_plus() {    }

    
    void transport_plus::configReq() {

    }


    void transport_plus::messReq() {

    }


    void transport_plus::sendMess() {

    }


    void transport_plus::receiveMess() {

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
}