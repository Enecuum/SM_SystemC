#include "transport_plus.h"

namespace P2P_MODEL
{
    transport_plus::transport_plus(sc_module_name _name): sc_module(_name) {
        
    }


    transport_plus::~transport_plus() {    }

    
    void transport_plus::config_req() {

    }


    void transport_plus::mess_req() {

    }


    void transport_plus::send_mess() {

    }


    void transport_plus::receive_mess() {

    }


    void transport_plus::set_network_address(const network_address& _netwAddr) {

        m_llchord.set_network_address(_netwAddr);
    }

    network_address& transport_plus::get_network_address() {
        return m_llchord.get_network_address();
    }


    node_address& transport_plus::get_node_address() {
        return m_llchord.get_node_address();
    }
}