#include <iostream>
#include <vector>
#include <string>

#include "app/application.h"
#include "tpl/transport_plus.h"
#include "tpl/low_latency_chord.h"

//#include "trafgen.h"
//#include "basicchannel.h"


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

    //init_configuration();

    //unsigned int testScenarioN = 1;
    //unsigned int testBuferrsN = 1;
    //if (argc >= 2)
    //    testScenarioN = atoi(argvStr.at(1).c_str());
    //if (argc >= 3)
    //    testBuferrsN = atoi(argvStr.at(2).c_str());


    //switch (testScenarioN)
    //{
    //case 0:
    //default:                                           //amount datasize           verify reply  increment period
    //    init_message_transmission_parameters(WRITE_COMMAND, 3,  1024,               false, false, true,      0.5); break;
    //case 1:
    //    init_message_transmission_parameters(WRITE_COMMAND, 10,  1*1024*1024 ,       true, true, true,        30); break;
    //case 2:
    //    init_message_transmission_parameters(READ_COMMAND, 3,   1024,               false, true, true,        10); break;
    //case 3:
    //    init_message_transmission_parameters(READ_COMMAND, 10,  1*1024*1024,        false, true, true,        40); break;
    //case 4:
    //    init_message_transmission_parameters(RMW_COMMAND,   3,           2,         true, true, true,          1); break;
    //case 5:
    //    init_message_transmission_parameters(RMW_COMMAND,   10,         8,          true, true, true,          1); break;
    //}



    //switch (testBuferrsN)
    //{
    //case 0:
    //default:
    //    init_speed_buffers(1.25, 4, 1024, 512);
    //    break;

    //case 1:
    //    init_speed_buffers(1.25, 4, 1024*1024, 512);
    //    break;

    //case 2:
    //    init_speed_buffers(1.25, 4, 1024*1024, 2*1024*1024);
    //    break;
    //}

    application    application1("application_1");
    application    application2("application_2");
    
    transport_plus   transport1("transport_1");
    transport_plus   transport2("transport_2");
    

    //ЗАДАТЬ НАЧАЛЬНЫЕ НАСТРОЙКИ ПРИКЛАДНЫХ УРОВНЕЙ
    application1.setEnabledLog();
    application1.setLogMode(EXTERNAL_LOG);
    application1.setPathLog("./log/app.txt");

    application2.setEnabledLog();
    application2.setLogMode(EXTERNAL_LOG);
    application2.setPathLog("./log/app.txt");


    //ЗАДАТЬ СЦЕНАРИИ ВЫДАЧИ СООБЩЕНИЙ ОТ ПРИКЛАДНЫХ УРОВНЕЙ



    //ЗАДАТЬ КОНФИГУРАЦИОННЫЕ ПАРАМЕТРЫ ДЛЯ Transport+ УРОВНЯ
    network_address networkAddr1, networkAddr2;
    networkAddr1.set("192.168.0.1", 4444, 1111);
    networkAddr2.set("192.168.0.2", 4444, 1111);
   
    transport1.setNetworkAddress(networkAddr1);
    transport2.setNetworkAddress(networkAddr2);
    node_address nodeAddr1, nodeAddr2;
    nodeAddr1 = transport1.getNodeAddress();
    cout << nodeAddr1 << endl;
    nodeAddr2 = transport2.getNodeAddress();
    cout << nodeAddr2 << endl;
    
    
    
    
    cout << endl << "Run simulation" << endl;

    sc_start(10, SC_SEC);
    cout << endl << "Read results" << endl;


    cout << endl << "*** Press Eneter to exit ***"; getchar();
    return 0;
}
