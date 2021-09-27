#include "application.h"

namespace P2P_MODEL
{

    application::application(sc_module_name _name) : sc_module(_name) {
        SC_METHOD(generate_hard_reset);
        dont_initialize();
        sensitive << m_eventGenerateHardReset;
        
        SC_METHOD(generate_soft_reset);
        dont_initialize();
        sensitive << m_eventGenerateSoftReset;

        SC_METHOD(generate_flush);
        dont_initialize();
        sensitive << m_eventGenerateFlush;


        SC_METHOD(generate_single);
        dont_initialize();
        sensitive << m_eventGenerateSingle;

        SC_METHOD(generate_broadcast);
        dont_initialize();
        sensitive << m_eventGenerateBroadcast;

        SC_METHOD(start);        
        sensitive << m_eventStart;       
    }


    application::~application() {    }



    void application::start() {
        m_eventGenerateSingle.notify(0, SC_MS);
    }

    void application::generate_hard_reset() {
        msgLog(name(), LOG_TX, LOG_OUT, "generate_hard_reset", DEBUG_LOG | EXTERNAL_LOG);
        m_eventGenerateSingle.notify(100, SC_MS);
    }

    void application::generate_soft_reset() {
        msgLog(name(), LOG_TX, LOG_OUT, "generate_soft_reset", DEBUG_LOG | EXTERNAL_LOG);
        m_eventGenerateSingle.notify(100, SC_MS);
    }

    void application::generate_flush() {
        msgLog(name(), LOG_TX, LOG_OUT, "generate_flush", DEBUG_LOG | EXTERNAL_LOG);
        m_eventGenerateSingle.notify(100, SC_MS);
    }                                      

    void application::generate_single() {
        msgLog(name(), LOG_TX, LOG_OUT, "generate_single", DEBUG_LOG | EXTERNAL_LOG);
        m_eventGenerateSingle.notify(100, SC_MS);
    }

    void application::generate_broadcast() {
        msgLog(name(), LOG_TX, LOG_OUT, "generate_broadcast", DEBUG_LOG | EXTERNAL_LOG);
        m_eventGenerateSingle.notify(100, SC_MS);
    }
}