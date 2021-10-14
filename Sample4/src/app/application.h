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
        vector<sim_request> m_randSimReq;
        vector<bool> m_isFilled;

        sc_event m_eventGenerateHardReset;
        sc_event m_eventGenerateSoftReset;
        sc_event m_eventGenerateFlush;
        sc_event m_eventGenerateSingle;
        sc_event m_eventGenerateMulticast;
        sc_event m_eventGenerateBroadcast;
        sc_event m_eventGeneratePause;
        sc_event m_eventGenerateContinue;

        sc_event m_eventStart;
        
        vector< vector<sim_request> > m_reqs;
        vector<int> m_sentReqCounter;

        bool m_isPaused;
        vector<bool> m_isTriggeredReq;
    
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

        void generateReq(const sim_request_type& type);

        
        void pushSimulatingReq(const sim_request& req);

        app_request& createAppRequest(const sim_request& s);

        sim_request& doRandSimReq(const sim_request& s);

        app_request_type simReqType2appReqType(const uint& type);

        sc_time findDelayToSendNextReq(const sim_request& message, const bool isFinished);
        
    };
}
#endif