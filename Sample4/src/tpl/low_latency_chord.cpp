#include "tpl/low_latency_chord.h"

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


    void low_latency_chord::setNetworkAddress(const network_address& addr) {
        m_netwAddr = addr;
        m_nodeAddr.set(addr);
    }

    network_address& low_latency_chord::getNetworkAddress() {
        return m_netwAddr;
    }

    node_address& low_latency_chord::getNodeAddress() {
        return m_nodeAddr;    
    }
}