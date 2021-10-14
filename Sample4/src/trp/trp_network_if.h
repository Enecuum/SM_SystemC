#ifndef __TRP_NETWORK_IF_H__
#define __TRP_NETWORK_IF_H__   //TRP_APPLICATION_IF is a shortening as "TRansport Plus layer with application InterFace"

#include "inc.h"


namespace P2P_MODEL {
    class trp_network_if : public sc_interface {
    public:        
        virtual void receive_mess(const message_info& mess) = 0;
    };
}

#endif