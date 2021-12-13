#ifndef __TRP_MONITOR_IF_H__
#define __TRP_MONITOR_IF_H__   //TRP_APPLICATION_IF is a shortening as "TRansport Plus layer with application InterFace"

#include "inc.h"


namespace P2P_MODEL {
    class trp_monitor_if : public sc_interface {
    public:
        virtual const vector<node_address_latency>* cw_fingers_pointer() const = 0;
        virtual const vector<node_address_latency>* ccw_fingers_pointer() const  = 0;
        virtual node_address node_addr() const = 0;
    };
}

#endif
