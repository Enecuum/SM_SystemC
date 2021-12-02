#ifndef __LLCHORD_CLASSES_DEFS_H__ //LLCHORD is "low lotency chord"
#define __LLCHORD_CLASSES_DEFS_H__

#include "inc.h"

namespace P2P_MODEL {    
    const int ERROR = -1;
    const int NO_ERROR = 0;
    const int BUFFER_NOT_CHOOSEN = -1;

    const sc_time NO_TIMEOUT = SC_ZERO_TIME;
    
    const uint MAX_SIZE_MEMORY_LIST = 100;

    //Default constants to init buffers of low_latency_chord
    const int MAX_SIZE_BUFF_CONFIG = 100;
    const int MAX_SIZE_BUFF_TIMER = 1000;
    const int MAX_SIZE_BUFF_APPTXDATA = 100;
    const int MAX_SIZE_BUFF_RX_MESS = 100;
    const int MAX_SIZE_BUFF_TX_MESS = MAX_SIZE_BUFF_APPTXDATA + MAX_SIZE_BUFF_RX_MESS + 1;

    const int MAX_DEEP_BUFF_CONFIG = MAX_SIZE_BUFF_CONFIG;
    const int MAX_DEEP_BUFF_TIMER = MAX_SIZE_BUFF_TIMER;
    const int MAX_DEEP_BUFF_APPTXDATA = 1;
    const int MAX_DEEP_BUFF_RX_MESS = 1;
    const int MAX_DEEP_BUFF_TX_MESS = MAX_DEEP_BUFF_APPTXDATA + MAX_DEEP_BUFF_RX_MESS + 1;
    
    //Default constants to init timers of low_latency_chord
    const sc_time DEFAULT_TIMEOUT_RX_SUCCESSOR_ON_JOIN = sc_time(20.0, SC_SEC);
    const sc_time DEFAULT_TIMEOUT_RX_SUCCESSOR = DEFAULT_TIMEOUT_RX_SUCCESSOR_ON_JOIN/2;
    const sc_time DEFAULT_TIMEOUT_RX_PREDECESSOR = DEFAULT_TIMEOUT_RX_SUCCESSOR/3;
    const sc_time DEFAULT_TIMEOUT_UPDATE = sc_time(1.0, SC_SEC);
    const sc_time DEFAULT_TIMEOUT_RX_ACK = sc_time(2, SC_SEC);       
    const sc_time DEFAULT_TIMEOUT_RX_DUPLE = 2*DEFAULT_TIMEOUT_RX_SUCCESSOR_ON_JOIN;

    //Default constants to init retry counters of low_latency_chord
    const uint    DEFAULT_COUNTER_TX_JOIN = 1;
    const uint    DEFAULT_COUNTER_TX_FIND_SUCC = 1;
    const uint    DEFAULT_COUNTER_TX_RETRY = 1;
    const uint    DEFAULT_COUNTER_RX_DUPLE = 1;
    const uint    DEFAULT_FINGERS_SIZE = 2;
    const uint    DEFAULT_NEEDS_ACK = 1;   
    const uint    DEFAULT_FILL_FINGERS_MIN_QTY = 0;
    

    //class snapshot {
    //public:
    //    snapshot() {}
    //};


    class bad_finger_with_retry_timer {
    public:
        node_address_latency badFinger;
        chord_timer_message  timer;

        bad_finger_with_retry_timer() {
            badFinger.clear();
            timer.clear();
        }

        bad_finger_with_retry_timer(const bad_finger_with_retry_timer& src) {
            *this = src;
        }

        bad_finger_with_retry_timer(const node_address_latency& badFinger, const chord_timer_message& timer) {
            this->badFinger.setCopy(badFinger);
            this->timer = timer;
        }

        bad_finger_with_retry_timer& operator= (const bad_finger_with_retry_timer& src) {
            if (this == &src)
                return *this;
            this->badFinger.setCopy(src.badFinger);
            this->timer = src.timer;
            return *this;
        }
    };



    class chord_conf_parameters {
    public:
        network_address netwAddr;
        vector<network_address> seed;
        sc_time   TrxSuccOnJoin;
        sc_time   TrxSucc;
        sc_time   TrxPred;
        sc_time   Tupdate;
        sc_time   TrxAck;
        sc_time   TrxDuple;
        uint      CtxJoin;
        uint      CtxFindSucc;
        uint      CtxRetry;
        uint      CrxDuple;
        uint      fingersSize;
        uint      needsACK;
        uint      fillFingersMinQty;

        chord_conf_parameters() {
            clear();
        }

        chord_conf_parameters(const chord_conf_parameters& src) {
            *this = src;
        }

        void clear() {
            seed.clear();
            netwAddr.clear();
            TrxSuccOnJoin = NO_TIMEOUT;
            TrxSucc     = NO_TIMEOUT;
            TrxPred     = NO_TIMEOUT;
            Tupdate         = NO_TIMEOUT;
            TrxAck          = NO_TIMEOUT;
            TrxDuple        = NO_TIMEOUT;
            CtxJoin         = 0;
            CtxFindSucc     = 0;
            CtxRetry          = 0;
            CrxDuple        = 0;
            fingersSize     = 1;
            needsACK = NO_ACK;
            fillFingersMinQty = 1;
        }

        void setDefaultTimersCountersFingersSize() {
            TrxSuccOnJoin   = DEFAULT_TIMEOUT_RX_SUCCESSOR_ON_JOIN;
            TrxSucc         = DEFAULT_TIMEOUT_RX_SUCCESSOR;
            TrxPred         = DEFAULT_TIMEOUT_RX_PREDECESSOR;
            Tupdate         = DEFAULT_TIMEOUT_UPDATE;
            TrxAck          = DEFAULT_TIMEOUT_RX_ACK;
            TrxDuple        = DEFAULT_TIMEOUT_RX_DUPLE;
            CtxJoin         = DEFAULT_COUNTER_TX_JOIN;
            CtxFindSucc     = DEFAULT_COUNTER_TX_FIND_SUCC;
            CtxRetry          = DEFAULT_COUNTER_TX_RETRY;
            CrxDuple        = DEFAULT_COUNTER_RX_DUPLE;
            fingersSize     = DEFAULT_FINGERS_SIZE;
            needsACK        = DEFAULT_NEEDS_ACK;
            fillFingersMinQty = DEFAULT_FILL_FINGERS_MIN_QTY;
        }

        chord_conf_parameters& operator= (const chord_conf_parameters& src) {
            if (this == &src)
                return *this;
            this->netwAddr = src.netwAddr;
            this->seed = src.seed;
            this->TrxSuccOnJoin = src.TrxSuccOnJoin;
            this->TrxSucc = src.TrxSucc;
            this->TrxPred = src.TrxPred;
            this->Tupdate = src.Tupdate;
            this->TrxAck = src.TrxAck;
            this->TrxDuple = src.TrxDuple;
            this->CtxJoin = src.CtxJoin;
            this->CtxFindSucc = src.CtxFindSucc;
            this->CtxRetry = src.CtxRetry;
            this->CrxDuple = src.CrxDuple;
            this->fingersSize = src.fingersSize;
            this->needsACK = src.needsACK;
            this->fillFingersMinQty = src.fillFingersMinQty;
            return *this;
        }
    };
}

#endif