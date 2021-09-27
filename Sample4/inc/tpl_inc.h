#ifndef __TRANSPORT_PLUS_LAYER_INC_H__
#define __TRANSPORT_PLUS_LAYER_INC_H__   //tpl = transport plus layer

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


        network_address(const string& _ip, const uint _inSocket, const uint _outSocket) {
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

        
        void set(const string _ip, const uint _inSocket, const uint _outSocket) {
            ip = _ip;
            inSocket = _inSocket;
            outSocket = _outSocket;
        } 

        network_address& operator= (const network_address& src) {
            if (this == &src)
                return *this;
            
            set(src.ip, src.inSocket, src.outSocket);
            return *this;
        }
        
        friend ostream& operator<< (ostream& out, const network_address& src);
    };



}
#endif