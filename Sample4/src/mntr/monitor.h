#ifndef __MONITOR_H__
#define __MONITOR_H__

#include "inc.h"
#include "log.h"
#include "trp/trp_monitor_if.h"
#include "mntr/monitor_trp_if.h"

namespace P2P_MODEL
{

    class monitor: public monitor_trp_if,
                   public sc_module,
                   public log
    {
    public:
        vector<sc_port<trp_monitor_if>* > trp_ports;
    
    private:
        sc_event m_eventCheckFingers;
        sc_time  m_checkFingersPeriod;

        map<uint160, vector<node_address> > m_id2refCwFingers;
        map<uint160, vector<node_address> > m_id2refCcwFingers;
        
       
        
        map<uint160, node_snapshot> m_copySnapshot;
        
        //map<uint160, const vector<node_address_latency>* > m_copyCwFingers;
        //map<uint160, const vector<node_address_latency>* > m_copyCcwFingers;
        //map<uint160, node_address> m_copyNodeAddrs;

        map<uint160, sc_time> m_howLongValidFingers;
        map<uint160, bool> m_isValidFingers;


    public:

        SC_HAS_PROCESS(monitor);

        monitor(sc_module_name _name, const uint nodes = 2);
        ~monitor();        

        void createPorts(const uint nodes = 2);
        void setReferenceCwFingers(const uint160 id,  const vector<node_address>& fingers);
        void setReferenceCcwFingers(const uint160 id, const vector<node_address>& fingers);

        void setCwFingersUnderTest(const uint160 id,  const vector<node_address_latency>* fingers);
        void setCcwFingersUnderTest(const uint160 id, const vector<node_address_latency>* fingers);
        void setFiniteStateUnderTest(const uint160 id, const finite_state* state);
        void setNodeAddrUnderTest(const uint160 id, const node_address& addr);
        void setSnapshotUnderTest(const node_snapshot& snapshot);

        void setPeriodCheckFingers(const sc_time period);



        void checkFingersPeriodically();

        void check_fingers(const node_address&  addr,     vector<node_address_latency>& invalidFingers);
        void check_fingers(const node_snapshot& snapshot, vector<node_address_latency>& invalidFingers);


    };
}
#endif