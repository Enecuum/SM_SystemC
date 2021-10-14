#ifndef __LLCHORD_CLASSES_DEFS_H__
#define __LLCHORD_CLASSES_DEFS_H__

#include "inc.h"

namespace P2P_MODEL {

    const int MAX_SIZE_BUFF_CONFIG_REQ = 100;
    const int MAX_SIZE_BUFF_TIMER_REQ = 10000;
    const int MAX_SIZE_BUFF_MESS_REQ = 100;
    const int MAX_SIZE_BUFF_RX_MESS = 100;
    const int MAX_SIZE_BUFF_TX_MESS = MAX_SIZE_BUFF_MESS_REQ + MAX_SIZE_BUFF_RX_MESS + 1;

    const int MAX_DEEP_BUFF_CONFIG_REQ = MAX_SIZE_BUFF_CONFIG_REQ;
    const int MAX_DEEP_BUFF_TIMER_REQ = MAX_SIZE_BUFF_TIMER_REQ;
    const int MAX_DEEP_BUFF_MESS_REQ = 1;
    const int MAX_DEEP_BUFF_RX_MESS = 1;
    const int MAX_DEEP_BUFF_TX_MESS = MAX_DEEP_BUFF_MESS_REQ + MAX_DEEP_BUFF_RX_MESS + 1;

    const int ERROR = -1;
    const int BUFFER_NOT_CHOOSEN = -1;

    const sc_time DEFAULT_TIMOUT_RXFINDSUCCJOIN = sc_time(15, SC_SEC);
    const sc_time DEFAULT_TIMOUT_RXFINDSUCC = sc_time(5, SC_SEC);
    const sc_time DEFAULT_TIMOUT_UPDATE = sc_time(1, SC_SEC);
    const sc_time DEFAULT_TIMEOUT_RXACK = sc_time(300, SC_MS);
    const sc_time DEFAULT_TIMEOUT_RXDUPLE = sc_time(2 * DEFAULT_TIMOUT_RXFINDSUCCJOIN.to_seconds(), SC_SEC);
    const uint    DEFAULT_COUNTER_TXJOIN = 1;
    const uint    DEFAULT_COUNTER_TXFINDSUCC = 1;
    const uint    DEFAULT_COUNTER_TXACK = 1;
    const uint    DEFAULT_COUNTER_RXDUPLE = 1;
    const uint    DEFAULT_FINGERS_ZIZE = 32;
    const sc_time NO_TIMEOUT = SC_ZERO_TIME;

    

    class chord_conf_parameters {
    public:
        network_address netwAddr;
        vector<network_address> seed;
        sc_time   TrxFindSuccJoin;
        sc_time   TrxFindSucc;
        sc_time   Tupdate;
        sc_time   TrxAck;
        sc_time   TrxDuple;
        uint      CtxJoin;
        uint      CtxFindSucc;
        uint      CtxAck;
        uint      CrxDuple;
        uint      fingersSize;

        chord_conf_parameters() {
            clear();
        }

        chord_conf_parameters(const chord_conf_parameters& src) {
            *this = src;
        }

        void clear() {
            seed.clear();
            netwAddr.clear();
            TrxFindSuccJoin = NO_TIMEOUT;
            TrxFindSucc = NO_TIMEOUT;
            Tupdate = NO_TIMEOUT;
            TrxAck = NO_TIMEOUT;
            TrxDuple = NO_TIMEOUT;
            CtxJoin = 0;
            CtxFindSucc = 0;
            CtxAck = 0;
            CrxDuple = 0;
            fingersSize = 1;
        }

        void setDefaultTimersCountersFingersSize() {
            TrxFindSuccJoin = DEFAULT_TIMOUT_RXFINDSUCCJOIN;
            TrxFindSucc = DEFAULT_TIMOUT_RXFINDSUCC;
            Tupdate = DEFAULT_TIMOUT_UPDATE;
            TrxAck = DEFAULT_TIMEOUT_RXACK;
            TrxDuple = DEFAULT_TIMEOUT_RXDUPLE;
            CtxJoin = DEFAULT_COUNTER_TXJOIN;
            CtxFindSucc = DEFAULT_COUNTER_TXFINDSUCC;
            CtxAck = DEFAULT_COUNTER_TXACK;
            CrxDuple = DEFAULT_COUNTER_RXDUPLE;
            fingersSize = DEFAULT_FINGERS_ZIZE;
        }

        chord_conf_parameters& operator= (const chord_conf_parameters& src) {
            if (this == &src)
                return *this;
            this->netwAddr = src.netwAddr;
            this->seed = src.seed;
            this->TrxFindSuccJoin = src.TrxFindSuccJoin;
            this->TrxFindSucc = src.TrxFindSucc;
            this->Tupdate = src.Tupdate;
            this->TrxAck = src.TrxAck;
            this->TrxDuple = src.TrxDuple;
            this->CtxJoin = src.CtxJoin;
            this->CtxFindSucc = src.CtxFindSucc;
            this->CtxAck = src.CtxAck;
            this->CrxDuple = src.CrxDuple;
            this->fingersSize = src.fingersSize;
            return *this;
        }
    };
}

#endif