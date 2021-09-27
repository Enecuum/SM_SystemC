#include "network.h"

namespace P2P_MODEL
{

    network::network(sc_module_name _name) : sc_module(_name) {
        //SC_METHOD(generate_hard_reset);
        //dont_initialize();
        //sensitive << m_eventGenerateHardReset;
    }


    network::~network() {    }


    void network::send_message() {

    }


    void network::set_latency() {

    }


    void network::set_router_table() {

    }
}