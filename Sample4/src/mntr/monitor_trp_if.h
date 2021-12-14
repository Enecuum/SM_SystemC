#ifndef __MONITOR_TRP_IF_H__
#define __MONITOR_TRP_IF_H__   

#include "inc.h"


namespace P2P_MODEL {
    class monitor_trp_if: public sc_interface {
    public:
        virtual void check_fingers(const node_address&,  vector<node_address_latency>& invalidFingers) = 0;
        virtual void check_fingers(const node_snapshot&, vector<node_address_latency>& invalidFingers) = 0;
    };
}

#endif