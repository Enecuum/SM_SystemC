#ifndef __TRP_LLCHORD_IF_H__
#define __TRP_LLCHORD_IF_H__   

#include "inc.h"


namespace P2P_MODEL {
    class trp_llchord_if : public sc_interface {
    public:        
        virtual void send_mess(const chord_byte_message&) = 0;
    };
}

#endif