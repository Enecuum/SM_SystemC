#ifndef __TRANSPORT_PLUS_H__ 
#define __TRANSPORT_PLUS_H__

#include "inc.h"
#include "log.h"
#include "trp/low_latency_chord.h"
#include "trp/trp_application_if.h"




namespace P2P_MODEL
{

    class transport_plus : public sc_module,
                           public log,
                           public trp_application_if
    {
    private:

        //sc_event m_eventReceiveMess;
        //sc_event m_eventSendMess;

        low_latency_chord m_llchord;

    public:

        SC_HAS_PROCESS(transport_plus);

        transport_plus(sc_module_name name);
        ~transport_plus();

        void config_req(const app_request& req);
        void mess_req(const app_request& req);
        void send_mess();
        void receive_mess();

        void setNetworkAddress(const network_address& addr);
        network_address& getNetworkAddress();

        chord_request& appReq2trpReq(const app_request& req);

        void setPathLog(const string& pathLog);
        void setLogMode(const log_mode& mode);
        void setDisabledLog();
        void setEnabledLog();

        void setSeedNodes(const vector<network_address> seed);


    public:
    //private:
        node_address& getNodeAddress();
    };

}
#endif