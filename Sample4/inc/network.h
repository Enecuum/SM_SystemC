#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "tpl_inc.h"
#include "log_defs.h"

namespace P2P_MODEL
{
    class network : public sc_module,
                    public MsgLog
    {
    private:
        sc_event m_eventSend;


    public:

        SC_HAS_PROCESS(network);

        network(sc_module_name _name);
        ~network();

        void send_message();
        void set_latency();
        void set_router_table();

    };
}
#endif