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

        SC_METHOD(generateMulticast);
        dont_initialize();
        sensitive << m_eventGenerateMulticast;

        SC_METHOD(generateBroadcast);
        dont_initialize();
        sensitive << m_eventGenerateBroadcast;

        SC_METHOD(run);        
        sensitive << m_eventStart;  
        
        m_reqs.resize(MAX_SIM_REQ_TYPE, vector<sim_request>());
        m_sentReqCounter.resize(MAX_SIM_REQ_TYPE, 0);
        m_isTriggeredReq.resize(MAX_SIM_REQ_TYPE, false);
        m_isPaused = false;
    }


    application::~application() {    }



    void application::run() {      
        for (int i = 0; i < m_reqs.size(); ++i) {
            if (m_reqs[i].size() > 0) {
                vector<sim_request>::iterator r = m_reqs[i].begin();
                switch (r->type)
                {
                case SIM_HARD_RESET:
                    m_eventGenerateHardReset.notify(r->firstDelay);
                    break;

                case SIM_SOFT_RESET:
                    m_eventGenerateSoftReset.notify(r->firstDelay);
                    break;

                case SIM_FLUSH:
                    m_eventGenerateFlush.notify(r->firstDelay);
                    break;

                case SIM_SINGLE:
                    m_eventGenerateSingle.notify(r->firstDelay);
                    break;

                case SIM_MULTICAST:
                    m_eventGenerateMulticast.notify(r->firstDelay);
                    break;

                case SIM_BROADCAST:
                    m_eventGenerateBroadcast.notify(r->firstDelay);
                    break;

                case SIM_PAUSE:
                    if (r->firstDelay == SC_ZERO_TIME)
                        m_isPaused = true;
                    else
                        m_eventGeneratePause.notify(r->firstDelay);
                    break;

                case SIM_CONTINUE:
                    if (r->firstDelay == SC_ZERO_TIME)
                        m_isPaused = false;
                    else
                        m_eventGenerateContinue.notify(r->firstDelay);
                    break;

                default:
                    //ERROR
                    m_logText = "run" + LOG_SPACER + r->type2str();
                    msgLog(name(), LOG_TX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
                    return;
                }
            }
        }
    }


    void application::generateReq(const sim_request_type& type) {
        if (m_isPaused) {
            m_isTriggeredReq[type] = true;
        }
        else {             
            if (!(type < m_reqs.size())) {
                //ERROR
                m_logText = "generateReq" + LOG_SPACER + sim_request().type2str(type);
                msgLog(name(), LOG_TX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
                return;
            }

            vector<sim_request>::iterator currReq = m_reqs[type].begin();

            if (currReq->type != type) {
                //ERROR
                m_logText = "generateReq" + LOG_SPACER + sim_request().type2str(type);
                msgLog(name(), LOG_TX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
                return;
            }

            sim_request s = doRandSimReq(*currReq);
            app_request a = createAppRequest(s);

            switch (type) {
            case SIM_HARD_RESET: 
            case SIM_SOFT_RESET:
            case SIM_FLUSH: 
                trp_port->config_req(a);
                break;
                
            case SIM_SINGLE: 
            case SIM_MULTICAST:
            case SIM_BROADCAST:
                trp_port->mess_req(a);                
                break;

            default:
                //ERROR
                m_logText = "generateReq" + LOG_SPACER + currReq->type2str();
                msgLog(name(), LOG_TX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
                return;
            }
            m_sentReqCounter[type]++;

            
            m_logText = "generateReq" + LOG_SPACER;
            m_logText +=  to_string(m_sentReqCounter[type]) + "(" + to_string(currReq->amount) + ")";
            m_logText += " " + a.toStr();
            msgLog(name(), LOG_TX, LOG_OUT, m_logText, DEBUG_LOG | EXTERNAL_LOG);
            

            m_isTriggeredReq[type] = false;

            sc_time nextGenerate;
            if (m_sentReqCounter[type] == currReq->amount) {
                m_reqs[type].erase(currReq);
                m_sentReqCounter[type] = 0;

                if (m_reqs[type].size() > 0) {
                    vector<sim_request>::iterator nextReq = m_reqs.at(type).begin();
                    nextGenerate = nextReq->firstDelay;
                }
                else
                    return;
            }
            else {
                nextGenerate = s.period;
            }

            switch (type) {
            case SIM_HARD_RESET: m_eventGenerateHardReset.notify(nextGenerate); break;
            case SIM_SOFT_RESET: m_eventGenerateSoftReset.notify(nextGenerate); break;
            case SIM_FLUSH: m_eventGenerateFlush.notify(nextGenerate); break;
            case SIM_SINGLE: m_eventGenerateSingle.notify(nextGenerate); break;
            case SIM_MULTICAST: m_eventGenerateMulticast.notify(nextGenerate); break;
            case SIM_BROADCAST: m_eventGenerateBroadcast.notify(nextGenerate); break;
            case SIM_PAUSE: m_eventGeneratePause.notify(nextGenerate); break;
            case SIM_CONTINUE: m_eventGenerateContinue.notify(nextGenerate); break;
            default:
                //ERROR        
                m_logText = "generateReq" + LOG_SPACER + currReq->type2str();
                msgLog(name(), LOG_TX, LOG_ERROR_INDICATOR, m_logText, DEBUG_LOG | ERROR_LOG);
                break;
            }
        }
    }


    void application::generateHardReset() {
        generateReq(SIM_HARD_RESET);
    }

    void application::generateSoftReset() {
        generateReq(SIM_SOFT_RESET);
    }

    void application::generateFlush() {
        generateReq(SIM_FLUSH);
    }
    

    app_request_type application::simReqType2appReqType(const uint& type) {
        switch (type)
        {
        case SIM_HARD_RESET: return APP_HARD_RESET;
        case SIM_SOFT_RESET: return APP_SOFT_RESET;
        case SIM_FLUSH:      return APP_FLUSH;
        case SIM_SINGLE:     return APP_SINGLE;
        case SIM_MULTICAST:  return APP_MULTICAST;
        case SIM_BROADCAST:  return APP_BROADCAST;       
        case SIM_CONF:       return APP_CONF;
        default:
            //ERROR
            msgLog(name(), LOG_TX, LOG_ERROR_INDICATOR, "simReqType2appReqType", DEBUG_LOG | ERROR_LOG);
            return APP_UNKNOWN;
        }
    }


    sim_request& application::doRandSimReq(const sim_request& s) {
        if (m_randSimReq.size() == 0)
            m_randSimReq.resize(MAX_SIM_REQ_TYPE, sim_request());

        if (m_isFilled.size() == 0)
            m_isFilled.resize(MAX_SIM_REQ_TYPE, false);

        if (!(s.type < MAX_SIM_REQ_TYPE)) {
            //ERROR
            msgLog(name(), LOG_TX, LOG_ERROR_INDICATOR, "doRandSimReq", DEBUG_LOG | ERROR_LOG);
            m_randSimReq[0].clear();
            return m_randSimReq[0];
        }

        if (m_isFilled[s.type] == false)            
            m_randSimReq[s.type] = s;

        for (uint i = 0; i < MAX_RAND_TYPE; ++i) {
            if (s.randType[i] == true) {
                switch (i) {
                case RAND_PERIOD: 
                    if ((s.randNeedRecalc[i] == true) || (m_isFilled[s.type] == false))
                        m_randSimReq[s.type].period = sc_time(s.randFrom[i] + rand() % s.randTo[i], s.timeUnit);
                    break;
            
                case RAND_FIRST_DELAY:
                    if (m_isFilled[s.type] == false)
                        m_randSimReq[s.type].firstDelay = sc_time(s.randFrom[i] + rand() % s.randTo[i], s.timeUnit);
                    break;

                case RAND_PERIOD_TAIL: 
                    if ((s.randNeedRecalc[i] == true) || (m_isFilled[s.type] == false)) {
                        int sign = rand() % 1;
                        if (sign == 1)
                            m_randSimReq[s.type].period += sc_time(s.randFrom[i] + rand() % s.randTo[i], s.timeUnit);
                        else
                            m_randSimReq[s.type].period -= sc_time(s.randFrom[i] + rand() % s.randTo[i], s.timeUnit);
                    } break;

                case RAND_DATA_SIZE:
                    if ((s.randNeedRecalc[i] == true) || (m_isFilled[s.type] == false)) {
                        if ((s.type == SIM_SINGLE) || (s.type == SIM_MULTICAST) || (s.type == SIM_BROADCAST))
                            m_randSimReq[s.type].dataSize = s.randFrom[i] + rand() % s.randTo[i];
                        else 
                            m_randSimReq[s.type].dataSize = 0;
                    } break;

                case RAND_DEST:                     
                    if ((s.randNeedRecalc[i] == true) || (m_isFilled[s.type] == false)) {
                        int amount = 1;
                        if ((s.type == SIM_BROADCAST) || (s.type == SIM_MULTICAST))
                            amount = s.randAmount[i];
                    
                        m_randSimReq[s.type].destination.clear();
                        for (int j = 0; j < amount; ++j) {
                            network_address tmp;
                            tmp.ip  = to_string(s.randFrom[i] + rand() % s.randTo[i]); tmp.ip += ".";
                            tmp.ip += to_string(s.randFrom[i] + rand() % s.randTo[i]); tmp.ip += ".";
                            tmp.ip += to_string(s.randFrom[i] + rand() % s.randTo[i]); tmp.ip += ".";
                            tmp.ip += to_string(s.randFrom[i] + rand() % s.randTo[i]);
                            tmp.inSocket = s.randFrom[i] + rand() % s.randTo[i];
                            tmp.outSocket = s.randFrom[i] + rand() % s.randTo[i];
                            m_randSimReq[s.type].destination.push_back(tmp);
                        }    
                    } break;

                default:
                    //ERROR
                    msgLog(name(), LOG_TX, LOG_ERROR_INDICATOR, "doRandSimReq", DEBUG_LOG | ERROR_LOG);
                    break;
                }
            }
        }

        if (m_isFilled[s.type] == false)
            m_isFilled[s.type] = true;

        return m_randSimReq[s.type];
    }

    app_request& application::createAppRequest(const sim_request& s) {
        static app_request res;        
        res.clear();

        res.destination = s.destination;        
        res.type = simReqType2appReqType(s.type);
        
        res.payload.clear();
        if ((s.type == SIM_SINGLE) || (s.type == SIM_MULTICAST) || (s.type == SIM_BROADCAST)) {
            switch (s.payloadType) {
            case DATA:                
                res.payloadSize = s.dataSize;
                res.payload.resize(res.payloadSize, 1);
                break;

            case K_BLOCK:
                res.payloadSize = 2025;
                res.payload.resize(res.payloadSize, 1);
                break;

            case M_BLOCK:
                res.payloadSize = 6533;
                res.payload.resize(res.payloadSize, 1);
                break;

            case S_BLOCK:
                res.payloadSize = 558;
                res.payload.resize(res.payloadSize, 1);
                break;

            default:
                //ERROR
                msgLog(name(), LOG_TX, LOG_ERROR_INDICATOR, "createAppRequest", DEBUG_LOG | ERROR_LOG);
                break;
            }
        }
        return res;
    }

    sc_time application::findDelayToSendNextReq(const sim_request& message, const bool isFinished) {
        sc_time t;
        if (isFinished) {
            t = message.firstDelay;
        }
        else {
            t = sc_time_stamp() + message.period;
        }
        return t;
    }



    void application::generateSingle() {
        generateReq(SIM_SINGLE);
    }                                        

    void application::generateMulticast() {
        generateReq(SIM_MULTICAST);
    }


    void application::generateBroadcast() {
        generateReq(SIM_BROADCAST);
    }


    void application::generatePause() {
        msgLog(name(), LOG_TX, LOG_OUT, "generate_pause", DEBUG_LOG | EXTERNAL_LOG);
        m_isPaused = true;
        m_eventGeneratePause.notify(100, SC_MS);
    }


    void application::generateContinue() {
        msgLog(name(), LOG_TX, LOG_OUT, "generate_continue", DEBUG_LOG | EXTERNAL_LOG);
        m_isPaused = false;
        m_eventGenerateContinue.notify(100, SC_MS);
    }


    void application::pushSimulatingReq(const sim_request& req) {        
        if (req.type < MAX_SIM_REQ_TYPE) {
            m_reqs.at(req.type).push_back(req);
        }
    }
}