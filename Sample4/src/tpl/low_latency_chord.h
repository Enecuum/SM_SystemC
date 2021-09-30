#ifndef __LOW_LATENCY_CHORD_H__
#define __LOW_LATENCY_CHORD_H__

#include "inc.h"
#include "sha1.hpp"

using namespace std;



namespace P2P_MODEL
{

    class node_address: public network_address,
                        public sc_module {
    private:        
        network_address m_tmp;

    public:
        uint160 id;

        //SC_HAS_PROCESS(node_address);

        node_address(sc_module_name _name = "node_address"): sc_module(_name) {
            reset();
        };

        node_address(const node_address& src, sc_module_name name = "node_address"): sc_module(name) {
            set(src);
        }


        node_address(const network_address& src, sc_module_name name = "node_address"): sc_module(name) {
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
            strID.insert(0, "0xus").c_str();
            uint160 res = strID.c_str();            
            return res;
        }
    };






    
   



    class low_latency_chord {
    private:
        network_address m_netwAddr;
        node_address m_nodeAddr;   //ÀÄÐÅÑ, ÈÑÏÎËÜÇÓÅÌÛÉ ÄËß ÈÄÅÍÒÈÔÈÊÀÖÈÈ ÓÇËÀ ÍÀ Transport+ ÓÐÎÂÍÅ ïî ID, âû÷èñëÿåìîì, êàê SHA-1

    public:
        
        low_latency_chord();
        //low_latency_chord(sc_module_name _name);
        ~low_latency_chord();

        void              setNetworkAddress(const network_address& _netwAddr);      
        network_address&  getNetworkAddress();        
        node_address&     getNodeAddress();
    };
}
#endif


