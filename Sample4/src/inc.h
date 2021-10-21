#ifndef __INC_H__
#define __INC_H__   //tpl = transport plus layer

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <map>
#include <list>
#include <string>

#include <systemc.h>
#include "trp/sha1.hpp"

using namespace std;

namespace P2P_MODEL {

    typedef sc_biguint<160>    uint160;
    typedef unsigned int       uint;
    typedef unsigned long long ulong;
    typedef uint               data_size_type;
    typedef vector<char>       data_type;
    typedef data_type          data_unit;

    
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


    enum sim_message_type {
        SIM_HARD_RESET = 0,
        SIM_SOFT_RESET,
        SIM_FLUSH,
        SIM_SINGLE,
        SIM_MULTICAST,
        SIM_BROADCAST,
        SIM_CONTINUE,
        SIM_PAUSE,
        SIM_CONF,
        MAX_SIM_MESS_TYPE,
        SIM_UNKNOWN
    };


    enum app_message_type {
        APP_HARD_RESET = 0,
        APP_SOFT_RESET,
        APP_FLUSH,
        APP_SINGLE,
        APP_MULTICAST,
        APP_BROADCAST,
        APP_CONF,
        MAX_APP_MESS_TYPE,
        APP_UNKNOWN
    };

    
    enum chord_message_type {
        CHORD_HARD_RESET = 0,  
        CHORD_SOFT_RESET,
        CHORD_FLUSH,

        CHORD_BROADCAST,
        CHORD_MULTICAST,
        CHORD_SINGLE,

        CHORD_CONF,

        CHORD_RX_JOIN,
        CHORD_RX_NOTIFY,
        CHORD_RX_ACK,
        CHORD_RX_REPLY_FIND_SUCCESSOR,
        CHORD_RX_FIND_SUCCESSOR,
        CHORD_RX_BROADCAST,
        CHORD_RX_MULTICAST,
        CHORD_RX_SINGLE,

        CHORD_TX_JOIN,
        CHORD_TX_NOTIFY,
        CHORD_TX_ACK,
        CHORD_TX_REPLY_FIND_SUCESSOR,
        CHORD_TX_FIND_SUCCESSOR,
        CHORD_TX_FWD_BROADCAST,
        CHORD_TX_FWD_MULTICAST,
        CHORD_TX_FWD_SINGLE,
        CHORD_TX_BROADCAST,
        CHORD_TX_MULTICAST,
        CHORD_TX_SINGLE,

        CHORD_TIMER_ACK,
        CHORD_TIMER_REPLY_FIND_SUCC,
        CHORD_TIMER_REPLY_FIND_SUCC_JOIN,
        CHORD_TIMER_UPDATE,
        
        MAX_CHORD_MESS_TYPE,
        CHORD_UNKNOWN
    };



         
    class network_address {
    public:
        string ip;
        uint inSocket;
        uint outSocket;

        network_address() {
            clear();
        };

        network_address(const network_address& src) {
            set(src);
        };


        network_address(const string& ip, const uint inSocket, const uint outSocket) {
            set(ip, inSocket, outSocket);
        };


        void clear() {
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

            this->ip        = src.ip;
            this->inSocket  = src.inSocket;
            this->outSocket = src.outSocket;
            return *this;
        }

        string& toStr() {
            static string str;            
            str = "" + ip + " " + to_string(inSocket) + " " + to_string(outSocket);
            return str;
        }

        friend ostream& operator<< (ostream& out, network_address& src);
    };


    class node_address : public network_address {
    public:
        uint160 id;

        node_address() {
            clear();
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


        void clear() {
            network_address::clear();
            id = 0;
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
            //onlyNumbers.erase(remove(onlyNumbers.begin(), onlyNumbers.end(), '.'), onlyNumbers.end());
            //onlyNumbers.append(to_string(network_address::inSocket));
            id = sha1(onlyNumbers);
        }


        node_address& operator= (const node_address& src) {
            if (this == &src)
                return *this;

            network_address::operator=(src);
            this->id = src.id;
            return *this;
        }

        string& toStr() {
            static string str;
            str = network_address::toStr() + " " + id.to_string(SC_HEX_US);
            return str;
        }

        friend ostream& operator<< (ostream& out, node_address& r);


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





    class sim_message {
    public:
        vector<network_address> destination;
        sim_message_type type;
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

        sim_message() {
            clear();
        }

        sim_message(const sim_message& src) {
            *this = src;
        }

        sim_message& operator= (const sim_message& src) {
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
                case SIM_CONF:       return res = "CONF";
                default:             return res = "SIM_UNKNOWN";
            }
        }

        string& toStr() {
            static string str;
            str = type2str();
            if ((type == SIM_SINGLE) || (type == SIM_MULTICAST) || (type == SIM_BROADCAST))
            //{
                str += " " + to_string(dataSize);

                if (destination.size() > 0) {
                    str += " dest: ";
                    str += destination.at(0).toStr();
                }
                for (size_t i = 1; i < destination.size(); ++i) {
                    str += ", ";
                    str += destination.at(i).toStr();
                }
            //}
            return str;
        }
    };


    class app_message {
    public:
        vector<network_address> destination;
        uint type;
        data_type payload;
        data_size_type payloadSize;

        app_message() {
            clear();
        }

        app_message(const app_message& src) {
            *this = src;
        }

        app_message& operator= (const app_message& src) {
            if (this == &src)
                return *this;

            destination = src.destination;
            type = src.type;
            payload = src.payload;
            payloadSize = src.payloadSize;
            return *this;
        }

        void clear() {
            destination.clear();
            type = APP_UNKNOWN;
            payload.clear();
            payloadSize = 0;
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
                case APP_CONF:       return res = "CONF";
                default:             return res = "APP_UNKNOWN";
            }
        }

        string& toStr() {
            static string str;
            str = type2str();
            if ((type == APP_SINGLE) || (type == APP_MULTICAST) || (type == APP_BROADCAST))
                str += " " + to_string(payloadSize/*payload.size()*/);           

            if (destination.size() > 0) {
                str += " dest: " + destination.at(0).toStr();                
                for (size_t i = 1; i < destination.size(); ++i) 
                    str += ", " + destination.at(i).toStr();
            }
            return str;
        }

        friend ostream& operator<< (ostream& out, app_message& r);
    };


    class chord_message: public app_message {
    public:
        node_address source;
        node_address mediator;
        uint160 destNodeID;
        sc_time  appearanceTime;
        chord_message* retryMess;

        chord_message(): retryMess(nullptr) {
            clear();
        }

        chord_message(const chord_message& src): retryMess(nullptr) {
            *this = src;
        }

        ~chord_message() {
            clear();
        }

        chord_message& operator= (const chord_message& src) {
            if (this == &src)
                return *this;

            destination    = src.destination;
            type           = src.type;
            payload        = src.payload;
            payloadSize    = src.payloadSize;
            source         = src.source;
            mediator       = src.mediator;

            destNodeID     = src.destNodeID;
            appearanceTime = src.appearanceTime;
            
            if (retryMess != nullptr) {
                delete retryMess;
                retryMess = nullptr;
            }
            if (src.retryMess != nullptr) {
                retryMess = new chord_message();
                (*retryMess).destination    = (*(src.retryMess)).destination;
                (*retryMess).type           = (*(src.retryMess)).type;
                (*retryMess).payload        = (*(src.retryMess)).payload;
                (*retryMess).payloadSize    = (*(src.retryMess)).payloadSize;
                (*retryMess).source         = (*(src.retryMess)).source;
                (*retryMess).mediator       = (*(src.retryMess)).mediator;
                (*retryMess).destNodeID     = (*(src.retryMess)).destNodeID;
                (*retryMess).appearanceTime = (*(src.retryMess)).appearanceTime;;
            }
            //cout << toStr() << endl;
            return *this;
        }

        void clear() {
            app_message::clear();
            type = CHORD_UNKNOWN;
            source.clear();
            mediator.clear();
            destNodeID = 0;
            appearanceTime = SC_ZERO_TIME;            

            if (retryMess != nullptr) {
                delete retryMess;
                retryMess = nullptr;
            }
        }

        chord_message* clone() {
            chord_message* p = nullptr;
            p = new chord_message();
            *p = *this;
            return p;
        }

        string& type2str(const int& type = -1) {
            static string res;
            int t = type;
            if (type == -1)
                t = this->type;

            switch (t) {
                case CHORD_HARD_RESET:             return res = app_message::type2str(APP_HARD_RESET);
                case CHORD_SOFT_RESET:             return res = app_message::type2str(APP_SOFT_RESET);
                case CHORD_FLUSH:                  return res = app_message::type2str(APP_FLUSH);
                                                   
                case CHORD_BROADCAST:              return res = app_message::type2str(APP_BROADCAST);
                case CHORD_MULTICAST:              return res = app_message::type2str(APP_MULTICAST);
                case CHORD_SINGLE:                 return res = app_message::type2str(APP_SINGLE);
                                                   
                case CHORD_CONF:                   return res = app_message::type2str(APP_CONF);

                case CHORD_RX_JOIN:                return res = "RX_JOIN";
                case CHORD_RX_NOTIFY:              return res = "RX_NOTIFY";
                case CHORD_RX_ACK:                 return res = "RX_ACK";
                case CHORD_RX_REPLY_FIND_SUCCESSOR: return res = "RX_REPLY_FIND_SUCC";
                case CHORD_RX_FIND_SUCCESSOR:      return res = "RX_FIND_SUCC";
                case CHORD_RX_BROADCAST:           return res = "RX_BROADCAST";
                case CHORD_RX_MULTICAST:           return res = "RX_MULTICAST";
                case CHORD_RX_SINGLE:              return res = "RX_SINGLE";

                case CHORD_TX_JOIN:                return res = "TX_JOIN";
                case CHORD_TX_NOTIFY:              return res = "TX_NOTIFY";
                case CHORD_TX_ACK:                 return res = "TX_ACK";
                case CHORD_TX_REPLY_FIND_SUCESSOR: return res = "TX_REPLY_FIND_SUCC";
                case CHORD_TX_FIND_SUCCESSOR:      return res = "TX_FIND_SUCC";
                case CHORD_TX_FWD_BROADCAST:       return res = "TX_FWD_BROADCAST";
                case CHORD_TX_FWD_MULTICAST:       return res = "TX_FWD_MULTICAST";
                case CHORD_TX_FWD_SINGLE:          return res = "TX_FWD_SINGLE";
                case CHORD_TX_BROADCAST:           return res = "TX_BROADCAST";
                case CHORD_TX_MULTICAST:           return res = "TX_MULTICAST";
                case CHORD_TX_SINGLE:              return res = "TX_SINGLE";     
                
                case CHORD_TIMER_ACK:              return res = "TIMER_ACK";
                case CHORD_TIMER_REPLY_FIND_SUCC:  return res = "TIMER_REPLY_FIND_SUCC";
                case CHORD_TIMER_REPLY_FIND_SUCC_JOIN:  return res = "TIMER_REPLY_FIND_SUCC_JOIN";
                case CHORD_TIMER_UPDATE:           return res = "TIMER_UPDATE";
                default:                           return res = "CHORD_UNKNOWN";
            }
        }

        string& toStr() {
            static string str;
            str.clear();

            switch (type) {
            case CHORD_HARD_RESET:
            case CHORD_SOFT_RESET:
            case CHORD_FLUSH: str = type2str(); break;

            case CHORD_BROADCAST:
            case CHORD_MULTICAST:
            case CHORD_SINGLE:

            case CHORD_RX_JOIN:
            case CHORD_RX_NOTIFY:
            case CHORD_RX_ACK:
            case CHORD_RX_REPLY_FIND_SUCCESSOR:
            case CHORD_RX_FIND_SUCCESSOR:
            case CHORD_RX_BROADCAST:
            case CHORD_RX_MULTICAST:
            case CHORD_RX_SINGLE:

            case CHORD_TX_JOIN:
            case CHORD_TX_NOTIFY:
            case CHORD_TX_ACK:
            case CHORD_TX_REPLY_FIND_SUCESSOR:
            case CHORD_TX_FIND_SUCCESSOR:
            case CHORD_TX_BROADCAST:
            case CHORD_TX_MULTICAST:
            case CHORD_TX_SINGLE:
                str = type2str();
                str += " " + to_string(payloadSize);

                if (destination.size() > 0) {
                    str += " dest: " + destination.at(0).toStr();
                
                    for (size_t i = 1; i < destination.size(); ++i) 
                        str += ", " + destination.at(i).toStr();
                }
                str += " " + destNodeID.to_string(SC_HEX_US);
                str += " " + string("src: ") + source.toStr();
                str += " " + string("med: ") + mediator.toStr();
                str += " appeared " + appearanceTime.to_string();
                str += " " + (retryMess == nullptr ? string("nullptr") : string("hasRetry"));
                break;

            case CHORD_TIMER_ACK:
            case CHORD_TIMER_REPLY_FIND_SUCC:
            case CHORD_TIMER_REPLY_FIND_SUCC_JOIN:
            case CHORD_TIMER_UPDATE:               str = type2str(); break;

            default: str = "CHORD_UNKOWN"; break;
            }
            return str;
        }

        friend ostream& operator<< (ostream& out, const chord_message& r);
    };


    struct raw_chord_message {
        chord_message info;
        data_unit rawByteArray;
    };
}
#endif