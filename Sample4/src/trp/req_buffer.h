#ifndef __REQ_BUFFER_H__
#define __REQ_BUFFER_H__

#include "inc.h"



using namespace std;



namespace P2P_MODEL
{
    typedef list<chord_request> req_buff_container;

    enum buffer_type {
        BUFF_CONFIG_REQ = 0,
        BUFF_TIMER_REQ,
        BUFF_MESS_REQ,
        BUFF_RX_MESS,

        BUFF_TX_JOIN,
        BUFF_TX_NOTIFY,
        BUFF_TX_ACK,
        BUFF_TX_REPLY_FIND_SUCCESSOR,
        BUFF_TX_FIND_SUCCESSOR,
        BUFF_TX_FWD_BROADCAST,
        BUFF_TX_FWD_MULTICAST,
        BUFF_TX_FWD_SINGLE,
        BUFF_TX_BROADCAST,
        BUFF_TX_MULTICAST,
        BUFF_TX_SINGLE,
        
        MAX_BUFF_TYPE,
        BUFF_UNKNOWN
    };


    class req_buffer {
    private:
        req_buff_container reqs;
        int maxDeep;
        int priority;
        bool immediate;
        int maxSize;
        int reqsCounter;
        buffer_type type;       
        sc_time lastCallTime;
        

    public:
        req_buffer() {
            clear();
        }

        ~req_buffer() {            
        }

        req_buffer(const req_buffer& src) {
            *this = src;
        }



        void clearReqs() {
            reqs.clear();            
            reqsCounter = 0;
            lastCallTime = SC_ZERO_TIME;
        }

        void set(const buffer_type type, const int maxDeep, const int maxSize, const bool immediate, const int priority) {
            this->type = type;
            this->maxDeep = maxDeep;
            this->priority = priority;
            this->immediate = immediate;
            this->maxSize = maxSize;
        }

        bool push(const chord_request& r) {
            if (reqs.size() < maxSize) {
                reqs.push_back(r);
                return true;
            }
            return false;
        }

        uint size() {
            return (uint) reqs.size();
        }    
        

    public:
        uint buffType() {
            return type;
        }

        chord_request& firstReqByImmediate() {
            static chord_request r;
            req_buff_container::iterator it = firstReqIteratorByImmediate();
            if (it == reqs.end())
                return r;
            return *it;
        }

        chord_request* firstReqPointerByImmediate() {
            req_buff_container::iterator it = firstReqIteratorByImmediate();
            if (it == reqs.end())
                return nullptr;
            return &(*it);
        }

        void erase(const req_buff_container::iterator& it) {
            reqs.erase(it);
        }


        void eraseFirstReq() {
            if (reqs.size() > 0)
                erase(reqs.begin());
        }

        void clearReqsCounter() {
            reqsCounter = 0;
        }


        bool operator < (const req_buffer& right) const {
            //priority "1" and immediate "true"  is the highest priority to service requests
            // ...
            //priority "5" and immediate "false" is the  lowest priority to service requests
            if (immediate != right.immediate) {
                if (!(right.immediate == true)) return true;
                else return false;
            }
            else
                return (priority < right.priority);
        }

        req_buffer& operator= (const req_buffer& src) {
            if (this == &src)
                return *this;
            this->type = src.type;
            this->reqs = src.reqs;
            this->maxDeep = src.maxDeep;
            this->priority = src.priority;
            this->immediate = src.immediate;
            this->maxSize = src.maxSize;
            this->reqsCounter = src.reqsCounter;
            return *this;
        }

        string& toStr() {
            static string str;
            str.clear();
            string typeStr;
            switch (type) {
                case BUFF_CONFIG_REQ:              typeStr = "BUFF_CONFIG_REQ"; break;
                case BUFF_TIMER_REQ:               typeStr = "BUFF_TIMER_REQ"; break;
                case BUFF_MESS_REQ:                typeStr = "BUFF_MESS_REQ"; break;
                case BUFF_RX_MESS:                 typeStr = "BUFF_RX_MESS"; break;

                case BUFF_TX_JOIN:                 typeStr = "BUFF_TX_JOIN"; break;
                case BUFF_TX_NOTIFY:               typeStr = "BUFF_TX_NOTIFY"; break;
                case BUFF_TX_ACK:                  typeStr = "BUFF_TX_ACK"; break;
                case BUFF_TX_REPLY_FIND_SUCCESSOR: typeStr = "BUFF_TX_REPLY_FIND_SUCCESSOR"; break;
                case BUFF_TX_FIND_SUCCESSOR:       typeStr = "BUFF_TX_FIND_SUCCESSOR"; break;
                case BUFF_TX_FWD_BROADCAST:        typeStr = "BUFF_TX_FWD_BROADCAST"; break;
                case BUFF_TX_FWD_MULTICAST:        typeStr = "BUFF_TX_FWD_MULTICAST"; break;
                case BUFF_TX_FWD_SINGLE:           typeStr = "BUFF_TX_FWD_SINGLE"; break;
                case BUFF_TX_BROADCAST:            typeStr = "BUFF_TX_BROADCAST"; break;
                case BUFF_TX_MULTICAST:            typeStr = "BUFF_TX_MULTICAST"; break;
                case BUFF_TX_SINGLE:               typeStr = "BUFF_TX_SINGLE"; break;
                //case BUFF_TX_MESS:    typeStr = "BUFF_TX_MESS"; break;
                default:              typeStr = "BUFF_UNKNOWN"; break;
            }
            str = typeStr + "\timmediate\t" + string(immediate == true ? "true" : "false") + "\tpri\t" + to_string(priority);
            return str;
        }

    private:
        void clear() {
            reqs.clear();
            type = BUFF_UNKNOWN;
            maxDeep = 0;
            priority = 0;
            immediate = false;
            maxSize = 0;
            reqsCounter = 0;
            lastCallTime = SC_ZERO_TIME;
        }

        req_buff_container::iterator firstReqIteratorByImmediate() {
            req_buff_container::iterator it = reqs.end();

            if (reqs.size() > 0)
            {
                if (sc_time_stamp() != lastCallTime)
                    reqsCounter = 0;

                if (immediate == true) {

                    it = reqs.begin();
                    lastCallTime = sc_time_stamp();
                    ++reqsCounter;
                }
                else {
                    if ((reqsCounter < maxDeep) && (reqsCounter < maxSize)) {
                        it = reqs.begin();
                        lastCallTime = sc_time_stamp();
                        ++reqsCounter;
                    }
                }
            }
            return it;
        }
    };

}
#endif