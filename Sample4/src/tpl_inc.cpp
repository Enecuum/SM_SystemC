#include "tpl_inc.h"

namespace P2P_MODEL
{
    

    ostream& operator<< (ostream& out, const network_address& r) {
        out << "network_address: ip " << r.ip << ", out socket " << r.outSocket << ", in socket " << r.inSocket;
        return out;
    }
}