#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "inc.h"
#include "log.h"
#include "trp/trp_application_if.h"

namespace P2P_MODEL
{

    class application: public sc_module,
                       public log
    {
    private:
        vector<sim_message> m_randSimReq;
        vector<bool> m_isFilled;

        sc_event m_eventGenerateHardReset;
        sc_event m_eventGenerateSoftReset;
        sc_event m_eventGenerateFlush;
        sc_event m_eventGenerateSingle;
        sc_event m_eventGenerateMulticast;
        sc_event m_eventGenerateBroadcast;
        sc_event m_eventGeneratePause;
        sc_event m_eventGenerateContinue;
        sc_event m_eventGenerateConfigurating;

        sc_event m_eventStart;
        
        vector< vector<sim_message> > m_reqs;
        vector<int> m_sentReqCounter;

        bool m_isPaused;
        vector<bool> m_isTriggeredReq;
        
        typedef void(application::* FP)();  //FP is function pointer
        vector<FP> arrFunctions;
    
    public:
        sc_port<trp_application_if> trp_port;

    public:

        SC_HAS_PROCESS(application);

        application(sc_module_name _name);
        ~application();

        

        void run();

        void generateHardReset();

        void generateSoftReset();

        void generateFlush();

        void generateSingle();

        void generateMulticast();

        void generateBroadcast();

        void generatePause();

        void generateContinue();
        
        void generateConfigurating();

        void generateMess(const sim_message_type& type);

        FP mess2generateFunc(const sim_message_type& type);

        sc_event* mess2generateEvent(const sim_message_type& type);
        
        void pushSimulatingMess(const sim_message& mess);

        app_message createAppMess(const sim_message& s);

        sim_message& doRandSimMess(const sim_message& s);

        app_message_type simMessType2appMessType(const uint& type);

        
    };
}
#endif