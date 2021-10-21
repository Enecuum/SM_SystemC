#include "inc.h"

namespace P2P_MODEL
{
    ostream& operator<< (ostream& out, network_address& r) {
        out << r.toStr();
        return out;
    }

    ostream& operator<< (ostream& out, node_address& r) {
        out << r.toStr();
        return out;
    }

    ostream& operator<< (ostream& out, app_message& r) {
        out << r.toStr();
        return out;
    }


    ostream& operator<< (ostream& out, chord_message& r) {
        out << r.toStr();
        return out;
    }
}