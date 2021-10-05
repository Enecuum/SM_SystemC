#include "inc.h"

namespace P2P_MODEL
{
    

    ostream& operator<< (ostream& out, network_address& r) {
        out << r.toStr();
        return out;
    }


    ostream& operator<< (ostream& out, app_request& r) {
        out << r.toStr();
        return out;
    }


    ostream& operator<< (ostream& out, chord_request& r) {
        out << r.toStr();
        return out;
    }
}