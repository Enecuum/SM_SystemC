#ifndef __TRANSPORT_PLUS_H__ 
#define __TRANSPORT_PLUS_H__

#include "inc.h"
#include "log.h"
#include "trp/low_latency_chord.h"
#include "trp/trp_application_if.h"
#include "trp/trp_network_if.h"
#include "trp/trp_llchord_if.h"
#include "net/network_trp_if.h"





namespace P2P_MODEL
{

    class transport_plus : public sc_module,
                           public log,
                           public trp_application_if,
                           public trp_network_if,
                           public trp_llchord_if
    {
    public:
        sc_port<network_trp_if> network_port;        

    private:
        low_latency_chord m_llchord;

    public:

        SC_HAS_PROCESS(transport_plus);

        transport_plus(sc_module_name name);
        ~transport_plus();

        void config_req(const app_message& mess);
        void mess_req(const app_message& mess);
        void send_mess(const chord_byte_message& mess);
        void receive_mess(const chord_byte_message& mess);

        void setNetworkAddress(const network_address& addr);
        network_address& getNetworkAddress();

        //chord_conf_message&        appMess2chordConfMess(const app_message& mess);
        //chord_byte_message_fields& chordByteMess2ChordByteMessFields(const chord_byte_message& mess);
        chord_message appMess2chordMess(const app_message& mess);
        chord_message chordByteMess2ChordMess(const chord_byte_message& mess);
        


        void setPathLog(const string& pathLog);
        void setLogMode(const log_mode& mode);
        void setDisabledLog();
        void setEnabledLog();

        void setSeedNodes(const vector<network_address>& seed);

        void setConfParameters(const chord_conf_parameters& params);


    public:
    //private:
        node_address& getNodeAddress();
    };

}
#endif