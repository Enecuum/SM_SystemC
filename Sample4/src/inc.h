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
#include <xstring>
#include <cstdlib> // needs for rand() and srand()
#include <ctime>   // needs for time() for rand
#include <math.h> 

#include <systemc.h>
#include "trp/sha1.hpp"
#include "nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace P2P_MODEL {    
    typedef sc_biguint<161>    uint161;
    typedef sc_biguint<11>     uint160;    
    typedef unsigned int       uint;
    typedef unsigned long long ulong;
    typedef uint               data_size_type;
    typedef unsigned char      byte;
    typedef vector<byte>       data_type;
    typedef vector<byte>       byte_array;
    typedef data_type          data_unit;

    extern uint MAX_SMALL_UINT;
    extern sc_time MONITOR_PERIOD_CHECK_FINGERS;

    const uint DENIED = 0;
    const int NONE = -1;
    const uint PID  = 0x01;    //PROTOCOL IDENTIFIER
    const uint IP_4_VERSION = 0; 
    const uint IP_6_VERSION = 1;
    const uint NO_ACK = 0;
    const uint NEEDS_ACK = 1;   
    const uint MAX_UINT = 0xffffffff;
    const sc_time DEFAULT_LATENCY = sc_time(10, SC_SEC);
    

    //String constanst for `motive` field of `node_address_latency` class
    const string MOTIVE_DEFAULT = "default";
    const string MOTIVE_PERIOD_RET_SUCC = "period";
    const string MOTIVE_FORCE_UPDATE = "forceupd";
    const string MOTIVE_STABILIZE = "stab";
    const string MOTIVE_NOTIFY = "notify";
    const string MOTIVE_COPY = "copy";
    const string MOTIVE_JOIN = "join";
    const string MOTIVE_PRED_SUCC = "succ<node"; //"succ less node"
    
    uint genRand(uint Min, uint Max);
    string decToHex(const uint dec);

    
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
        APP_CONTINUE,
        APP_PAUSE,
        APP_CONF,
        MAX_APP_MESS_TYPE,
        APP_UNKNOWN
    };


    enum chord_byte_message_type {
        MIN_CHORD_BYTE_TYPE = 0,
        CHORD_BYTE_JOIN,
        CHORD_BYTE_NOTIFY,
        CHORD_BYTE_ACK,
        CHORD_BYTE_SUCCESSOR,
        CHORD_BYTE_FIND_SUCCESSOR,
        CHORD_BYTE_PREDECESSOR,
        CHORD_BYTE_FIND_PREDECESSOR,
        CHORD_BYTE_BROADCAST,
        CHORD_BYTE_MULTICAST,
        CHORD_BYTE_SINGLE,
        MAX_CHORD_BYTE_TYPE,
        CHORD_BYTE_UNKNOWN
    };

    enum base_message_type {
        MIN_BASE_TYPE = MAX_CHORD_BYTE_TYPE,
        BASE_UNKNOWN_TYPE,
        CHORD_UNKNOWN,
        MAX_BASE_TYPE
    };
    
    enum chord_conf_message_type {
        MIN_CHORD_CONF_TYPE = MAX_BASE_TYPE,
        CHORD_HARD_RESET,
        CHORD_SOFT_RESET,
        CHORD_FLUSH,
        CHORD_CONTINUE,
        CHORD_PAUSE,
        CHORD_CONF,
        MAX_CHORD_CONF_TYPE,
        CHORD_CONF_UNKNOWN
    };

    enum chord_timer_message_type {
        MIN_CHORD_TIMER_TYPE = MAX_CHORD_CONF_TYPE,
        CHORD_TIMER_RX_ACK,
        CHORD_TIMER_RX_SUCCESSOR,
        CHORD_TIMER_RX_SUCCESSOR_ON_JOIN,
        CHORD_TIMER_RX_PREDECESSOR,
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
        CHORD_RX_SUCCESSOR,
        CHORD_RX_FIND_SUCCESSOR,
        CHORD_RX_PREDECESSOR,
        CHORD_RX_FIND_PREDECESSOR,
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
        CHORD_TX_SUCCESSOR,
        CHORD_TX_FIND_SUCCESSOR,
        //CHORD_TX_FWD_FIND_SUCCESSOR,
        CHORD_TX_PREDECESSOR,
        CHORD_TX_FIND_PREDECESSOR,
        CHORD_TX_FWD_BROADCAST,
        CHORD_TX_FWD_MULTICAST,
        CHORD_TX_FWD_SINGLE,
        CHORD_TX_BROADCAST,
        CHORD_TX_MULTICAST,
        CHORD_TX_SINGLE,
        MAX_CHORD_TX_TYPE,
        CHORD_TX_UNKNOWN
    };


    enum finite_state {
        STATE_OFF = 0,
        STATE_LOAD,
        STATE_INIT,
        STATE_JOIN,
        STATE_IDLE,
        STATE_INDATA,
        STATE_SERVICE,
        STATE_UPDATE,
        STATE_APPREQUEST,        
        MAX_FINITE_STATE,        
        STATE_UNKNOWN
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

        network_address(const string& ip, const uint inSocket, const uint outSocket = 0) {
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
            stringstream ss;
            ss << ip << " " << dec /*hex*/ << inSocket << " " << outSocket;
            return ss.str();
        }

        bool isNone() const {
            if (ip.compare(m_ipNULL) == 0)   //equaled "-1.-1.-1.-1"
                return true;
            return false;
        }

        friend ostream& operator<< (ostream& out,       const network_address& src);
        friend bool     operator== (const network_address& l, const network_address& r);
    };


    class node_address : virtual public network_address {
        //private:
        //    network_address m_networkAddr;
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

            network_address::set(src.ip, src.inSocket, src.outSocket);
            this->id = src.id;
            return *this;
        }

        string toStr(const bool isDec = true) const {
            string str;
            //str = network_address::toStr() + " ";
            if (isDec == true) {
                string hex = id.to_string(SC_DEC);
                str += "@" + hex + (isNone() ? string(" None") : string(""));
            }
            else {
                string hex = id.to_string(SC_HEX_US);
                str += "@" + hex.erase(0, 4) + (isNone() ? string(" None") : string(""));
            }
            return str;
        }

        string toStrIDonly(const bool isDec = true) const {
            string str;

            if (isDec == true) {
                string hex = id.to_string(SC_DEC);
                str += "@" + hex + (isNone() ? string(" None") : string(""));
            }
            else {
                string hex = id.to_string(SC_HEX_US);
                str += "@" + hex.erase(0, 4) + (isNone() ? string(" None") : string(""));
            }
            return str;
        }




        network_address toNetworAddress() const {
            network_address a;
            a.network_address::set(this->ip, this->inSocket, this->outSocket);
            return a;
        }

        friend ostream& operator<< (ostream& out,          const node_address& r);
        friend bool     operator== (const node_address& l, const node_address& r);

    private:    
        uint160 sha1(const string& str, const bool isDec = true) {
            static map<string, uint160> ip2id;
            static uint160 uniqueID = 0;
            uint160 res;

            auto it = ip2id.find(str);
            if (it == ip2id.end()) {
                if (isDec == true) {
                    res = uniqueID;
                    uniqueID++;
                }
                else {
                    SHA1 checksum;
                    checksum.update(str);
                    string strID = checksum.final();
                    strID.insert(0, "0xus");
                    res = strID.c_str();                    
                }
                ip2id[str] = res;                
            }
            else {
                res = it->second;
            }
                                               
            return res;
        }


        unsigned char crc8(const vector<unsigned char>& data, const uint from = 0)
        {
            static unsigned char crcTable[] = {0x00, 0x91, 0xe3, 0x72, 0x07, 0x96, 0xe4, 0x75
                                                , 0x0e, 0x9f, 0xed, 0x7c, 0x09, 0x98, 0xea, 0x7b
                                                , 0x1c, 0x8d, 0xff, 0x6e, 0x1b, 0x8a, 0xf8, 0x69
                                                , 0x12, 0x83, 0xf1, 0x60, 0x15, 0x84, 0xf6, 0x67
                                                , 0x38, 0xa9, 0xdb, 0x4a, 0x3f, 0xae, 0xdc, 0x4d
                                                , 0x36, 0xa7, 0xd5, 0x44, 0x31, 0xa0, 0xd2, 0x43
                                                , 0x24, 0xb5, 0xc7, 0x56, 0x23, 0xb2, 0xc0, 0x51
                                                , 0x2a, 0xbb, 0xc9, 0x58, 0x2d, 0xbc, 0xce, 0x5f
                                                , 0x70, 0xe1, 0x93, 0x02, 0x77, 0xe6, 0x94, 0x05
                                                , 0x7e, 0xef, 0x9d, 0x0c, 0x79, 0xe8, 0x9a, 0x0b
                                                , 0x6c, 0xfd, 0x8f, 0x1e, 0x6b, 0xfa, 0x88, 0x19
                                                , 0x62, 0xf3, 0x81, 0x10, 0x65, 0xf4, 0x86, 0x17
                                                , 0x48, 0xd9, 0xab, 0x3a, 0x4f, 0xde, 0xac, 0x3d
                                                , 0x46, 0xd7, 0xa5, 0x34, 0x41, 0xd0, 0xa2, 0x33
                                                , 0x54, 0xc5, 0xb7, 0x26, 0x53, 0xc2, 0xb0, 0x21
                                                , 0x5a, 0xcb, 0xb9, 0x28, 0x5d, 0xcc, 0xbe, 0x2f
                                                , 0xe0, 0x71, 0x03, 0x92, 0xe7, 0x76, 0x04, 0x95
                                                , 0xee, 0x7f, 0x0d, 0x9c, 0xe9, 0x78, 0x0a, 0x9b
                                                , 0xfc, 0x6d, 0x1f, 0x8e, 0xfb, 0x6a, 0x18, 0x89
                                                , 0xf2, 0x63, 0x11, 0x80, 0xf5, 0x64, 0x16, 0x87
                                                , 0xd8, 0x49, 0x3b, 0xaa, 0xdf, 0x4e, 0x3c, 0xad
                                                , 0xd6, 0x47, 0x35, 0xa4, 0xd1, 0x40, 0x32, 0xa3
                                                , 0xc4, 0x55, 0x27, 0xb6, 0xc3, 0x52, 0x20, 0xb1
                                                , 0xca, 0x5b, 0x29, 0xb8, 0xcd, 0x5c, 0x2e, 0xbf
                                                , 0x90, 0x01, 0x73, 0xe2, 0x97, 0x06, 0x74, 0xe5
                                                , 0x9e, 0x0f, 0x7d, 0xec, 0x99, 0x08, 0x7a, 0xeb
                                                , 0x8c, 0x1d, 0x6f, 0xfe, 0x8b, 0x1a, 0x68, 0xf9
                                                , 0x82, 0x13, 0x61, 0xf0, 0x85, 0x14, 0x66, 0xf7
                                                , 0xa8, 0x39, 0x4b, 0xda, 0xaf, 0x3e, 0x4c, 0xdd
                                                , 0xa6, 0x37, 0x45, 0xd4, 0xa1, 0x30, 0x42, 0xd3
                                                , 0xb4, 0x25, 0x57, 0xc6, 0xb3, 0x22, 0x50, 0xc1
                                                , 0xba, 0x2b, 0x59, 0xc8, 0xbd, 0x2c, 0x5e, 0xcf
            };

            unsigned char crc = 0;
            unsigned char crcTableIndex;
            uint lenght;
            for (lenght = from; lenght < data.size(); lenght++) {
                crcTableIndex = crc ^ static_cast<unsigned char>(data.at(lenght));
                crc = crcTable[crcTableIndex];
            }
            return crc;
        }


        unsigned short crc16(const vector<unsigned char>& data, const uint from = 0)
        {
            static unsigned short crcTable[] = {0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7
                                                , 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF
                                                , 0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6
                                                , 0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE
                                                , 0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485
                                                , 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D
                                                , 0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4
                                                , 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC
                                                , 0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823
                                                , 0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B
                                                , 0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12
                                                , 0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A
                                                , 0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41
                                                , 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49
                                                , 0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70
                                                , 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78
                                                , 0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F
                                                , 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067
                                                , 0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E
                                                , 0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256
                                                , 0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D
                                                , 0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405
                                                , 0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C
                                                , 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634
                                                , 0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB
                                                , 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3
                                                , 0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A
                                                , 0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92
                                                , 0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9
                                                , 0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1
                                                , 0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8
                                                , 0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
            };

            unsigned short crc = 0x1D0F;
            uint lenght;
            for (lenght = from; lenght < data.size(); lenght++) {
                crc = (crc << 8) ^ crcTable[(crc >> 8) ^ (0x00ff & (data.at(lenght)))];
            }
            return crc;
        }

    };

    
    class node_address_latency: virtual public node_address {
    public:
        sc_time latency;
        uint fingerIndex;
        bool isUpdated;
        sc_time updateTime;
        bool isClockWise; 
        string motive;

        node_address_latency() {
            clear();
        };

        node_address_latency(const node_address_latency& src) {
            *this = src;
        }

        node_address_latency(const node_address& src): isUpdated(false), isClockWise(true){
            motive = MOTIVE_DEFAULT;
            *this = src;
        }

        node_address_latency(const network_address& src): isUpdated(false), isClockWise(true) {                       
            motive = MOTIVE_DEFAULT;
            *this = src;
        }

        void clear() {
            node_address::clear();
            latency = DEFAULT_LATENCY;
            fingerIndex = 0;
            isClockWise = true;
            isUpdated = false;
            updateTime = SC_ZERO_TIME;
            motive = MOTIVE_DEFAULT;
        }

        void setCopy(const node_address_latency& src) {
            clear();
            network_address::set(src.ip, src.inSocket, src.outSocket);
            id = src.id;
            latency = src.latency;
            fingerIndex = src.fingerIndex;
            isClockWise = src.isClockWise;
            isUpdated = src.isUpdated;
            updateTime = src.updateTime;
            motive = src.motive;
        }

        node_address_latency& operator= (const node_address_latency& src) {
            if (this == &src)
                return *this;

            network_address::set(src.ip, src.inSocket, src.outSocket);
            this->id = src.id;
            this->latency = src.latency;
            this->isUpdated = src.isUpdated;
            this->updateTime = src.updateTime;            
            this->motive = src.motive;
            return *this;
        }

        node_address_latency& operator= (const node_address& src) {
            if (this == &src)
                return *this;

            network_address::set(src.ip, src.inSocket, src.outSocket);
            this->id = src.id;
            return *this;
        }

        node_address_latency& operator= (const network_address& src) {
            if (this == &src)
                return *this;

            node_address::set(src.ip, src.inSocket, src.outSocket);            
            return *this;
        }

        node_address toNodeAddress() const {
            node_address a;
            a.node_address::set(this->ip, this->inSocket, this->outSocket);
            return a;
        }

        string toStr() const {            
            string str = node_address::toStrIDonly() + /*string(", latency ") + latency.to_string() +*/ string(", index ") + to_string(fingerIndex) + (isClockWise ? string(" cw") : string(" ccw")) + (isUpdated ? string(" upd time ") : string(" old time ")) + updateTime.to_string();
            return str;
        }

        string toStrIDmotive(bool isDec = true) const {
            string str;

            if (isDec == true) {
                string hex = id.to_string(SC_DEC);
                str += /*"@" +*/ hex + (isNone() ? string(" None ") : string(" ")) + motive + string("   ") + this->updateTime.to_string();
            }
            else {
                string hex = id.to_string(SC_HEX_US);
                str += /*"@" +*/ hex.erase(0, 4) + (isNone() ? string(" None ") : string(" ")) + motive + string("   ") + this->updateTime.to_string();
            }
            return str;
            
        }

        string toStrFinger(const bool allParams = false) const {
            stringstream str;
            if (this->isClockWise) {
                str << "cwfinger" << to_string(fingerIndex) << " " << node_address::toStrIDonly();
                if (allParams == true)
                    str << (isUpdated ? string(", upd ") : string(", old ")) << updateTime.to_string() << (isNone() ? string(" None") : string(""))/* << ", latency " << latency.to_string()*/;
            }
            else {
                str << "ccwfinger" << to_string(fingerIndex) << " " << node_address::toStrIDonly();
                if (allParams == true)
                    str << (isUpdated ? string(", upd ") : string(", old ")) << updateTime.to_string() << (isNone() ? string(" None") : string(""))/* << ", latency " << latency.to_string()*/;
            }
            return str.str();
        }

        friend ostream& operator<< (ostream& out, node_address_latency& r);
        friend bool     operator== (const node_address_latency& l, const node_address_latency& r);
    };


    class sim_message {
    public:
        vector<network_address> destNetwAddrs;
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

            destNetwAddrs = src.destNetwAddrs;
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
            destNetwAddrs.clear();
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

                if (destNetwAddrs.size() > 0) {
                    str += " dest: ";
                    str += destNetwAddrs.at(0).toStr();
                }
                for (size_t i = 1; i < destNetwAddrs.size(); ++i) {
                    str += ", ";
                    str += destNetwAddrs.at(i).toStr();
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
        vector<network_address> destNetwAddrs;        
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
            destNetwAddrs = src.destNetwAddrs;           
            payload = src.payload;
            payloadSize = src.payloadSize;
            return *this;
        }

        void clear() {
            base_message::clear();
            type = APP_UNKNOWN;
            destNetwAddrs.clear();            
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
                case APP_PAUSE:      return res = "PAUSE";
                case APP_CONTINUE:   return res = "CONTINUE";
                case APP_CONF:       return res = "CONF";
                default:             return res = "APP_UNKNOWN";
            }
        }

        string toStr() const {
            string str;
            str = type2str();
            if ((type == APP_SINGLE) || (type == APP_MULTICAST) || (type == APP_BROADCAST))
                str += " " + to_string(payloadSize/*payload.size()*/);           

            if (destNetwAddrs.size() > 0) {
                str += " dest: " + destNetwAddrs.at(0).toStr();                
                for (size_t i = 1; i < destNetwAddrs.size(); ++i) 
                    str += ", " + destNetwAddrs.at(i).toStr();
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
            case CHORD_PAUSE:      return str = app_message().type2str(APP_PAUSE);
            case CHORD_CONTINUE:   return str = app_message().type2str(APP_CONTINUE);
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
            case CHORD_PAUSE:      return str = "PAUSE";
            case CHORD_CONTINUE:   return str = "CONTINUE";
            case CHORD_CONF:       return str = "CONF";
            default:               return str = "CHORD_UNKOWN";
            }
        }

        friend ostream& operator<< (ostream& out, const chord_conf_message& r);
    };

    
    class chord_apptx_message: public virtual app_message {
    public:        
        sc_time  creatingTime;               

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
            creatingTime = src.creatingTime;
            return *this;
        }

        void clear() {
            app_message::clear();
            type           = CHORD_APPTX_UNKNOWN;
            creatingTime = SC_ZERO_TIME;               
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

                if (destNetwAddrs.size() > 0) {
                    str += " dest " + destNetwAddrs.at(0).toStr();
                
                    for (size_t i = 1; i < destNetwAddrs.size(); ++i) 
                        str += ", " + destNetwAddrs.at(i).toStr();
                }
                //str += " " + destNodeIDwithSocket.to_string(SC_HEX_US);
                //str += " " + string("src ") + source.toStr();
                //str += " " + string("init ") + initiator.toStr();
                str += " when " + creatingTime.to_string();
                break;

            default: str = "CHORD_UNKOWN"; break;                
            }
            return str;
        }

        friend ostream& operator<< (ostream& out, const chord_apptx_message& r);
    };


    struct chord_bits_flags {
        uint bitMessType : 4;
        uint needsACK  : 1;       
        uint destIPversion: 1;
        uint srcIPversion : 1;
        uint initiatorIPversion : 1;
        uint payloadSize : 32;
        uint reserve : 16;

        chord_bits_flags() {
            clear();
        }

        void clear() {
            bitMessType = CHORD_BYTE_UNKNOWN;
            needsACK = NO_ACK;
            destIPversion = IP_4_VERSION;
            srcIPversion = IP_4_VERSION;
            initiatorIPversion = IP_4_VERSION;
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
            case CHORD_BYTE_SUCCESSOR:            return str = "SUCCESSOR";
            case CHORD_BYTE_FIND_SUCCESSOR:       return str = "FIND_SUCCESSOR";
            case CHORD_BYTE_PREDECESSOR:          return str = "PREDECESSOR";
            case CHORD_BYTE_FIND_PREDECESSOR:     return str = "FIND_PREDECESSOR";
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
            destIPversion = src.destIPversion;
            srcIPversion = src.srcIPversion;
            initiatorIPversion = src.initiatorIPversion;
            payloadSize = src.payloadSize;
            reserve = src.reserve;
            return *this;
        }

        friend ostream& operator<< (ostream& out, const chord_bits_flags& r);
    };

    //Uses for store info about received message from network (type of buffer is BUFF_RX_MESS) or planned for transmiting message into network (buffers types are >= BUFF_TX_JOIN and others <= BUFF_TX_SINGLE)
    class chord_byte_message_fields : public virtual base_message {
    public:
        byte             pid;
        chord_bits_flags flags;
        node_address destNodeIDwithSocket;
        node_address srcNodeIDwithSocket;                
        node_address initiatorNodeIDwithSocket;   
        uint messageID;       
        byte_array   payload;
        node_address searchedNodeIDwithSocket; //this should be into payload byte-array field       
        sc_time  netwokrStartTime;
        

        chord_byte_message_fields() {
            clear();
        }

        chord_byte_message_fields(const chord_byte_message_fields& src) {
            *this = src;
        }

        //chord_byte_message_fields* clone() const {
        //    chord_byte_message_fields* p = nullptr;
        //    p = new chord_byte_message_fields();
        //    *p = *this;
        //    return p;
        //}


        chord_byte_message_fields& operator= (const chord_byte_message_fields& src) {
            if (this == &src)
                return *this;

            base_message::operator=(src);
            pid = src.pid;
            flags = src.flags;
            destNodeIDwithSocket = src.destNodeIDwithSocket;
            srcNodeIDwithSocket = src.srcNodeIDwithSocket;            
            initiatorNodeIDwithSocket = src.initiatorNodeIDwithSocket;
            messageID = src.messageID;
            payload = src.payload;
            searchedNodeIDwithSocket = src.searchedNodeIDwithSocket;
            netwokrStartTime = src.netwokrStartTime;
            return *this;
        }

        void clear() {
            type = CHORD_UNKNOWN;
            pid = PID;
            flags.clear();
            destNodeIDwithSocket.clear();       
            srcNodeIDwithSocket.clear();            
            initiatorNodeIDwithSocket.clear();  
            messageID = 0; 
            payload.clear();
            searchedNodeIDwithSocket.clear(); 
            netwokrStartTime = SC_ZERO_TIME;
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
                case CHORD_RX_SUCCESSOR:           return str = "RX_SUCCESSOR";
                case CHORD_RX_FIND_SUCCESSOR:      return str = "RX_FIND_SUCCESSOR";
                case CHORD_RX_PREDECESSOR:         return str = "RX_PREDECESSOR";
                case CHORD_RX_FIND_PREDECESSOR:    return str = "RX_FIND_PREDECESSOR";
                case CHORD_RX_BROADCAST:           return str = "RX_BROADCAST";
                case CHORD_RX_MULTICAST:           return str = "RX_MULTICAST";
                case CHORD_RX_SINGLE:              return str = "RX_SINGLE";

                case CHORD_TX_JOIN:                return str = "TX_JOIN";
                case CHORD_TX_NOTIFY:              return str = "TX_NOTIFY";
                case CHORD_TX_ACK:                 return str = "TX_ACK";
                case CHORD_TX_SUCCESSOR:           return str = "TX_SUCCESSOR";
                case CHORD_TX_FIND_SUCCESSOR:      return str = "TX_FIND_SUCCESSOR";
                //case CHORD_TX_FWD_FIND_SUCCESSOR:      return str = "TX_FWD_FIND_SUCCESSOR";
                case CHORD_TX_PREDECESSOR:         return str = "TX_PREDECESSOR";
                case CHORD_TX_FIND_PREDECESSOR:    return str = "TX_FIND_PREDECESSOR";
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
            str += string("mID ") + to_string(messageID) + string(" ");
            str += string("pid ") + to_string(pid) + string(" ");
            str += string("f ") + flags.toStr() + string(" ");
            str += string("d ") + destNodeIDwithSocket.toStr() + string(" ");
            str += string("s ") + srcNodeIDwithSocket.toStr() + string(" ");            
            str += string("i ") + initiatorNodeIDwithSocket.toStr() + string(" ");           
            //string hex = searchedNodeIDwithSocket.to_string(SC_HEX_US); hex.erase(0, 4);
            str += string("sID ") + searchedNodeIDwithSocket.toStr() + string(" ");
            str += string("pS ") + to_string(payload.size());
            return str;
        }

        friend ostream& operator<< (ostream& out, const chord_byte_message_fields& r);
    };


    class chord_timer_message: public virtual chord_apptx_message {
    public:
       
       chord_byte_message_fields retryMess;
       sc_time checkTime;
       uint retryCounter;
       uint requestCounter;
       finite_state issuedState;
       bool isDelayed;


        chord_timer_message()/*: retryMess(nullptr)*/ {
            clear();
        }

        chord_timer_message(const chord_timer_message& src)/*: retryMess(nullptr)*/ {
            *this = src;
        }

        ~chord_timer_message() {
            clear();
        }

        chord_timer_message& operator= (const chord_timer_message& src) {
            if (this == &src)
                return *this;

            chord_apptx_message::operator=(src);

            retryMess = src.retryMess;
            //if (retryMess != nullptr) {
            //    delete retryMess;
            //    retryMess = nullptr;
            //}
            //if (src.retryMess != nullptr) {
            //    retryMess = new chord_byte_message_fields();
            //    *retryMess = *(src.retryMess);
            //}
            checkTime = src.checkTime;
            retryCounter = src.retryCounter;
            requestCounter = src.requestCounter;
            issuedState = src.issuedState;
            isDelayed = false;
            return *this;
        }

        void clear() {           
            chord_apptx_message::clear();            
            type = CHORD_TIMER_UNKNOWN;
            retryMess.clear();
            //if (retryMess != nullptr) {
            //    delete retryMess;
            //    retryMess = nullptr;
            //}
            checkTime = SC_ZERO_TIME;
            retryCounter = 0;
            requestCounter = 0;
            issuedState = STATE_UNKNOWN;
            isDelayed = false;
        }

        string type2str(const int& type = NONE) const {
            string str;
            uint t = type;
            if (type == NONE)
                t = this->type;

            switch (t) {
            case CHORD_TIMER_RX_ACK:               return str = "TIMER_ACK";
            case CHORD_TIMER_RX_SUCCESSOR:         return str = "TIMER_SUCCESSOR";
            case CHORD_TIMER_RX_SUCCESSOR_ON_JOIN: return str = "TIMER_SUCCESSOR_JOIN";
            case CHORD_TIMER_RX_PREDECESSOR:       return str = "TIMER_PREDECESSOR";
            case CHORD_TIMER_UPDATE:               return str = "TIMER_UPDATE";
            default:                               return str = "CHORD_UNKNOWN";
            }
        }

        string toStr() const {
            string str;
            switch (type) {
            case CHORD_TIMER_RX_ACK:
            case CHORD_TIMER_RX_SUCCESSOR:
            case CHORD_TIMER_RX_SUCCESSOR_ON_JOIN:
            case CHORD_TIMER_RX_PREDECESSOR:       
            case CHORD_TIMER_UPDATE:               return str = type2str() + string(" mID ") + to_string(retryMess.messageID) + string(" retryC ") + to_string(retryCounter) + string(" reqC ") + to_string(requestCounter);
            default:                               return str = "CHORD_UNKOWN"; 
            }
        }

        friend ostream& operator<< (ostream& out, const chord_timer_message& r);
    };
    

    class chord_message:
        public virtual app_message,
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

        friend ostream& operator<< (ostream& out, const chord_message& r);
    };
    

    class chord_byte_message {
    public:
        byte_array byteArray;
        uint size;
        chord_byte_message_fields fields;

        chord_byte_message()/* : fields(nullptr)*/ {
            clear();
        }

        chord_byte_message(const chord_byte_message& src)/* : fields(nullptr)*/ {
            *this = src;
        }

        chord_byte_message& operator= (const chord_byte_message& src) {
            if (this == &src)
                return *this;
            
            byteArray = src.byteArray;
            size      = src.size;

            fields    = src.fields;
            //if (fields != nullptr) {
            //    delete fields;
            //    fields = nullptr;
            //}
            //if (src.fields != nullptr) {
            //    fields = new chord_byte_message_fields;
            //    *fields = *(src.fields);
            //}
            return *this;
        }

        ~chord_byte_message() { 
            clear(); 
        }

        void clear() {
            byteArray.clear();
            size = 0;  
            fields.clear();
            //if (fields != nullptr) {                
            //    delete fields;
            //    fields = nullptr;
            //}
        }

        string toStr() const {
            string str;
            str += string("array ") + to_string(byteArray.size()) + string("|") + to_string(size) + string(" ") + fields.toStr(); //(fields == nullptr ? string("") : fields->toStr());
            return str;
        }

        friend ostream& operator<< (ostream& out, const chord_byte_message& r);
    };



}
#endif
