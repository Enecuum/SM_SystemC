#ifndef __NETWORK_TRP_IF_H__
#define __NETWORK_TRP_IF_H__   //TRP_APPLICATION_IF is a shortening as "TRansport Plus layer with application InterFace"

#include "inc.h"


namespace P2P_MODEL {
    class network_trp_if : public sc_interface {
    public:
        virtual void push_into_network(const message_info& mess) = 0;
    };
}

#endif