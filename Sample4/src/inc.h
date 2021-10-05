#ifndef __INC_H__
#define __INC_H__   //tpl = transport plus layer

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <map>
#include <string>

#include <systemc.h>

using namespace std;

namespace P2P_MODEL {

    typedef sc_biguint<160>    uint160;
    typedef unsigned int       uint;
    typedef unsigned long long ulong;
    typedef uint               data_size_type;
    typedef vector<char>       data_type;

    
    enum payload_type {
        K_BLOCK = 0,
        S_BLOCK,
        M_BLOCK,
        DATA,
        PAYLOAD_UNKNOWN
    };


    enum random_type {
        RAND_PERIOD = 0,
        RAND_FIRST_DELAY,
        RAND_PERIOD_TAIL,
        RAND_DATA_SIZE,
        RAND_DEST,
        MAX_RAND_TYPE,
        RAND_UNKNOWN
    };


    enum sim_request_type {
        SIM_HARD_RESET = 0,
        SIM_SOFT_RESET,
        SIM_FLUSH,
        SIM_SINGLE,
        SIM_MULTICAST,
        SIM_BROADCAST,
        SIM_CONTINUE,
        SIM_PAUSE,
        MAX_SIM_REQ_TYPE,
        SIM_UNKNOWN
    };


    enum app_request_type {
        APP_HARD_RESET = 0,
        APP_SOFT_RESET,
        APP_FLUSH,
        APP_SINGLE,
        APP_MULTICAST,
        APP_BROADCAST,
        MAX_APP_REQ_TYPE,
        APP_UNKNOWN
    };

    
    enum chord_request_type {
        CHORD_HARD_RESET = 0,  //TRP is abbrivation "TRansport Plus"
        CHORD_SOFT_RESET,
        CHORD_FLUSH,

        CHORD_JOIN,
        CHORD_NOTIFY,
        CHORD_ACK,
        CHORD_REPLY_FIND_SUCESSOR,
        CHORD_FIND_SUCCESSOR,
        CHORD_FORWARD_BROADCAST,
        CHORD_FORWARD_MULTICAST,
        CHORD_FORWARD_SINGLE,
        CHORD_BROADCAST,
        CHORD_MULTICAST,        
        CHORD_SINGLE,
        MAX_CHORD_REQ_TYPE,
        CHORD_UNKNOWN
    };

         
    class network_address {
    public:
        string ip;
        uint inSocket;
        uint outSocket;

        network_address() {
            reset();
        };

        network_address(const network_address& src) {
            set(src);
        };


        network_address(const string& ip, const uint inSocket, const uint outSocket) {
            set(ip, inSocket, outSocket);
        };


        void reset() {
            ip = "0.0.0.0";
            inSocket = 0;
            outSocket = 0;
        }


        void set(const network_address& src) {
            set(src.ip, src.inSocket, src.outSocket);
        }


        void set(const string ip, const uint inSocket, const uint outSocket) {
            this->ip = ip;
            this->inSocket = inSocket;
            this->outSocket = outSocket;
        }

        network_address& operator= (const network_address& src) {
            if (this == &src)
                return *this;

            set(src.ip, src.inSocket, src.outSocket);
            return *this;
        }

        string toStr() {
            string str;            
            str = "" + ip + " " + to_string(inSocket) + " " + to_string(outSocket);
            return str;
        }

        friend ostream& operator<< (ostream& out, network_address& src);
    };


    class sim_request {
    public:
        vector<network_address> destination;
        sim_request_type type;
        payload_type payloadType;
        uint amount;                    
        sc_time period;
        sc_time firstDelay;        
        bool randType[MAX_RAND_TYPE];        
        uint randFrom[MAX_RAND_TYPE];
        uint randTo[MAX_RAND_TYPE];
        uint randAmount[MAX_RAND_TYPE];
        bool randNeedRecalc[MAX_RAND_TYPE];
        sc_time_unit   timeUnit;
        data_size_type dataSize;

        sim_request() {
            clear();
        }

        sim_request(const sim_request& src) {
            *this = src;
        }

        sim_request& operator= (const sim_request& src) {
            if (this == &src)
                return *this;

            destination = src.destination;
            type = src.type;
            payloadType = src.payloadType;
            amount = src.amount;
            period = src.period;
            firstDelay = src.firstDelay;

            for (int i = 0; i < MAX_RAND_TYPE; ++i) {
                randType[i] = src.randType[i];
                randFrom[i] = src.randFrom[i];
                randTo[i] = src.randTo[i];
                randAmount[i] = src.randAmount[i];
                randNeedRecalc[i] = src.randNeedRecalc[i];
            }

            timeUnit = src.timeUnit;
            dataSize = src.dataSize;            
            return *this;
        }

        void clear() {
            destination.clear();
            type = SIM_UNKNOWN;
            payloadType = PAYLOAD_UNKNOWN;
            amount = 0;
            period = SC_ZERO_TIME;
            firstDelay = SC_ZERO_TIME;

            for (int i = 0; i < MAX_RAND_TYPE; ++i) {
                randType[i] = false;
                randFrom[i] = 0;
                randTo[i] = 0;
                randAmount[i] = 0;
                randNeedRecalc[i] = false;
            }

            timeUnit = SC_MS;
            dataSize = 0;
        }


        string& type2str(const int& type = -1) {
            static string res;
            int t = type;
            if (type == -1)
                t = this->type;

            switch (t) {
            case SIM_HARD_RESET: return res = "HARD_RESET";
            case SIM_SOFT_RESET: return res = "SOFT_RESET";
            case SIM_FLUSH:      return res = "FLUSH";
            case SIM_SINGLE:     return res = "SINGLE";
            case SIM_MULTICAST:  return res = "MULTICAST";
            case SIM_BROADCAST:  return res = "BROADCAST";
            case SIM_PAUSE:      return res = "PAUSE";
            case SIM_CONTINUE:   return res = "CONTINUE";
            default:             return res = "SIM_UNKNOWN";
            }
        }

        string& toStr() {
            static string str;
            str = type2str();
            if ((type == SIM_SINGLE) || (type == SIM_MULTICAST) || (type == SIM_BROADCAST))
                str += " " + to_string(dataSize);

            if (destination.size() > 0) {
                str += " dest: ";
                str += destination.at(0).toStr();
            }
            for (size_t i = 1; i < destination.size(); ++i) {
                str += ", ";
                str += destination.at(i).toStr();
            }
            return str;
        }
    };


    class app_request {
    public:
        vector<network_address> destination;
        uint type;
        data_type payload;

        app_request() {
            clear();
        }

        app_request(const app_request& src) {
            *this = src;
        }

        app_request& operator= (const app_request& src) {
            if (this == &src)
                return *this;

            destination = src.destination;
            type = src.type;
            payload = src.payload;
            return *this;
        }

        void clear() {
            destination.clear();
            type = APP_UNKNOWN;
            payload.clear();
        }

        string& type2str(const int& type = -1) {
            static string res;
            int t = type;
            if (type == -1)
                t = this->type;

            switch (t) {
                case APP_HARD_RESET: return res = "HARD_RESET";
                case APP_SOFT_RESET: return res = "SOFT_RESET";
                case APP_FLUSH:      return res = "FLUSH";
                case APP_SINGLE:     return res = "SINGLE";
                case APP_MULTICAST:  return res = "MULTICAST";
                case APP_BROADCAST:  return res = "BROADCAST";
                default:             return res = "APP_UNKNOWN";
            }
        }

        string& toStr() {
            static string str;
            str = type2str();
            if ((type == APP_SINGLE) || (type == APP_MULTICAST) || (type == APP_BROADCAST))
                str += " " + to_string(payload.size());
           
            if (destination.size() > 0) {
                str += " dest: ";
                str += destination.at(0).toStr();
            }
            for (size_t i = 1; i < destination.size(); ++i) {
                str += ", ";
                str += destination.at(i).toStr();
            }
            return str;
        }

        friend ostream& operator<< (ostream& out, app_request& r);
    };


    class chord_request: public app_request {
    public:
        network_address source;

        chord_request() {
            clear();
        }

        chord_request(const chord_request& src) {
            *this = src;
        }

        chord_request& operator= (const chord_request& src) {
            if (this == &src)
                return *this;

            destination = src.destination;
            type = src.type;
            payload = src.payload;
            source = src.source;
            return *this;
        }

        void clear() {
            app_request::clear();
            type = CHORD_UNKNOWN;
        }

        string& type2str(const int& type = -1) {
            static string res;
            int t = type;
            if (type == -1)
                t = this->type;

            switch (t) {
                case CHORD_HARD_RESET:          return res = app_request::type2str(SIM_HARD_RESET);
                case CHORD_SOFT_RESET:          return res = app_request::type2str(SIM_SOFT_RESET);
                case CHORD_FLUSH:               return res = app_request::type2str(SIM_FLUSH);
                case CHORD_JOIN:                return res = "JOIN";
                case CHORD_NOTIFY:              return res = "NOTIFY";
                case CHORD_ACK:                 return res = "ACK";
                case CHORD_REPLY_FIND_SUCESSOR: return res = "REPLY_FIND_SUCC";
                case CHORD_FIND_SUCCESSOR:      return res = "FIND_SUCC";
                case CHORD_FORWARD_BROADCAST:   return res = "FW_BROADCAST";
                case CHORD_FORWARD_MULTICAST:   return res = "FW_MULTICAST";
                case CHORD_FORWARD_SINGLE:      return res = "FW_SINGLE";
                case CHORD_BROADCAST:           return res = app_request::type2str(SIM_BROADCAST);
                case CHORD_MULTICAST:           return res = app_request::type2str(SIM_MULTICAST);
                case CHORD_SINGLE:              return res = app_request::type2str(SIM_SINGLE);
                default: return res = "CHORD_UNKNOWN";
            }
        }

        string& toStr() {
            static string str;
            str = type2str();
            if ((type == CHORD_SINGLE) || (type == CHORD_MULTICAST) || (type == CHORD_BROADCAST))
                str += " " + to_string(payload.size());
           
            if (destination.size() > 0)  {
                str += " dest: ";
                str += destination.at(0).toStr();
            }
            for (size_t i = 1; i < destination.size(); ++i) {
                str += ", ";
                str += destination.at(i).toStr();
            }
            str += " ";
            str += "src: ";
            str += source.toStr();
            return str;
        }

        friend ostream& operator<< (ostream& out, const chord_request& r);
    };

}
#endif