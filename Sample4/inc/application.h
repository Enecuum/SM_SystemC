#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "tpl_inc.h"
#include "log_defs.h"

namespace P2P_MODEL
{
    class application: public sc_module,
                       public MsgLog
    {
    private:

        sc_event m_eventGenerateHardReset;
        sc_event m_eventGenerateSoftReset;
        sc_event m_eventGenerateFlush;
        sc_event m_eventGenerateSingle;
        sc_event m_eventGenerateBroadcast;

        sc_event m_eventStart;


    public:

        SC_HAS_PROCESS(application);

        application(sc_module_name _name);
        ~application();

        void start();

        void generate_hard_reset();

        void generate_soft_reset();

        void generate_flush();

        void generate_single();

        void generate_broadcast();

        
    };
}
#endif