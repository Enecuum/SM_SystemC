#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "inc.h"
#include "log.h"

namespace P2P_MODEL
{

    enum app_request_type {
        HARD_RESET = 0,
        SOFT_RESET,
        FLUSH,
        SINGL,
        MULTICAST,
        BROADCAST,
        START,
        STOP,
        LAST
    };
    

    enum payload_type {
        K_BLOCK = 0,
        S_MBLOCK,
        M_BLOCK,
        DATA
    };

    enum random_type {
        NO_RANDOM = 0,
        RAND_PERIOD,
        RAND_FIRST_DELAY,
        RAND_PERIOD_TAIL,
        RAND_DATA_SIZE
    };


    struct app_request_params {
        vector<network_address> destination;
        app_request_type type;
        payload_type payloadType; 
        uint amount;
        sc_time period;
        sc_time firstDelay;
        random_type randType;
        uint randFrom;
        uint randTo;
        sc_time_unit   timeUnit;
        data_size_type dataSize;
    };


    class application: public sc_module,
                       public log
    {
    private:

        sc_event m_eventGenerateHardReset;
        sc_event m_eventGenerateSoftReset;
        sc_event m_eventGenerateFlush;
        sc_event m_eventGenerateSingle;
        sc_event m_eventGenerateBroadcast;

        sc_event m_eventStart;
        
        vector< vector<app_request_params> >  m_reqs;


    public:

        SC_HAS_PROCESS(application);

        application(sc_module_name _name);
        ~application();

        void run();

        void generateHardReset();

        void generateSoftReset();

        void generateFlush();

        void generateSingle();

        void generateBroadcast();

        
        void pushSimulatingReq(const app_request_params& req);


        
    };
}
#endif