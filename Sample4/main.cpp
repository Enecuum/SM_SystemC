#include <iostream>
#include <vector>
#include <string>

#include "app/application.h"
#include "trp/transport_plus.h"
//#include "trp/low_latency_chord.h"
#include "net/network.h"



using namespace P2P_MODEL;
using namespace std;

//Rdma_Project::trafgen trafgen1("trafgen1");
//Rdma_Project::trafgen trafgen2("trafgen2");
//Rdma_Project::rdma rdma1("rdma1");
//Rdma_Project::rdma rdma2("rdma2");
//basicchannel channel12("channel_12");
//basicchannel channel21("channel_21");

//void init_configuration() {
//    trafgen1.rdma_port.bind(*(rdma1.UpDataWrap));
//    rdma1.UpDataWrap->up_data_wrap_sap_port.bind(trafgen1);
//    rdma1.LowDataWrap->low_data_wrap_basicchannel_port.bind(channel12);
//
//    trafgen2.rdma_port.bind(*(rdma2.UpDataWrap));
//    rdma2.UpDataWrap->up_data_wrap_sap_port.bind(trafgen2);
//    rdma2.LowDataWrap->low_data_wrap_basicchannel_port.bind(channel21);
//
//    channel12.wrappertx_port.bind(*(rdma1.LowDataWrap));
//    channel12.wrapperrx_port.bind(*(rdma2.LowDataWrap));
//
//    channel21.wrappertx_port.bind(*(rdma2.LowDataWrap));
//    channel21.wrapperrx_port.bind(*(rdma1.LowDataWrap));
//
//    trafgen1.setEnabled(true);
//    trafgen1.setLogMode(ALL_LOG);
//    trafgen1.setLogAddress(std::vector<uint>(1, 32));
//    trafgen1.setDataLogLimit(2);
//    trafgen1.setMaxLengthMethodAndTimeLog(30, 12);
//
//    trafgen2.setEnabled(true);
//    trafgen2.setLogMode(ALL_LOG);
//    trafgen2.setLogAddress(std::vector<uint>(1, 33));
//    trafgen2.setDataLogLimit(2);
//    trafgen2.setMaxLengthMethodAndTimeLog(30, 10);
//
//    rdma1.setEnabledLog(true);
//    rdma1.setLogMode(ALL_LOG);
//    rdma1.setDataLogLimit(2);
//    rdma1.setMaxLengthMethodAndTimeLog(44, 10);
//
//    rdma2.setEnabledLog(true);
//    rdma2.setLogMode(ALL_LOG);
//    rdma2.setDataLogLimit(2);
//    rdma2.setMaxLengthMethodAndTimeLog(44, 10);
//
//}
//
//void init_speed_buffers(double gbps, int maxPackets, int maxSize, unsigned int maxAppMemSize) {
//    channel12.setSpeed(gbps);
//    channel21.setSpeed(gbps);
//
//    rdma1.setBufferSizeInBytes(maxPackets*maxSize, maxPackets*maxSize);
//    rdma1.setBufferSizeInPackets(maxPackets, maxPackets);
//
//    rdma2.setBufferSizeInBytes(maxPackets*maxSize, maxPackets*maxSize);
//    rdma2.setBufferSizeInPackets(maxPackets, maxPackets);
//
//    trafgen1.setInnerMemmorySize(maxAppMemSize);
//    trafgen2.setInnerMemmorySize(maxAppMemSize);
//}
//
//void init_message_transmission_parameters(message_type type, int amount, int paylodLength,  bool verify, bool reply, bool increment, double periodMS) {
//    message_type messageType = type;
//    unsigned int destLogicalAddress = 33;
//    unsigned int identificationNumber = 0;
//    unsigned int destApplicationID = 33;
//    unsigned int sourceApplicationID = 32;
//    unsigned int sourceLogicalAddress = 32;
//    int key = 0;
//    bool replyFlag = reply;
//    unsigned int memoryAddress = 1000;
//    bool incrementMemmory = increment;
//    bool verifyData = verify;
//    unsigned int dataLength = paylodLength;
//    unsigned int amountMessages = amount;
//    sc_time packetPeriod = SimTime(periodMS, SC_MS);
//    sc_time sendingDelay = SimTime(0, SC_MS);
//
//    message_generation_parameters message(messageType, destLogicalAddress, identificationNumber,
//        destApplicationID, sourceApplicationID, sourceLogicalAddress, key, replyFlag, memoryAddress,
//        incrementMemmory, verifyData, dataLength, amountMessages, packetPeriod, sendingDelay);
//
//    messages_config messagesConfig;
//    messagesConfig.message.push_back(message);
//    messagesConfig.type.push_back(message.messageType);
//    trafgen1.setMessageGenerationParameters(messagesConfig);  //НАСТРОЙКА trafgen   
//}

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

    //Set initial settings of Application layer and Transport+ layer
    application1.setEnabledLog();
    application1.setLogMode(ALL_LOG);
    application1.setPathLog("./log/app.txt");

    transport1.setEnabledLog();
    transport1.setLogMode(ALL_LOG);
    transport1.setPathLog("./log/trp.txt");

    application2.setEnabledLog();
    application2.setLogMode(ALL_LOG);
    application2.setPathLog("./log/app.txt");

    transport2.setEnabledLog();
    transport2.setLogMode(ALL_LOG);
    transport2.setPathLog("./log/trp.txt");


    network1.setEnabledLog();
    network1.setLogMode(ALL_LOG);
    network1.setPathLog("./log/net.txt");


    //Set simulating scenarios of message issueing for Application layer
    sim_request req;

    //req.clear();
    //req.type = SIM_HARD_RESET;
    //req.amount = 1;    
    //req.firstDelay = sc_time(1, SC_MS);
    //application1.pushSimulatingReq(req);


    //req.clear();
    //req.destination;
    //req.type = SIM_SINGLE;
    //req.payloadType = DATA;  
    //req.amount = 1;
    //req.period = sc_time(15, SC_SEC);
    //req.firstDelay = sc_time(0, SC_MS);
    //req.randType[RAND_DEST] = true;
    //req.randFrom[RAND_DEST] = 0;
    //req.randTo[RAND_DEST] = 256;
    //req.randAmount[RAND_DEST] = 1;
    //req.timeUnit = SC_MS;
    //req.dataSize = 1000;
    //application2.pushSimulatingReq(req);

    //req.clear();
    //req.destination;
    //req.type = SIM_MULTICAST;
    //req.payloadType = DATA;
    //req.amount = 4;
    //req.period = sc_time(15, SC_SEC);
    //req.firstDelay = sc_time(0, SC_MS);
    //req.randType[RAND_DEST] = true;
    //req.randFrom[RAND_DEST] = 0;
    //req.randTo[RAND_DEST] = 256;
    //req.randAmount[RAND_DEST] = 3;
    //req.randNeedRecalc[RAND_DEST] = true;
    //req.timeUnit = SC_MS;
    //req.dataSize = 1000;
    //application2.pushSimulatingReq(req);

    req.clear();
    req.type = SIM_HARD_RESET;
    req.amount = 1;
    req.period = sc_time(5, SC_SEC);
    req.firstDelay = sc_time(1, SC_MS);
    application2.pushSimulatingReq(req);

    //req.clear();
    //req.type = SIM_SOFT_RESET;
    //req.amount = 4;
    //req.period = sc_time(6, SC_SEC);
    //req.firstDelay = sc_time(2, SC_MS);
    //application2.pushSimulatingReq(req);

    //req.clear();
    //req.type = SIM_FLUSH;
    //req.amount = 5;
    //req.period = sc_time(7, SC_SEC);
    //req.firstDelay = sc_time(3, SC_MS);
    //application2.pushSimulatingReq(req);


    //Set configuration parameters of Transport+ layer
    chord_conf_parameters params1, params2;    
    params1.netwAddr.set("192.168.0.1", 4444, 1111);
    params2.netwAddr.set("192.168.0.2", 4444, 1111);
    
    params1.setDefaultTimersCountersFingersSize();

    params2.setDefaultTimersCountersFingersSize();
    params2.seed.push_back(params1.netwAddr);
   
    transport1.setConfParameters(params1);
    transport2.setConfParameters(params2);

    //Set configuration parameters of network model
    node_address a;
    a.set(params1.netwAddr);    
    network1.pushLatency(a.id, sc_time(50, SC_MS));

    a.set(params2.netwAddr);
    network1.pushLatency(a.id, sc_time(70, SC_MS));


    //LOG
    node_address nodeAddr1, nodeAddr2;
    nodeAddr1 = transport1.getNodeAddress();
    cout << nodeAddr1 << endl;
    nodeAddr2 = transport2.getNodeAddress();
    cout << nodeAddr2 << endl;
    
    
    
    //Run simulation
    cout << endl << "Run simulation" << endl;

    sc_start(60*10, SC_SEC);
    cout << endl << "Read results" << endl;


    cout << endl << "*** Press Eneter to exit ***"; getchar();
    return 0;
}
