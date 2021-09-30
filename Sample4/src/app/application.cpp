#include "app/application.h"

namespace P2P_MODEL
{

    application::application(sc_module_name _name) : sc_module(_name) {
        SC_METHOD(generateHardReset);
        dont_initialize();
        sensitive << m_eventGenerateHardReset;
        
        SC_METHOD(generateSoftReset);
        dont_initialize();
        sensitive << m_eventGenerateSoftReset;

        SC_METHOD(generateFlush);
        dont_initialize();
        sensitive << m_eventGenerateFlush;


        SC_METHOD(generateSingle);
        dont_initialize();
        sensitive << m_eventGenerateSingle;

        SC_METHOD(generateBroadcast);
        dont_initialize();
        sensitive << m_eventGenerateBroadcast;

        SC_METHOD(run);        
        sensitive << m_eventStart;       
    }


    application::~application() {    }



    void application::run() {
        //for (int i = 0; i < )
        //    m_eventGenerateSingle.notify(0, SC_MS);
    }

    void application::generateHardReset() {
        msgLog(name(), LOG_TX, LOG_OUT, "generate_hard_reset", DEBUG_LOG | EXTERNAL_LOG);
        m_eventGenerateSingle.notify(100, SC_MS);
    }

    void application::generateSoftReset() {
        msgLog(name(), LOG_TX, LOG_OUT, "generate_soft_reset", DEBUG_LOG | EXTERNAL_LOG);
        m_eventGenerateSingle.notify(100, SC_MS);
    }

    void application::generateFlush() {
        msgLog(name(), LOG_TX, LOG_OUT, "generate_flush", DEBUG_LOG | EXTERNAL_LOG);
        m_eventGenerateSingle.notify(100, SC_MS);
    }                                      

    void application::generateSingle() {
        msgLog(name(), LOG_TX, LOG_OUT, "generate_single", DEBUG_LOG | EXTERNAL_LOG);
        m_eventGenerateSingle.notify(100, SC_MS);
    }

    void application::generateBroadcast() {
        msgLog(name(), LOG_TX, LOG_OUT, "generate_broadcast", DEBUG_LOG | EXTERNAL_LOG);
        m_eventGenerateSingle.notify(100, SC_MS);
    }


    void application::pushSimulatingReq(const app_request_params& req) {        
        if (req.type < app_request_type::LAST) {
            m_reqs.at(req.type).push_back(req);
        }
    }
}