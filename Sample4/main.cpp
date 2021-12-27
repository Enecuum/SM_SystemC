#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>

#include "app/application.h"
#include "trp/transport_plus.h"
#include "net/network.h"
#include "mntr/monitor.h"

using namespace P2P_MODEL;
using namespace std;



const uint NODES                                = 5;
const bool LOOKS_MOTIVE                         = true;
const uint FINGERS_SIZE                         = (uint) ceil(log10(NODES) / log10(2));
const sc_time DELAY_TURN_ON                     = sc_time(FINGERS_SIZE * 1 + 10, SC_SEC);
const sc_time ADD_SIM_TIME                      = sc_time(300, SC_SEC);
const sc_time SIM_TIME                          = NODES*DELAY_TURN_ON + ADD_SIM_TIME;


//Definition for exter variables from inc.h
uint P2P_MODEL::MAX_SMALL_UINT                  = uint160(-1).to_uint();
sc_time P2P_MODEL::TRP_PERIOD_SNAPSHOTS = sc_time(1, SC_SEC);



int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");
    
    _setmaxstdio(2048);


    vector<string> argvStr;
    for (int i = 0; i < argc; ++i) {
        argvStr.push_back(argv[i]);
        auto itBegin = argvStr.at(i).begin();
        auto itEnd = argvStr.at(i).end();
        auto itRemoveSymbol = remove(itBegin, itEnd, ',');
        argvStr[i].erase(itRemoveSymbol, itEnd);
    }
    
    uint nodes = NODES;
    uint fingersSize = FINGERS_SIZE;
    application*    applications[NODES];
    transport_plus* transports[NODES];
    network*        network1 = nullptr;    
    monitor*        monitor1 = nullptr;
    string str;


    //Set simulating scenarios of message issuing for Application layer
    sim_message mess;
    vector<network_address> addrs;


    
    mess.type = SIM_HARD_RESET;
    mess.amount = 1;
    mess.firstDelay = sc_time(0, SC_MS);

    bool testNonContinuosNumbers = true;

    network1 = new network("netw", nodes);
    monitor1 = new monitor("monitor", nodes, fingersSize, LOOKS_MOTIVE);
    for (uint i = 0; i < nodes; ++i) {
        str = string("app") + to_string(i);
        applications[i] = new application(str.c_str());
        str = string("trp") + to_string(i);
        transports[i] = new transport_plus(str.c_str());

        applications[i]->trp_port.bind( *(transports[i]) );
        transports[i]->network_port.bind(*network1);
        transports[i]->monitor_port.bind(*monitor1);
        network1->trp_ports[i]->bind( *(transports[i]) );
        monitor1->trp_ports[i]->bind( *(transports[i]) );        

        applications[i]->setEnabledLog();
        applications[i]->setLogMode(ALL_LOG);        
        str = "./log/app" + string(".txt");
        applications[i]->setPathLog(str.c_str());
        applications[i]->msgLog(applications[i]->name(), LOG_TXRX, LOG_INFO, "create", ALL_LOG);

        transports[i]->setEnabledLog();
        transports[i]->setLogMode(ALL_LOG);
        str = "./log/trp" + to_string(i) + string(".txt");
        transports[i]->setPathLog(str);
        str = "./log/snapshot.txt";
        transports[i]->setSnapshotPathLog(str);
        transports[i]->msgLog(transports[i]->name(), LOG_TXRX, LOG_INFO, "create", ALL_LOG);
        

        if (testNonContinuosNumbers == true) {
            switch (i) {
            case 0:
                mess.firstDelay = sc_time(0, SC_SEC);
                break;

            default:
                mess.firstDelay = (SIM_TIME-ADD_SIM_TIME)-i*DELAY_TURN_ON;
                break;
            }
            applications[i]->pushSimulatingMess(mess);
        }
        else {
            applications[i]->pushSimulatingMess(mess);
            mess.firstDelay += DELAY_TURN_ON;                   //sc_time(10, SC_SEC);
        }

        chord_conf_parameters params;
        str = string("192.168.0.") + to_string(i);
        params.netwAddr.set(str.c_str(), 1111, 2222);
        params.setDefaultTimersCountersFingersSize();
        params.fingersSize = fingersSize;
        addrs.push_back(params.netwAddr);
        
        if (i >= 1) {
            params.seed.push_back(addrs.at(0));
        }
        transports[i]->setConfParameters(params);
       
        monitor1->setSnapshotUnderTest(transports[i]->snapshot_pointers());
    }


    
    //mess.clear();
    //mess.type = SIM_PAUSE;
    //mess.amount = 1;
    //mess.firstDelay = sc_time(0.0, SC_SEC);
    //applications[1]->pushSimulatingMess(mess);
    //
    //
    //mess.clear();
    //mess.type = SIM_CONTINUE;
    //mess.amount = 1;
    //mess.firstDelay = sc_time(30, SC_SEC);
    //applications[1]->pushSimulatingMess(mess);

    network1->setEnabledLog();
    network1->setLogMode(ALL_LOG);
    network1->setPathLog("./log/net.txt");

    monitor1->setEnabledLog();
    monitor1->setLogMode(ALL_LOG);
    monitor1->setPathLog("./log/snapshot.txt");

    //Set configuration parameters of Network model (simplified simulation model of TCP/UDP network) 
    network1->setNodeAddressList(addrs);
    network1->setRandomLatencyTable(10, 10, 0);
    network1->msgLog(network1->name(), LOG_TXRX, LOG_INFO, "create", ALL_LOG);
    network1->msgLog(network1->name(), LOG_TXRX, LOG_INFO, string("latencies, ms: \n") + network1->latencyTableToStr(), ALL_LOG);

    //Set configuration parameters of Monitor    
    monitor1->msgLog(monitor1->name(), LOG_TXRX, LOG_INFO, "create", ALL_LOG);

    //Set valid reference clockwise fingers for every node
    vector<node_address> fingers;
    uint index;
    fingers.resize(DEFAULT_FINGERS_SIZE, node_address());

    index = 0;
    fingers[index].id = 1; ++index;
    fingers[index].id = 2; ++index;
    monitor1->setReferenceCwFingers(0, fingers);

    index = 0;
    fingers[index].id = 2; ++index;
    fingers[index].id = 3; ++index;
    monitor1->setReferenceCwFingers(1, fingers);

    index = 0;
    fingers[index].id = 3; ++index;
    fingers[index].id = 3; ++index;
    monitor1->setReferenceCwFingers(2, fingers);

    index = 0;
    fingers[index].id = 0; ++index;
    fingers[index].id = 2; ++index;
    monitor1->setReferenceCwFingers(3, fingers);


    //Set valid reference counter clockwise fingers for every node
    index = 0;
    fingers[index].id = 3; ++index;
    fingers[index].id = 3; ++index;
    monitor1->setReferenceCcwFingers(0, fingers);

    index = 0;
    fingers[index].id = 0; ++index;
    fingers[index].id = 3; ++index;
    monitor1->setReferenceCcwFingers(1, fingers);

    index = 0;
    fingers[index].id = 1; ++index;
    fingers[index].id = 0; ++index;
    monitor1->setReferenceCcwFingers(2, fingers);

    index = 0;
    fingers[index].id = 2; ++index;
    fingers[index].id = 1; ++index;
    monitor1->setReferenceCcwFingers(3, fingers);



    
    //Run simulation
    cout << endl << "Run simulation" << endl;

    
    sc_start(SIM_TIME);
    cout << endl << "Read results" << endl;


    //Free memory layer-by-layer
    char c = 'y';
    cout << endl << "*** Prepare to free memory used by applications. Enter 'y': "; //cin >> c;
    cout << endl;
    for (uint i = 0; i < nodes; ++i) {
        delete applications[i];
        applications[i] = nullptr;
    }
    
    cout << "*** Prepare to free memory used by low_latency_chord. Enter 'y': "; //cin >> c;
    cout << endl;
    for (uint i = 0; i < nodes; ++i) {
        delete transports[i];
        transports[i] = nullptr;
    }
    
    cout << "*** Prepare to free memory used by network. Enter 'y': "; //cin >> c;
    cout << endl;
    delete network1;
    network1 = nullptr;

    delete monitor1;
    monitor1 = nullptr;
    
    cout << "*** Enter 'y' to exit: "; //cin >> c;
    return 0;
}




//mess.clear();
//mess.destNetwAddrs;
//mess.type = SIM_SINGLE;
//mess.payloadType = DATA;  
//mess.amount = 1;
//mess.period = sc_time(15, SC_SEC);
//mess.firstDelay = sc_time(0, SC_MS);
//mess.randType[RAND_DEST] = true;
//mess.randFrom[RAND_DEST] = 0;
//mess.randTo[RAND_DEST] = 256;
//mess.randAmount[RAND_DEST] = 1;
//mess.timeUnit = SC_MS;
//mess.dataSize = 1000;
//application2.pushSimulatingMess(mess);

//mess.clear();
//mess.destNetwAddrs;
//mess.type = SIM_MULTICAST;
//mess.payloadType = DATA;
//mess.amount = 4;
//mess.period = sc_time(15, SC_SEC);
//mess.firstDelay = sc_time(0, SC_MS);
//mess.randType[RAND_DEST] = true;
//mess.randFrom[RAND_DEST] = 0;
//mess.randTo[RAND_DEST] = 256;
//mess.randAmount[RAND_DEST] = 3;
//mess.randNeedRecalc[RAND_DEST] = true;
//mess.timeUnit = SC_MS;
//mess.dataSize = 1000;
//application2.pushSimulatingMess(mess);
