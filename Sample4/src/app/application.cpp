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

        SC_METHOD(generatePause);
        dont_initialize();
        sensitive << m_eventGeneratePause;

        SC_METHOD(generateContinue);
        dont_initialize();
        sensitive << m_eventGenerateContinue;

        SC_METHOD(generateConfigurating);
        dont_initialize();
        sensitive << m_eventGenerateConfigurating;

        SC_METHOD(run);        
        sensitive << m_eventStart;  
        
        m_reqs.resize(MAX_SIM_MESS_TYPE, vector<sim_message>());
        m_sentReqCounter.resize(MAX_SIM_MESS_TYPE, 0);
        m_isTriggeredReq.resize(MAX_SIM_MESS_TYPE, false);
        m_isPaused = false;        
    }


    application::~application() {    }



    void application::run() {  
//DEBUG
        if (string(name()) == string("app800"))
            int tmp = 0;

        for (int i = 0; i < m_reqs.size(); ++i) {
            if (m_reqs[i].size() > 0) {
                vector<sim_message>::iterator r = m_reqs[i].begin();
                switch (r->type)
                {
                case SIM_HARD_RESET: {
                    string time = r->firstDelay.to_string();
                    m_eventGenerateHardReset.notify(r->firstDelay);                    
                    }
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
                    //if (r->firstDelay == SC_ZERO_TIME)
                    //    m_isPaused = true;
                    //else
                        m_eventGeneratePause.notify(r->firstDelay);
                    break;

                case SIM_CONTINUE:
                    //if (r->firstDelay == SC_ZERO_TIME)
                    //    m_isPaused = false;
                    //else
                        m_eventGenerateContinue.notify(r->firstDelay);
                    break;

                default:
                    //ERROR
                    m_logText = "run" + LOG_TAB + LOG_ERROR_NOT_RECOGNIZED;
                    msgLog(name(), LOG_TX, LOG_ERROR, m_logText, ALL_LOG);
                    return;
                }
            }
        }
    }


    application::FP application::mess2generateFunc(const sim_message_type& type) {
        if (arrFunctions.size() == 0) {
            arrFunctions.resize(MAX_SIM_MESS_TYPE, nullptr);
            arrFunctions[SIM_HARD_RESET] = &application::generateHardReset;
            arrFunctions[SIM_SOFT_RESET] = &application::generateSoftReset;
            arrFunctions[SIM_FLUSH]      = &application::generateFlush;
            arrFunctions[SIM_SINGLE]     = &application::generateSingle;
            arrFunctions[SIM_MULTICAST]  = &application::generateMulticast;
            arrFunctions[SIM_BROADCAST]  = &application::generateBroadcast;
            arrFunctions[SIM_CONTINUE] = &application::generateContinue;
            arrFunctions[SIM_PAUSE] = &application::generatePause;
            arrFunctions[SIM_CONF] = &application::generateConfigurating;
        }

        switch (type) {
        case SIM_HARD_RESET:
            return arrFunctions[SIM_HARD_RESET];

        case SIM_SOFT_RESET:
            return arrFunctions[SIM_SOFT_RESET];

        case SIM_FLUSH:
            return arrFunctions[SIM_FLUSH];

        case SIM_SINGLE:
            return arrFunctions[SIM_SINGLE];

        case SIM_MULTICAST: 
            return arrFunctions[SIM_MULTICAST];

        case SIM_BROADCAST:
            return arrFunctions[SIM_BROADCAST];

        case SIM_CONTINUE:
            return arrFunctions[SIM_CONTINUE];

        case SIM_PAUSE:
            return arrFunctions[SIM_PAUSE];

        case SIM_CONF:
            return arrFunctions[SIM_CONF];

        default:
            return nullptr;
        }
    }


    sc_event* application::mess2generateEvent(const sim_message_type& type) {        
        switch (type) {
        case SIM_HARD_RESET:
            return &m_eventGenerateHardReset;

        case SIM_SOFT_RESET:
            return &m_eventGenerateSoftReset;

        case SIM_FLUSH:
            return &m_eventGenerateFlush;

        case SIM_SINGLE:
            return &m_eventGenerateSingle;

        case SIM_MULTICAST:
            return &m_eventGenerateMulticast;

        case SIM_BROADCAST:
            return &m_eventGenerateBroadcast;

        case SIM_CONTINUE:
            return &m_eventGenerateContinue;

        case SIM_PAUSE:
            return &m_eventGeneratePause;

        case SIM_CONF:
            return &m_eventGenerateConfigurating;

        default:
            return nullptr;
        }
    }


    void application::generateMess(const sim_message_type& type) {
        if (type == SIM_CONTINUE)
            m_isPaused = false;

        if (m_isPaused) {
            m_isTriggeredReq[type] = true;
        }
        else { 
            if (type == SIM_PAUSE) {
                m_isPaused = true;
            }
            else if (type == SIM_CONTINUE){
                if (m_isPaused == true) {
                    m_isPaused = false;
                    for (uint i = 0; i < m_isTriggeredReq.size(); ++i) {
                        if (m_isTriggeredReq[i] == true) {
                            m_isTriggeredReq[i] = false;
                            sc_event* e;
                            e = mess2generateEvent((sim_message_type) i);
                            if (e != nullptr)
                                e->notify(0, SC_MS);
                        }
                    }
                }
            }

            if (!(type < m_reqs.size())) {
                //ERROR
                m_logText = "generateMess" + LOG_TAB + LOG_ERROR_NOT_RECOGNIZED + LOG_SPACE + sim_message().type2str(type);
                msgLog(name(), LOG_TX, LOG_ERROR, m_logText, ALL_LOG);
                return;
            }

            vector<sim_message>::iterator currReq = m_reqs[type].begin();

            if (currReq->type != type) {
                //ERROR
                m_logText = "generateMess" + LOG_TAB + LOG_ERROR_NOT_RECOGNIZED + LOG_SPACE + sim_message().type2str(type);
                msgLog(name(), LOG_TX, LOG_ERROR, m_logText, ALL_LOG);
                return;
            }

            sim_message s = doRandSimMess(*currReq);
            app_message a = createAppMess(s);

            switch (type) {
            case SIM_HARD_RESET: 
            case SIM_SOFT_RESET:
            case SIM_FLUSH: 
            case SIM_PAUSE:
            case SIM_CONTINUE:
            case SIM_CONF:
                trp_port->config_req(a);
                break;
                
            case SIM_SINGLE: 
            case SIM_MULTICAST:
            case SIM_BROADCAST:
                trp_port->mess_req(a);                
                break;

            default:
                //ERROR
                m_logText = "generateMess" + LOG_TAB + LOG_ERROR_NOT_RECOGNIZED + LOG_SPACE + currReq->type2str();
                msgLog(name(), LOG_TX, LOG_ERROR, m_logText, ALL_LOG);
                return;
            }
            m_sentReqCounter[type]++;

            
            m_logText = "generateMess" + LOG_TAB;
            m_logText +=  to_string(m_sentReqCounter[type]) + "(" + to_string(currReq->amount) + ")";
            m_logText += " " + a.toStr();
            msgLog(name(), LOG_TX, LOG_OUT, m_logText, DEBUG_LOG | EXTERNAL_LOG);
            

            m_isTriggeredReq[type] = false;

            sc_time nextGenerate;
            if (m_sentReqCounter[type] == currReq->amount) {
                m_reqs[type].erase(currReq);
                m_sentReqCounter[type] = 0;

                if (m_reqs[type].size() > 0) {
                    vector<sim_message>::iterator nextReq = m_reqs.at(type).begin();
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
            case SIM_CONF: m_eventGenerateConfigurating.notify(nextGenerate); break;
            default:
                //ERROR        
                m_logText = "generateMess" + LOG_TAB + LOG_ERROR_NOT_RECOGNIZED + LOG_SPACE + currReq->type2str();
                msgLog(name(), LOG_TX, LOG_ERROR, m_logText, ALL_LOG);
                break;
            }
        }
    }


    void application::generateHardReset() {
        generateMess(SIM_HARD_RESET);
    }

    void application::generateSoftReset() {
        generateMess(SIM_SOFT_RESET);
    }

    void application::generateFlush() {
        generateMess(SIM_FLUSH);
    }
    

    app_message_type application::simMessType2appMessType(const uint& type) {
        switch (type)
        {
        case SIM_HARD_RESET: return APP_HARD_RESET;
        case SIM_SOFT_RESET: return APP_SOFT_RESET;
        case SIM_FLUSH:      return APP_FLUSH;
        case SIM_SINGLE:     return APP_SINGLE;
        case SIM_MULTICAST:  return APP_MULTICAST;
        case SIM_BROADCAST:  return APP_BROADCAST;       
        case SIM_PAUSE:      return APP_PAUSE;
        case SIM_CONTINUE:   return APP_CONTINUE;
        case SIM_CONF:       return APP_CONF;
        default:
            //ERROR
            msgLog(name(), LOG_TX, LOG_ERROR, string("simMessType2appMessType") + LOG_ERROR_NOT_RECOGNIZED, ALL_LOG);
            return APP_UNKNOWN;
        }
    }


    sim_message& application::doRandSimMess(const sim_message& s) {
        if (m_randSimReq.size() == 0)
            m_randSimReq.resize(MAX_SIM_MESS_TYPE, sim_message());

        if (m_isFilled.size() == 0)
            m_isFilled.resize(MAX_SIM_MESS_TYPE, false);

        if (!(s.type < MAX_SIM_MESS_TYPE)) {
            //ERROR
            msgLog(name(), LOG_TX, LOG_ERROR, string("doRandSimMess ") + LOG_ERROR_NOT_RECOGNIZED, ALL_LOG);
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
                        m_randSimReq[s.type].period = sc_time(genRand(s.randFrom[i], s.randTo[i]), s.timeUnit);
                    break;
            
                case RAND_FIRST_DELAY:
                    if (m_isFilled[s.type] == false)
                        m_randSimReq[s.type].firstDelay = sc_time(genRand(s.randFrom[i], s.randTo[i]), s.timeUnit);
                    break;

                case RAND_PERIOD_TAIL: 
                    if ((s.randNeedRecalc[i] == true) || (m_isFilled[s.type] == false)) {
                        int sign = genRand(0, 1);
                        if (sign == 1)
                            m_randSimReq[s.type].period += sc_time(genRand(s.randFrom[i], s.randTo[i]), s.timeUnit);
                        else
                            m_randSimReq[s.type].period -= sc_time(genRand(s.randFrom[i], s.randTo[i]), s.timeUnit);
                    } break;

                case RAND_DATA_SIZE:
                    if ((s.randNeedRecalc[i] == true) || (m_isFilled[s.type] == false)) {
                        if ((s.type == SIM_SINGLE) || (s.type == SIM_MULTICAST) || (s.type == SIM_BROADCAST))
                            m_randSimReq[s.type].dataSize = genRand(s.randFrom[i], s.randTo[i]);
                        else 
                            m_randSimReq[s.type].dataSize = 0;
                    } break;

                case RAND_DEST:                     
                    if ((s.randNeedRecalc[i] == true) || (m_isFilled[s.type] == false)) {
                        int amount = 1;
                        if ((s.type == SIM_BROADCAST) || (s.type == SIM_MULTICAST))
                            amount = s.randAmount[i];
                    
                        m_randSimReq[s.type].destNetwAddrs.clear();
                        for (int j = 0; j < amount; ++j) {
                            network_address tmp;
                            tmp.ip  = to_string(genRand(s.randFrom[i], s.randTo[i])); tmp.ip += ".";
                            tmp.ip += to_string(genRand(s.randFrom[i], s.randTo[i])); tmp.ip += ".";
                            tmp.ip += to_string(genRand(s.randFrom[i], s.randTo[i])); tmp.ip += ".";
                            tmp.ip += to_string(genRand(s.randFrom[i], s.randTo[i]));
                            tmp.inSocket  = genRand(s.randFrom[i], s.randTo[i]);
                            tmp.outSocket = genRand(s.randFrom[i], s.randTo[i]);
                            m_randSimReq[s.type].destNetwAddrs.push_back(tmp);
                        }    
                    } break;

                default:
                    //ERROR
                    msgLog(name(), LOG_TX, LOG_ERROR, string("doRandSimMess ") + LOG_ERROR_NOT_RECOGNIZED, ALL_LOG);
                    break;
                }
            }
        }

        if (m_isFilled[s.type] == false)
            m_isFilled[s.type] = true;

        return m_randSimReq[s.type];
    }

    app_message application::createAppMess(const sim_message& s) {
        app_message res;        
        res.clear();

        res.destNetwAddrs = s.destNetwAddrs;        
        res.type = simMessType2appMessType(s.type);
        
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
                msgLog(name(), LOG_TX, LOG_ERROR, "createAppMess " + LOG_ERROR_NOT_RECOGNIZED, ALL_LOG);
                break;
            }
        }
        return res;
    }

    //sc_time application::findDelayToSendNextReq(const sim_message& message, const bool isFinished) {
    //    sc_time t;
    //    if (isFinished) {
    //        t = message.firstDelay;
    //    }
    //    else {
    //        t = sc_time_stamp() + message.period;
    //    }
    //    return t;
    //}



    void application::generateSingle() {
        generateMess(SIM_SINGLE);
    }                                        

    void application::generateMulticast() {
        generateMess(SIM_MULTICAST);
    }


    void application::generateBroadcast() {
        generateMess(SIM_BROADCAST);
    }


    void application::generatePause() {
        generateMess(SIM_PAUSE);
    }


    void application::generateContinue() {
        generateMess(SIM_CONTINUE);
    }

    void application::generateConfigurating() {
        generateMess(SIM_CONF);
    }


    void application::pushSimulatingMess(const sim_message& mess) {
        if (mess.type < MAX_SIM_MESS_TYPE) {
            m_reqs.at(mess.type).push_back(mess);
        }
    }
}