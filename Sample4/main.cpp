#include <iostream>
#include <vector>
#include <string>

#include "app/application.h"
#include "trp/transport_plus.h"
#include "net/network.h"



using namespace P2P_MODEL;
using namespace std;



int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");

    vector<string> argvStr;
    for (int i = 0; i < argc; ++i) {
        argvStr.push_back(argv[i]);
        auto itBegin = argvStr.at(i).begin();
        auto itEnd = argvStr.at(i).end();
        auto itRemoveSymbol = remove(itBegin, itEnd, ',');
        argvStr[i].erase(itRemoveSymbol, itEnd);
    }
    

    application    application1("application_1");
    application    application2("application_2");
    
    transport_plus   transport1("transport_1");
    transport_plus   transport2("transport_2");

    uint nodes = 2;
    network          network1("network", nodes);;

    application1.trp_port.bind(transport1);
    application2.trp_port.bind(transport2);

    transport1.network_port.bind(network1);
    transport2.network_port.bind(network1);

    
    uint nodeIndex = 0;    
    network1.trp_ports[nodeIndex++]->bind(transport1);
    network1.trp_ports[nodeIndex++]->bind(transport2);

    //Set initial settings of layers: Application, Transport+ and Network model
    application1.setEnabledLog();
    application1.setLogMode(ALL_LOG);
    application1.setPathLog("./log/app1.txt");

    transport1.setEnabledLog();
    transport1.setLogMode(ALL_LOG);
    transport1.setPathLog("./log/trp1.txt");

    application2.setEnabledLog();
    application2.setLogMode(ALL_LOG);
    application2.setPathLog("./log/app2.txt");

    transport2.setEnabledLog();
    transport2.setLogMode(ALL_LOG);
    transport2.setPathLog("./log/trp2.txt");

    network1.setEnabledLog();
    network1.setLogMode(ALL_LOG);
    network1.setPathLog("./log/net.txt");


    //Set simulating scenarios of message issuing for Application layer
    sim_message mess;

    //mess.clear();
    mess.type = SIM_HARD_RESET;
    mess.amount = 1;
    mess.firstDelay = sc_time(0, SC_MS);
    application1.pushSimulatingMess(mess);


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

    mess.clear();
    mess.type = SIM_HARD_RESET;
    mess.amount = 1;
    mess.firstDelay = sc_time(1, SC_MS);
    application2.pushSimulatingMess(mess);

    //mess.clear();
    //mess.type = SIM_SOFT_RESET;
    //mess.amount = 4;
    //mess.period = sc_time(6, SC_SEC);
    //mess.firstDelay = sc_time(2, SC_MS);
    //application2.pushSimulatingMess(mess);

    //mess.clear();
    //mess.type = SIM_FLUSH;
    //mess.amount = 5;
    //mess.period = sc_time(7, SC_SEC);
    //mess.firstDelay = sc_time(3, SC_MS);
    //application2.pushSimulatingMess(mess);


    //Set configuration parameters of Transport+ layer
    chord_conf_parameters params1, params2;    
    params1.netwAddr.set("C0.A8.0.1", 0x115C, 0x0457);
    params2.netwAddr.set("C0.A8.0.2", 0x115C, 0x0457);
    
    params1.setDefaultTimersCountersFingersSize();

    params2.setDefaultTimersCountersFingersSize();
    params2.seed.push_back(params1.netwAddr);
   
    transport1.setConfParameters(params1);
    transport2.setConfParameters(params2);


    //Set configuration parameters of Network model (simplified simulation model of TCP/IP network or network on UDP protocol)    
    vector<network_address> addrs;
    addrs.push_back( params1.netwAddr );
    addrs.push_back( params2.netwAddr );

    network1.setNodeAddressList(addrs);
    network1.setRandomLatencyTable(100, 150, 0);
    network1.msgLog(network1.name(), LOG_TXRX, LOG_INFO, string("latencies, ms: \n") + network1.latencyTableToStr(), ALL_LOG);



    //LOG
    node_address nodeAddr1, nodeAddr2;
    nodeAddr1 = transport1.getNodeAddress();
    cout << nodeAddr1 << endl;
    nodeAddr2 = transport2.getNodeAddress();
    cout << nodeAddr2 << endl;
    
    
    
    //Run simulation
    cout << endl << "Run simulation" << endl;

    sc_start(60*100, SC_SEC);
    cout << endl << "Read results" << endl;


    cout << endl << "*** Press Enter to exit ***"; getchar();
    return 0;
}
