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
        //sc_event m_eventSendMess;

    private:
        low_latency_chord m_llchord;

    public:

        SC_HAS_PROCESS(transport_plus);

        transport_plus(sc_module_name name);
        ~transport_plus();

        void config_req(const app_message&);
        void mess_req(const app_message&);
        void send_mess(const chord_message &);
        void receive_mess(const raw_chord_message&);

        void setNetworkAddress(const network_address& addr);
        network_address& getNetworkAddress();

        chord_message& appMess2chordMess(const app_message& );
        chord_message& rawChordMess2ChordMess(const raw_chord_message&);


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