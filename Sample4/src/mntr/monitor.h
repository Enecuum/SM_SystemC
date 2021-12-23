#ifndef __MONITOR_H__
#define __MONITOR_H__

#include "inc.h"
#include "log.h"
#include "trp/trp_monitor_if.h"
#include "mntr/monitor_trp_if.h"

namespace P2P_MODEL
{
    enum monitor_mode {
        AUTO_VERIFY = 0,
        REF_VERIFY,
    };


    class monitor: public monitor_trp_if,
                   public sc_module,
                   public log
    {
    public:
        vector<sc_port<trp_monitor_if>* > trp_ports;
    
    private:
        //sc_event m_eventCheckFingers;
        sc_time  m_checkFingersPeriod;
        monitor_mode m_verifyMode;

        map<uint160, vector<node_address> >  m_id2refCwFingers;
        map<uint160, vector<node_address> >  m_id2refCcwFingers;
        
        map<uint160, node_snapshot> m_copySnapshot;       

        map<uint160, sc_time> m_howLongValidFingers;
        map<uint160, bool>    m_isValidFingers;

        uint m_fingersSize;
        uint m_nodes;

        struct invalid_fingers_time_intervals {
            sc_time beginTime;
            sc_time endTime;
            vector<node_address_latency> invalidFingers;
        };

        struct invalid_fingers_info {
            node_address node;
            vector<invalid_fingers_time_intervals> fingersIntervals;
        };

        map<uint160, invalid_fingers_info> m_id2invalidFingers;
        bool m_looksMotive;


    public:

        SC_HAS_PROCESS(monitor);

        monitor(sc_module_name _name, const uint nodes = 2, const uint fingersSize = 1, const bool looksMotive = true);
        ~monitor();        

        void createPorts(const uint nodes = 2);
        void setReferenceCwFingers(const uint160 id,  const vector<node_address>& fingers);
        void setReferenceCcwFingers(const uint160 id, const vector<node_address>& fingers);

        void setCwFingersUnderTest(const uint160 id,  const vector<node_address_latency>* fingers);
        void setCcwFingersUnderTest(const uint160 id, const vector<node_address_latency>* fingers);
        void setFiniteStateUnderTest(const uint160 id, const finite_state* state);
        void setNodeAddrUnderTest(const uint160 id, const node_address& addr);
        void setSnapshotUnderTest(const node_snapshot& snapshot);
        void setVerifyMode(const monitor_mode mode);

        //void setPeriodCheckFingers(const sc_time period);

        void checkFingersPeriodically();
        void check_fingers(const node_address&  addr,     vector<node_address_latency>& invalidFingers);
        void check_fingers(const node_snapshot& snapshot, vector<node_address_latency>& invalidFingers);

        bool verifySnapshots(const vector<node_address>& addrs);
        bool verifyByRefFingers(const uint160 id, const vector<node_address_latency>& nodeFingers, const vector<node_address>& refFingers, vector<node_address_latency>& invalidFingers);

        vector<node_address> genRefFingers(const vector<node_address>& addrs, const uint160 id, const bool isClockWise);
    };
}
#endif