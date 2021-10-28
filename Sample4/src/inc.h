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
#include <cstdlib> // needs for rand() and srand()
#include <ctime>   // needs for time()

#include <systemc.h>
#include "trp/sha1.hpp"

using namespace std;

namespace P2P_MODEL {

    typedef sc_bigint<161>     int161;
    typedef sc_biguint<160>    uint160;
    typedef unsigned int       uint;
    typedef unsigned long long ulong;
    typedef uint               data_size_type;
    typedef unsigned char      byte;
    typedef vector<byte>       data_type;
    typedef vector<byte>       byte_array;
    typedef data_type          data_unit;

    const int NONE = -1;
    const int PID  = 0x01;    //PROTOCOL IDENTIFIER
    


    
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

    enum chord_byte_message_type {
        MIN_CHORD_BYTE_TYPE = 0,
        CHORD_BYTE_JOIN,
        CHORD_BYTE_NOTIFY,
        CHORD_BYTE_ACK,
        CHORD_BYTE_REPLY_FIND_SUCCESSOR,
        CHORD_BYTE_FIND_SUCCESSOR,
        CHORD_BYTE_BROADCAST,
        CHORD_BYTE_MULTICAST,
        CHORD_BYTE_SINGLE,
        MAX_CHORD_BYTE_TYPE,
        CHORD_BYTE_UNKNOWN
    };

    enum base_message_type {
        BASE_UNKNOWN_TYPE = MAX_CHORD_BYTE_TYPE,
        CHORD_UNKNOWN,
        MAX_BASE_TYPE
    };
    
    enum chord_conf_message_type {
        MIN_CHORD_CONF_TYPE = MAX_BASE_TYPE,
        CHORD_HARD_RESET,
        CHORD_SOFT_RESET,
        CHORD_FLUSH,
        CHORD_CONF,
        MAX_CHORD_CONF_TYPE,
        CHORD_CONF_UNKNOWN
    };

    enum chord_timer_message_type {
        MIN_CHORD_TIMER_TYPE = MAX_CHORD_CONF_TYPE,
        CHORD_TIMER_ACK,
        CHORD_TIMER_REPLY_FIND_SUCC,
        CHORD_TIMER_REPLY_FIND_SUCC_JOIN,
        CHORD_TIMER_UPDATE,
        MAX_CHORD_TIMER_TYPE,
        CHORD_TIMER_UNKNOWN
    };

    enum chord_apptx_message_type {
        MIN_CHORD_APPTX_TYPE = MAX_CHORD_TIMER_TYPE,
        CHORD_SINGLE,
        CHORD_MULTICAST,
        CHORD_BROADCAST,
        MAX_CHORD_APPTX_TYPE,
        CHORD_APPTX_UNKNOWN
    };

    enum chord_rx_message_type {
        MIN_CHORD_RX_TYPE = MAX_CHORD_APPTX_TYPE,
        CHORD_RX_JOIN,
        CHORD_RX_NOTIFY,
        CHORD_RX_ACK,
        CHORD_RX_REPLY_FIND_SUCCESSOR,
        CHORD_RX_FIND_SUCCESSOR,
        CHORD_RX_BROADCAST,
        CHORD_RX_MULTICAST,
        CHORD_RX_SINGLE,
        MAX_CHORD_RX_TYPE,
        CHORD_RX_UNKNOWN
    };

    enum chord_tx_message_type {
        MIN_CHORD_TX_TYPE = MAX_CHORD_RX_TYPE,
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
        MAX_CHORD_TX_TYPE,
        CHORD_TX_UNKNOWN
    };





         
    class network_address {
    private:
        const string m_ipNULL = "-1.-1.-1.-1";

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
            ip = m_ipNULL;
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

        string toStr() const {
            string str;            
            str = "" + ip + " " + to_string(inSocket) + " " + to_string(outSocket);
            return str;
        }

        bool isNone() {
            if (ip.compare(m_ipNULL) == 0)   //equaled "-1.-1.-1.-1"
                return true;
            return false;
        }

        friend ostream& operator<< (ostream& out,       network_address& src);
        friend bool     operator== (const network_address& l, const network_address& r);
    };


    class node_address : virtual public network_address {
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

        string toStr() const {
            string str;
            str = network_address::toStr() + " " + id.to_string(SC_HEX_US);
            return str;
        }

        friend ostream& operator<< (ostream& out,          node_address& r);
        friend bool     operator== (const node_address& l, const node_address& r);


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


        string type2str(const int& type = NONE) const {
            string res;
            int t = type;
            if (type == NONE)
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

        string toStr() const {
            string str;
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


    class base_message {
    public:        
        uint type;

        base_message() {
            clear();
        }

        virtual void clear() {
            type = BASE_UNKNOWN_TYPE;
        }

        virtual base_message& operator= (const base_message& src) {
            if (this == &src)
                return *this;
            type = src.type;
            return *this;
        }
    };


    class app_message: public virtual base_message {
    public:
        vector<network_address> destination;        
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
            
            base_message::operator=(src);            
            destination = src.destination;           
            payload = src.payload;
            payloadSize = src.payloadSize;
            return *this;
        }

        void clear() {
            base_message::clear();
            type = APP_UNKNOWN;
            destination.clear();            
            payload.clear();
            payloadSize = 0;
        }

        string type2str(const int& type = NONE) const {
            string res;
            int t = type;
            if (type == NONE)
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

        string toStr() const {
            string str;
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


    class chord_conf_message : public virtual base_message {
    public:

        chord_conf_message() {
            clear();
        }

        chord_conf_message(const chord_conf_message& src) {
            *this = src;
        }

        chord_conf_message& operator= (const chord_conf_message& src) {
            if (this == &src)
                return *this;
            base_message::operator=(src);
            //type = src.type;            
            return *this;
        }

        void clear() {
            type = CHORD_CONF_UNKNOWN;
        }

        string type2str(const int& type = NONE) const {
            string str;
            int t = type;
            if (type == NONE)
                t = this->type;

            
            switch (t) {
            case CHORD_HARD_RESET: return str = app_message().type2str(APP_HARD_RESET);
            case CHORD_SOFT_RESET: return str = app_message().type2str(APP_SOFT_RESET);
            case CHORD_FLUSH:      return str = app_message().type2str(APP_FLUSH);
            case CHORD_CONF:       return str = app_message().type2str(APP_CONF);
            default:               return str = "CHORD_UNKNOWN";
            }
        }

        string toStr() const {
            string str;
            switch (type) {
            case CHORD_HARD_RESET: return str = "HARD_RESET";
            case CHORD_SOFT_RESET: return str = "SOFT_RESET";
            case CHORD_FLUSH:      return str = "FLUSH";
            case CHORD_CONF:       return str = "CONF";
            default:               return str = "CHORD_UNKOWN";
            }
        }

        friend ostream& operator<< (ostream& out, const chord_conf_message& r);
    };

    
    class chord_apptx_message: public virtual app_message {
    public:        
        //uint160  destNodeID;
        sc_time  appearanceTime;               

        chord_apptx_message() {
            clear();
        }

        chord_apptx_message(const chord_apptx_message& src) {
            *this = src;
        }

        chord_apptx_message& operator= (const chord_apptx_message& src) {
            if (this == &src)
                return *this;

            app_message::operator=(src);            
            appearanceTime = src.appearanceTime;
            return *this;
        }

        void clear() {
            app_message::clear();
            type           = CHORD_APPTX_UNKNOWN;
            appearanceTime = SC_ZERO_TIME;               
        }

        string type2str(const int& type = NONE) const {
            string res;
            int t = type;
            if (type == NONE)
                t = this->type;

            switch (t) {
                case CHORD_BROADCAST: return res = app_message::type2str(APP_BROADCAST);
                case CHORD_MULTICAST: return res = app_message::type2str(APP_MULTICAST);
                case CHORD_SINGLE:    return res = app_message::type2str(APP_SINGLE);
                default:              return res = "CHORD_UNKNOWN";
            }
        }

        string toStr() const {
            string str;
            str.clear();

            switch (type) {
            case CHORD_BROADCAST:
            case CHORD_MULTICAST:
            case CHORD_SINGLE:           
                str = type2str();
                str += " " + to_string(payloadSize);

                if (destination.size() > 0) {
                    str += " dest " + destination.at(0).toStr();
                
                    for (size_t i = 1; i < destination.size(); ++i) 
                        str += ", " + destination.at(i).toStr();
                }
                //str += " " + destNodeID.to_string(SC_HEX_US);
                //str += " " + string("src ") + source.toStr();
                //str += " " + string("init ") + initiator.toStr();
                str += " when " + appearanceTime.to_string();
                break;

            default: str = "CHORD_UNKOWN"; break;                
            }
            return str;
        }

        friend ostream& operator<< (ostream& out, const chord_apptx_message& r);
    };


    struct chord_bits_flags {
        uint bitMessType : 4 ;
        uint needsACK : 1;
        uint payloadSize : 32;
        uint reserve : 3;

        chord_bits_flags() {
            clear();
        }

        void clear() {
            bitMessType = CHORD_BYTE_UNKNOWN;
            needsACK = 0;
            payloadSize = 0;
            reserve = 0;
        }

        string type2str(const int& type = NONE) const {
            string str;
            int t = type;
            if (type == NONE)
                t = bitMessType;

            switch (t) {
            case CHORD_BYTE_JOIN:                 return str = "JOIN";
            case CHORD_BYTE_NOTIFY:               return str = "NOTIFY";
            case CHORD_BYTE_ACK:                  return str = "ACK";
            case CHORD_BYTE_REPLY_FIND_SUCCESSOR: return str = "REPLY_FIND_SUCC";
            case CHORD_BYTE_FIND_SUCCESSOR:       return str = "FIND_SUCC";
            case CHORD_BYTE_BROADCAST:            return str = "BROADCAST";
            case CHORD_BYTE_MULTICAST:            return str = "MULTICAST";
            case CHORD_BYTE_SINGLE:               return str = "SINGLE";
            default:                              return str = "CHORD_BYTE_MESSAGE_UNKNOWN";
            }
        }

        string toStr() const {
            string str;
            str += string("t ") + type2str() + string(" a ") + to_string(needsACK) + string(" ps ") + to_string(payloadSize) + string(" r ") + to_string(reserve);
            return str;
        }

        chord_bits_flags& operator= (const chord_bits_flags& src) {
            if (this == &src)
                return *this;

            bitMessType = src.bitMessType;
            needsACK = src.needsACK;
            payloadSize = src.payloadSize;
            reserve = src.reserve;
            return *this;
        }

        friend ostream& operator<< (ostream& out, const chord_bits_flags& r);
    };


    class chord_byte_message_fields : public virtual base_message {
    public:
        node_address destNodeID;
        node_address srcNodeID;
        byte         pid;
        chord_bits_flags   flags;
        node_address initiatorID;
        uint160      nodeIDpayload;
        byte_array   payload;

        chord_byte_message_fields() {
            clear();
        }

        chord_byte_message_fields(const chord_byte_message_fields& src) {
            *this = src;
        }

        chord_byte_message_fields* clone() {
            chord_byte_message_fields* p = nullptr;
            p = new chord_byte_message_fields();
            *p = *this;
            return p;
        }


        chord_byte_message_fields& operator= (const chord_byte_message_fields& src) {
            if (this == &src)
                return *this;

            base_message::operator=(src);
            //type = src.type;
            destNodeID = src.destNodeID;
            srcNodeID = src.srcNodeID;
            pid = src.pid;
            initiatorID = src.initiatorID;
            flags = src.flags;
            nodeIDpayload = src.nodeIDpayload;
            payload = src.payload;
            return *this;
        }

        void clear() {
            type = CHORD_BYTE_UNKNOWN;
            destNodeID.clear();       //        = 0;
            srcNodeID.clear();        //        = 0;
            pid = PID;
            initiatorID.clear();      //        = 0;
            flags.bitMessType = CHORD_BYTE_UNKNOWN;
            flags.needsACK = 0;
            flags.payloadSize = 0;
            flags.reserve = 0;
            nodeIDpayload = 0;
            payload.clear();
        }

        string type2str(const int& type = NONE) const {
            string str;
            uint t = type;
            if (type == NONE)
                t = this->type;
            
            switch (t) {
                case CHORD_RX_JOIN:                return str = "RX_JOIN";
                case CHORD_RX_NOTIFY:              return str = "RX_NOTIFY";
                case CHORD_RX_ACK:                 return str = "RX_ACK";
                case CHORD_RX_FIND_SUCCESSOR:      return str = "RX_FIND_SUCCESSOR";
                case CHORD_RX_BROADCAST:           return str = "RX_BROADCAST";
                case CHORD_RX_MULTICAST:           return str = "RX_MULTICAST";
                case CHORD_RX_SINGLE:              return str = "RX_SINGLE";

                case CHORD_TX_JOIN:                return str = "TX_JOIN";
                case CHORD_TX_NOTIFY:              return str = "TX_NOTIFY";
                case CHORD_TX_ACK:                 return str = "TX_ACK";
                case CHORD_TX_REPLY_FIND_SUCESSOR: return str = "TX_REPLY_FIND_SUCESSOR";
                case CHORD_TX_FIND_SUCCESSOR:      return str = "TX_FIND_SUCCESSOR";
                case CHORD_TX_FWD_BROADCAST:       return str = "TX_FWD_BROADCAST";
                case CHORD_TX_FWD_MULTICAST:       return str = "TX_FWD_MULTICAST";
                case CHORD_TX_FWD_SINGLE:          return str = "TX_FWD_SINGLE";
                case CHORD_TX_BROADCAST:           return str = "TX_BROADCAST";
                case CHORD_TX_MULTICAST:           return str = "TX_MULTICAST";
                case CHORD_TX_SINGLE:              return str = "TX_SINGLE";
                default:                           return str = "CHORD_UNKNOWN";
            }
        }


        string toStr() const {
            string str;
            
            str += type2str() + string(" ");
            str += string("d ") + destNodeID.toStr() + string(" ");
            str += string("s ") + srcNodeID.toStr() + string(" ");
            str += string("pid ") + to_string(pid) + string(" ");
            str += string("i ") + initiatorID.toStr() + string(" ");
            str += string("f ") + flags.toStr() + string(" ");
            str += string("npid ") + nodeIDpayload.to_string(SC_HEX_US) + string(" ");
            str += string("p ") + to_string(payload.size());
            return str;
        }

        friend ostream& operator<< (ostream& out, const chord_byte_message_fields& r);
    };


    class chord_timer_message: public virtual chord_apptx_message {
    public:
       
       chord_byte_message_fields* retryMess;


        chord_timer_message(): retryMess(nullptr) {
            clear();
        }

        chord_timer_message(const chord_timer_message& src): retryMess(nullptr) {
            *this = src;
        }

        ~chord_timer_message() {
            clear();
        }

        chord_timer_message& operator= (const chord_timer_message& src) {
            if (this == &src)
                return *this;

            chord_apptx_message::operator=(src);

            if (retryMess != nullptr) {
                delete retryMess;
                retryMess = nullptr;
            }
            if (src.retryMess != nullptr) {
                retryMess = new chord_byte_message_fields();
                *retryMess = *(src.retryMess);
            }
            return *this;
        }

        void clear() {           
            chord_apptx_message::clear();
            type = CHORD_TIMER_UNKNOWN;
            if (retryMess != nullptr) {
                delete retryMess;
                retryMess = nullptr;
            }
        }

        string type2str(const int& type = NONE) const {
            string str;
            uint t = type;
            if (type == NONE)
                t = this->type;

            switch (t) {
            case CHORD_TIMER_ACK:                  return str = "TIMER_ACK";
            case CHORD_TIMER_REPLY_FIND_SUCC:      return str = "TIMER_REPLY_FIND_SUCC";
            case CHORD_TIMER_REPLY_FIND_SUCC_JOIN: return str = "TIMER_REPLY_FIND_SUCC_JOIN";
            case CHORD_TIMER_UPDATE:               return str = "TIMER_UPDATE";
            default:                               return str = "CHORD_UNKNOWN";
            }
        }

        string toStr() const {
            string str;
            switch (type) {
            case CHORD_TIMER_ACK:
            case CHORD_TIMER_REPLY_FIND_SUCC:
            case CHORD_TIMER_REPLY_FIND_SUCC_JOIN:
            case CHORD_TIMER_UPDATE:               return str = type2str(); 
            default:                               return str = "CHORD_UNKOWN"; 
            }
        }

        friend ostream& operator<< (ostream& out, const chord_timer_message& r);
    };
    

    class chord_message:
        public virtual chord_conf_message,
        public virtual chord_timer_message,
        public virtual chord_apptx_message,       
        public virtual chord_byte_message_fields {
    public:    
        chord_message() {
            clear();
        }

        chord_message(const app_message& r) {
            *this = r;
        }

        chord_message(const chord_conf_message& r) {
            *this = r;
        }

        chord_message(const chord_timer_message& r) {
            *this = r;
        }

        chord_message(const chord_apptx_message& r) {
            *this = r;
        }

        chord_message(const chord_byte_message_fields& r) {
            *this = r;
        }

        chord_message(const chord_message& r) {
            *this = r;
        }

        void clear() {           
            chord_conf_message::clear();
            chord_timer_message::clear();
            chord_apptx_message::clear();           
            chord_byte_message_fields::clear();
        }
        
        ~chord_message() {
            clear();
        }

        chord_message& operator= (const app_message& src) {
            if (this == &src)
                return *this;

            app_message::operator=(src);

            return *this;
        }

        chord_message& operator= (const chord_conf_message& src) {
            if (this == &src)
                return *this;

            chord_conf_message::operator=(src);

            return *this;
        }

        chord_message& operator= (const chord_timer_message& src) {
            if (this == &src)
                return *this;

            chord_timer_message::operator=(src);

            return *this;
        }

        chord_message& operator= (const chord_apptx_message& src) {
            if (this == &src)
                return *this;

            chord_apptx_message::operator=(src);

            return *this;
        }

        chord_message& operator= (const chord_byte_message_fields& src) {
            if (this == &src)
                return *this;

            chord_byte_message_fields::operator=(src);

            return *this;
        }

        chord_message& operator= (const chord_message& src) {
            if (this == &src)
                return *this;
            
            app_message::operator=(src);
            chord_conf_message::operator=(src);
            chord_timer_message::operator=(src);
            chord_apptx_message::operator=(src);           
            chord_byte_message_fields::operator=(src);            
            
            return *this;
        }

        string toStr() const {
            if ((type > MIN_CHORD_CONF_TYPE) && (type < MAX_CHORD_CONF_TYPE)) {
                return chord_conf_message::toStr();
            }
            else if ((type > MIN_CHORD_TIMER_TYPE) && (type < MAX_CHORD_TIMER_TYPE)) {
                return chord_timer_message::toStr();
            }
            else if ((type > MIN_CHORD_APPTX_TYPE) && (type < MAX_CHORD_APPTX_TYPE)) {
                return chord_apptx_message::toStr();
            }
            else if ((type > MIN_CHORD_RX_TYPE) && (type < MAX_CHORD_RX_TYPE)) {
                return chord_byte_message_fields::toStr();
            }
            else if ((type > MIN_CHORD_TX_TYPE) && (type < MAX_CHORD_TX_TYPE)) {
                return chord_byte_message_fields::toStr();
            }
            else {                
                return string("BASE_UNKNOWN_TYPE");
            }

        }
    };
    

    class chord_byte_message {
    public:
        byte_array byteArray;
        uint size;
        chord_byte_message_fields* fields;

        chord_byte_message(): fields(nullptr) {
            clear();
        }

        chord_byte_message(const chord_byte_message& src): fields(nullptr) {
            *this = src;
        }

        chord_byte_message& operator= (const chord_byte_message& src) {
            if (this == &src)
                return *this;
            
            byteArray = src.byteArray;
            size      = src.size;
            //fields    = src.fields;

            if (fields != nullptr) {
                delete fields;
                fields = nullptr;
            }
            if (src.fields != nullptr) {
                fields = new chord_byte_message_fields;
                *fields = *(src.fields);
            }

            return *this;
        }

        ~chord_byte_message() { 
            clear(); 
        }

        void clear() {
            byteArray.clear();
            size = 0;            
            if (fields != nullptr) {                
                delete fields;
                fields = nullptr;
            }
        }

        string toStr() {
            string str;
            str += string("array ") + to_string(byteArray.size()) + string("|") + to_string(size) + string(" ") + (fields == nullptr ? string("") : fields->toStr());
            return str;
        }

        friend ostream& operator<< (ostream& out, const chord_byte_message& r);
    };


    void initRands();
}
#endif