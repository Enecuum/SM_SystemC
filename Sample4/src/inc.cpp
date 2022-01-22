#include "inc.h"

namespace P2P_MODEL
{
    map<string, uint160> node_address::ip2id;
    uint160 node_address::lastMaxID = 0;

    ostream& operator<< (ostream& out, const network_address& r) {
        out << r.toStr();
        return out;
    }

    bool operator== (const network_address& l, const network_address& r) {
        if ((l.ip == r.ip) && (l.inSocket == r.inSocket) && (l.outSocket == r.outSocket))
            return true;
        return false;
    }

    ostream& operator<< (ostream& out, const node_address& r) {
        out << r.toStr();
        return out;
    }

    bool operator== (const node_address& l, const node_address& r) {
        if (operator==(static_cast<const network_address&>(l), static_cast<const network_address&>(r)) == true) 
            if (l.id == r.id)
                return true;
        return false;
    }

    ostream& operator<< (ostream& out, const node_address_latency& r) {
        out << r.toStr();
        return out;
    }

    bool operator== (const node_address_latency& l, const node_address_latency& r) {
        if (operator==(static_cast<const node_address&>(l), static_cast<const node_address&>(r)) == true)
            if (l.latency == r.latency)
                return true;
        return false;
    }    
    
    ostream& operator<< (ostream& out, const app_message& r) {
        out << r.toStr();
        return out;
    }

    ostream& operator<< (ostream& out, const chord_conf_message& r) {
        out << r.toStr();
        return out;
    }

    ostream& operator<< (ostream& out, const chord_apptx_message& r) {
        out << r.toStr();
        return out;
    }

    ostream& operator<< (ostream& out, const chord_timer_message& r) {
        out << r.toStr();
        return out;
    }

    ostream& operator<< (ostream& out, const chord_bits_flags& r) {
        out << r.toStr();
        return out;
    }

    ostream& operator<< (ostream& out, const chord_byte_message_fields& r) {
        out << r.toStr();
        return out;
    }

    ostream& operator<< (ostream& out, const chord_byte_message& r) {
        out << r.toStr();
        return out;
    }

    ostream& operator<< (ostream& out, chord_message& r) {
        out << r.toStr();
        return out;
    }


    uint genRand(uint Min, uint Max)
    {
        static bool first = false;
        if (first == false) {
            srand(static_cast<unsigned int>(time(0)));
            first = true;
        }
        return ((uint)(rand() * (Max + 1 - Min) / RAND_MAX + Min));
    }

    string decToHex(const uint dec) {
        char buff[100];
        _itoa_s(dec, buff, 100, 16);
        return string(buff);
    }
}