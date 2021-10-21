#ifndef __TRP_APPLICATION_IF_H__
#define __TRP_APPLICATION_IF_H__   //TRP_APPLICATION_IF is a shortening as "TRansport Plus layer with application InterFace"

#include "inc.h"


namespace P2P_MODEL {
    class trp_application_if : public sc_interface {
    public:
        virtual void config_req(const app_message& ) = 0;
        virtual void mess_req(const app_message&   ) = 0;
    };
}

#endif
