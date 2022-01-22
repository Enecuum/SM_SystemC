

#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>

#include "app/application.h"
#include "trp/transport_plus.h"
#include "net/network.h"
#include "mntr/monitor.h"

#include "nlohmann/json.hpp"

#include <algorithm>
#include <random>
#include <typeinfo>
#include <type_traits>
#include <cstdint>

using namespace P2P_MODEL;
using namespace std;

using json = nlohmann::json;


//Replaced by values from config.txt
const uint         DEFAULT_NODES                           = 10;
const bool         DEFAULT_GEN_RANDOM                      = false;
const bool         DEFAULT_SHUFFLE                         = false;
const sc_time      DEFAULT_SNAPSHOT_PERIOD_SEC             = sc_time(1, SC_SEC);

//Pre-defined parameters
const bool         SHOW_MOTIVE       = false;
const monitor_mode MONITOR_MODE      = NO_VERIFY;
const bool         SHOW_CCW_FINGERS  = false;

const sc_time      ADD_SIM_TIME     = sc_time(100, SC_SEC);


const int    JSON_FAILED      = -1;


//Definition for extern variables from inc.h
uint160 P2P_MODEL::MAX_UINT160          = uint160(-1);
uint    P2P_MODEL::MAX_SMALL_UINT       = P2P_MODEL::MAX_UINT160.to_uint();
bool    P2P_MODEL::RANDOM_IDS           = false;
bool    P2P_MODEL::CCW_FINGERS_TURN_ON  = false; 
bool    P2P_MODEL::MAKE_SNAPSHOT_ALWAYS = false;
sc_time P2P_MODEL::TRP_PERIOD_SNAPSHOTS = sc_time(1, SC_SEC);



//GLOBAL VARIABLES
vector<uint64_t> arrayID = {12, 4, 2, 10, 1, 14, 5, 3, 11, 7}; 
uint         NODES;
bool         GEN_RANDOM;
bool         SHUFFLE;
sc_time      SNAPSHOT_PERIOD_SEC;
uint         FINGERS_SIZE;
sc_time      DELAY_TURN_ON;
sc_time      SIM_TIME;

bool jsonParser(const string& path) {
    cout << "Read config file: " << path << endl;
    ifstream file(path.c_str());
    json j;
    try {
        file >> j; 
        //cout << setw(4) << j;

        if (j["gen_random"].get<bool>()) {
            GEN_RANDOM = true;
            NODES = j["nodes"].get<uint>();
            arrayID.clear();
        }
        else {  
            GEN_RANDOM = false;
            vector<uint64_t> arr = j["arrayID"].get<vector<uint64_t>>();
            arrayID = arr;
            NODES = (uint) arrayID.size();
        }
        double value = j["snapshot_period_sec"].get<double>();
        SNAPSHOT_PERIOD_SEC = sc_time(value, SC_SEC);
        bool flag = j["shuffle"].get<bool>();
        SHUFFLE = flag;
    }
    catch (...) {
        cout << endl << "File is invalid. Check all lines in JSON file !!!" << endl;
        return false;
    }
    cout << endl << endl;
    return true;    
}





int main(int argc, char* argv[])
{

    //631 536 127 141 329 173 44

    /*631,
        536,
        127,
        141,
        329,
        173,
        44,
        985,
        796,
        531,
        525,
        356,
        166*/

    std::cout << std::boolalpha;

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
    
    string path = ("./config.txt");
    
    if (jsonParser(path) == false) {
        NODES                = DEFAULT_NODES;
        GEN_RANDOM           = DEFAULT_GEN_RANDOM;
        SHUFFLE              = DEFAULT_SHUFFLE;
        SNAPSHOT_PERIOD_SEC  = DEFAULT_SNAPSHOT_PERIOD_SEC;        
        cout << "Please, input 'y' to exit " << endl; getchar();
        return JSON_FAILED;
        //cout << "Pre-difined parameters will be used" << endl;
        //print here pre-defined parameters
    }
    
    //Gen random IDs for nodes or do shuffling
    auto rng = std::default_random_engine{(unsigned int)time(0)};
    if (GEN_RANDOM) {
        vector<uint64_t> possibleNodes;
        possibleNodes.reserve(NODES);
        for (uint i = 0; i <= MAX_UINT160; ++i) {
            possibleNodes.push_back(i);
        }
        
        //if (SHUFFLE)
        {
            for (int i = 0; i < 5; ++i)
                shuffle(std::begin(possibleNodes), std::end(possibleNodes), rng);
        }
        
        arrayID.assign(possibleNodes.begin(), possibleNodes.begin()+NODES);
    }
    else if (SHUFFLE) {
        for (int i = 0; i < 5; ++i)
            shuffle(std::begin(arrayID), std::end(arrayID), rng);
    }

    //Calc finger size and sim time
    FINGERS_SIZE         = (uint)ceil(log10(MAX_UINT160.to_uint64()+1) / log10(2));
    DELAY_TURN_ON        = sc_time(FINGERS_SIZE * 1 + 40, SC_SEC);
    SIM_TIME             = NODES * DELAY_TURN_ON + ADD_SIM_TIME;
    TRP_PERIOD_SNAPSHOTS = SNAPSHOT_PERIOD_SEC;

    //LOG    
    cout << arrayID.size() << " nodes = { ";
    for (auto it = arrayID.begin(); (it != arrayID.end()) && (distance(arrayID.begin(), it) < NODES); ++it) {
        cout << *it << " ";
    }
    cout << "}" << endl;    
    //LOG

            
    uint fingersSize  = FINGERS_SIZE;
    vector<application*>    applications; applications.resize(NODES, nullptr);
    vector<transport_plus*> transports;   transports.resize(NODES, nullptr);
    network*        network1 = nullptr;    
    monitor*        monitor1 = nullptr;
    string str;


    //Set simulating scenarios of message issuing for Application layer
    sim_message mess;
    vector<network_address> addrs;


    
    mess.type = SIM_HARD_RESET;
    mess.amount = 1;
    mess.firstDelay = sc_time(0, SC_MS);

    bool testNonContinuosNumbers = false;

    network1 = new network("netw", NODES);
    monitor1 = new monitor("monitor", NODES, fingersSize, SHOW_MOTIVE, SHOW_CCW_FINGERS, MONITOR_MODE);


    for (uint i = 0; i < NODES; ++i) {
        //ID for node
        chord_conf_parameters params;
        params.netwAddr.set(arrayID.at(i), string("192.168.0.") + to_string(i), 1111, 2222);
        params.setDefaultTimersCountersFingersSize();
        params.fingersSize = fingersSize;

        str = string("app") + params.netwAddr.id.to_string(SC_DEC);
        applications[i] = new application(str.c_str());
        str = string("trp") + params.netwAddr.id.to_string(SC_DEC);
        transports[i] = new transport_plus(str.c_str());

        applications[i]->trp_port.bind( *(transports[i]) );
        transports[i]->network_port.bind(*network1);
        transports[i]->monitor_port.bind(*monitor1);
        network1->trp_ports[i]->bind( *(transports[i]) );
        monitor1->trp_ports[i]->bind( *(transports[i]) );        

        applications[i]->setLogMode(ALL_LOG);
        applications[i]->setEnabledLog();        
        str = "./log/app" + string(".txt");
        applications[i]->setPathLog(str.c_str());
        applications[i]->msgLog(applications[i]->name(), LOG_TXRX, LOG_INFO, "create", ALL_LOG);


        transports[i]->setLogMode(LOG_DISABLED);
        transports[i]->setEnabledLog();        
        str = "./log/trp" + params.netwAddr.id.to_string(SC_DEC) + string(".txt");
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
                
        addrs.push_back(params.netwAddr);
        
        
        if (i >= 1) {            
            uint loadNode = i/10*10;
            if (i == loadNode)
                loadNode = (i-1)/10*10;
            params.seed.push_back(addrs.at(loadNode));
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
    network1->setLogMode(LOG_DISABLED);
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
    monitor1->setSimTime(SIM_TIME);



    // current date/time based on current system
    time_t now = time(0);

    // convert now to string form
#pragma warning(suppress : 4996)
    char* dt = ctime(&now);


    
    //Run simulation    
    cout << endl << "Run simulation " << SIM_TIME.to_string() << endl;
    cout << "" << dt << endl;

    


    time_progress timeProgress(SIM_TIME, SIM_TIME/10);
    sc_start(SIM_TIME);
    cout << endl << "Write results" << endl;


    //Free memory layer-by-layer
    char c = 'y';
    cout << endl << "*** Prepare to free memory used by applications. Enter 'y': "; //cin >> c;
    cout << endl;
    for (uint i = 0; i < applications.size(); ++i) {
        delete applications[i];
        applications[i] = nullptr;
    }
    
    cout << "*** Prepare to free memory used by low_latency_chord. Enter 'y': "; //cin >> c;
    cout << endl;
    for (uint i = 0; i < transports.size(); ++i) {
        delete transports[i];
        transports[i] = nullptr;
    }
    
    cout << "*** Prepare to free memory used by network. Enter 'y': "; //cin >> c;
    cout << endl;
    delete network1;
    network1 = nullptr;

    delete monitor1;
    monitor1 = nullptr;

    cout << "Test " << "finished ___________________________________" << endl;

    // current date/time based on current system
    // convert now to string form
    now = time(0);  
#pragma warning(suppress : 4996)
    dt = ctime(&now);
    cout << "" << dt << endl;

    getchar();    
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


    ////Set valid reference clockwise fingers for every node
    //vector<node_address> fingers;
    //uint index;
    //fingers.resize(DEFAULT_FINGERS_SIZE, node_address());

    //index = 0;
    //fingers[index].id = 1; ++index;
    //fingers[index].id = 2; ++index;
    //monitor1->setReferenceCwFingers(0, fingers);

    //index = 0;
    //fingers[index].id = 2; ++index;
    //fingers[index].id = 3; ++index;
    //monitor1->setReferenceCwFingers(1, fingers);

    //index = 0;
    //fingers[index].id = 3; ++index;
    //fingers[index].id = 3; ++index;
    //monitor1->setReferenceCwFingers(2, fingers);

    //index = 0;
    //fingers[index].id = 0; ++index;
    //fingers[index].id = 2; ++index;
    //monitor1->setReferenceCwFingers(3, fingers);


    ////Set valid reference counter clockwise fingers for every node
    //index = 0;
    //fingers[index].id = 3; ++index;
    //fingers[index].id = 3; ++index;
    //monitor1->setReferenceCcwFingers(0, fingers);

    //index = 0;
    //fingers[index].id = 0; ++index;
    //fingers[index].id = 3; ++index;
    //monitor1->setReferenceCcwFingers(1, fingers);

    //index = 0;
    //fingers[index].id = 1; ++index;
    //fingers[index].id = 0; ++index;
    //monitor1->setReferenceCcwFingers(2, fingers);

    //index = 0;
    //fingers[index].id = 2; ++index;
    //fingers[index].id = 1; ++index;
    //monitor1->setReferenceCcwFingers(3, fingers);