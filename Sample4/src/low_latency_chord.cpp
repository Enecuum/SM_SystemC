#include "low_latency_chord.h"

namespace P2P_MODEL
{
    ostream& operator<< (ostream& out, const node_address& r) {
        out << "node_address: ip " << r.ip << ", out socket " << r.outSocket << ", in socket " << r.inSocket << ", id: " << r.id.to_string(SC_HEX_US);
        return out;
    }


    //low_latency_chord::low_latency_chord(sc_module_name _name): sc_module(_name) {
    low_latency_chord::low_latency_chord() {

    }


    low_latency_chord::~low_latency_chord() {    }


    void low_latency_chord::set_network_address(const network_address& _netwAddr) {
        m_netwAddr = _netwAddr;
        m_nodeAddr.set(_netwAddr);
    }

    network_address& low_latency_chord::get_network_address() {
        return m_netwAddr;
    }

    node_address& low_latency_chord::get_node_address() {
        return m_nodeAddr;    
    }
}