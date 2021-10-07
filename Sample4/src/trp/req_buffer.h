#ifndef __REQ_BUFFER_H__
#define __REQ_BUFFER_H__

#include "inc.h"
#include "sha1.hpp"

#include "req_buffer.h"

using namespace std;



namespace P2P_MODEL
{

    enum buffer_type {
        BUFF_CONFIG_REQ = 0,
        BUFF_TIMER_REQ,
        BUFF_MESS_REQ,
        BUFF_RX_MESS,
        BUFF_TX_MESS,
        MAX_BUFF_TYPE,
        BUFF_UNKNOWN
    };


    class req_buffer {
    private:
        vector<chord_request> reqs;
        int maxDeep;
        int priority;
        bool immediate;
        int maxSize;
        int reqCounter;
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

        void clear() {
            reqs.clear();
            type = BUFF_UNKNOWN;
            maxDeep = 0;
            priority = 0;
            immediate = false;
            maxSize = 0;
            reqCounter = 0;            
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

    private:
        vector<chord_request>::iterator firstReqIteratorByImmediate() {
            vector<chord_request>::iterator it = reqs.end();

            if (reqs.size() > 0) 
            {
                if (sc_time_stamp() != lastCallTime)
                    reqCounter = 0;

                if (immediate == true) {
                
                    it = reqs.begin();
                    lastCallTime = sc_time_stamp();
                    ++reqCounter;
                }
                else {
                    if ((reqCounter < maxDeep) && (reqCounter < maxSize)) {
                        it = reqs.begin();
                        lastCallTime = sc_time_stamp();
                        ++reqCounter;                   
                    }
                }
            }
            return it;
        }

    public:
        uint buffType() {
            return type;
        }

        chord_request& firstReqByImmediate() {
            static chord_request r;
            vector<chord_request>::iterator it = firstReqIteratorByImmediate();
            if (it == reqs.end())
                return r;
            return *it;
        }

        chord_request* firstReqPointerByImmediate() {
            vector<chord_request>::iterator it = firstReqIteratorByImmediate();
            if (it == reqs.end())
                return nullptr;
            return &(*it);
        }

        void erase(const vector<chord_request>::iterator& it) {
            reqs.erase(it);
        }


        void eraseFirstReq() {
            if (reqs.size() > 0)
                erase(reqs.begin());
        }

        void resetReqCounter() {
            reqCounter = 0;
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
            this->reqCounter = src.reqCounter;
            return *this;
        }

        string& toStr() {
            static string str;
            str.clear();
            string typeStr;
            switch (type) {
                case BUFF_CONFIG_REQ: typeStr = "BUFF_CONFIG_REQ"; break;
                case BUFF_TIMER_REQ:  typeStr = "BUFF_TIMER_REQ"; break;
                case BUFF_MESS_REQ:   typeStr = "BUFF_MESS_REQ"; break;
                case BUFF_RX_MESS:    typeStr = "BUFF_RX_MESS"; break;
                case BUFF_TX_MESS:    typeStr = "BUFF_TX_MESS"; break;
                default: typeStr = "BUFF_UNKNOWN"; break;
            }
            str = typeStr + "\timmediate\t" + string(immediate == true ? "true" : "false") + "\tpri\t" + to_string(priority);
            return str;
        }
    };


    
    class chord_buffer {
    public:
        vector<req_buffer> buffer;

        chord_buffer() { };


        void setPriority();
    };



    class node_address : public network_address {
    private:
        network_address m_tmp;

    public:
        uint160 id;

        node_address() {
            reset();
        };


        node_address(const node_address& src) {
            set(src);
        }


        node_address(const network_address& src) {
            set(src);
        }


        node_address(const string& ip, uint inSocket, uint outSocket) {
            set(ip, inSocket, outSocket);
        }


        void reset() {
            network_address::reset();
            id = "0xus0";
        }


        void set(const node_address& src) {
            set(src.ip, src.inSocket, src.outSocket);
        }


        void set(const network_address& src) {
            set(src.ip, src.inSocket, src.outSocket);
        }


        void set(const string& ip, const uint inSocket, const uint outSocket) {
            network_address::set(ip, inSocket, outSocket);

            string onlyNumbers = network_address::ip;
            onlyNumbers.erase(remove(onlyNumbers.begin(), onlyNumbers.end(), '.'), onlyNumbers.end());
            onlyNumbers.append(to_string(network_address::inSocket));
            id = sha1(onlyNumbers);
        }


        node_address& operator= (const node_address& src) {
            if (this == &src)
                return *this;

            set(src.ip, src.inSocket, src.outSocket);
            return *this;
        }

        friend ostream& operator<< (ostream& out, const node_address& r);


    private:
        uint160 sha1(const string& str) {
            SHA1 checksum;
            checksum.update(str);
            string strID = checksum.final();
            strID.insert(0, "0xus");
            uint160 res = strID.c_str();
            return res;
        }
    };
}
#endif