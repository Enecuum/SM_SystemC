#ifndef __MONITOR_TRP_IF_H__
#define __MONITOR_TRP_IF_H__   

#include "inc.h"


namespace P2P_MODEL {
    class monitor_trp_if: public sc_interface {
    public:
        virtual bool check_fingers(const uint160 id, vector<node_address_latency>& invalidFingers) = 0;
    };
}

#endif