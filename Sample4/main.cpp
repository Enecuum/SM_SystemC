
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING  // It is necessary to include experimental filesystem functin check directory
#include <experimental/filesystem>                            //

#include <set>
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


using namespace P2P_MODEL;
using namespace std;

using json = nlohmann::json;

//Pre-defined parameters
      bool         SHOW_MOTIVE                       = false;
const monitor_mode MONITOR_MODE                      = AUTO_VERIFY;
const bool         SHOW_CCW_FINGERS                  = false;
const int          JSON_FAILED                       = -1;
const bool         DEFAULT_GEN_RANDOM                = false;
const bool         DEFAULT_LOG_CHORD_DEBUG           = false;
const uint         DEFAULT_NODES                     = 5;
const sc_time      DEFAULT_PERIOD_FINGERS_UPDATE_SEC = sc_time(1, SC_SEC);
const sc_time      DEFAULT_PERIOD_NODE_RUN_SEC       = sc_time(60, SC_SEC);
const sc_time      DEFAULT_TIME_ADD_SIM_SEC          = sc_time(180, SC_SEC);
const bool         DEFAULT_SHUFFLE                   = false;
const sc_time      DEFAULT_PERIOD_SNAPSHOT_SEC       = sc_time(1, SC_SEC);




//Definition for extern variables declared into  inc.h
uint160  P2P_MODEL::MAX_UINT160          = uint160(-1);
uint     P2P_MODEL::MAX_SMALL_UINT       = P2P_MODEL::MAX_UINT160.to_uint();
bool     P2P_MODEL::RANDOM_IDS           = false;
bool     P2P_MODEL::CCW_FINGERS_TURN_ON  = false; 
bool     P2P_MODEL::MAKE_SNAPSHOT_ALWAYS = false;
sc_time  P2P_MODEL::TRP_PERIOD_SNAPSHOTS = sc_time(1, SC_SEC);








//Structure for JSON parser
namespace ns {
    struct parser_parameters {
        vector<uint> arrayID;
        bool         gen_random;
        bool         log_chord_debug;
        uint         nodes;
        double       period_fingers_update_sec;
        double       period_node_run_sec;
        double       period_snapshot_sec;
        double       time_add_sim_sec;
        double       wait_rx_ack_sec;
        double       wait_rx_successor_on_join_sec;
        double       wait_rx_successor_sec;
        double       wait_rx_predecessor_sec;
        bool         shuffle;
       

        void default() {
            arrayID                        = {12, 4, 2, 10, 1, 14, 5, 3, 11, 7};
            gen_random                     = DEFAULT_GEN_RANDOM;
            log_chord_debug                = DEFAULT_LOG_CHORD_DEBUG;
            nodes                          = static_cast<uint>( arrayID.size() );                               //DEFAULT_NODES;
            period_fingers_update_sec      = DEFAULT_PERIOD_FINGERS_UPDATE_SEC.to_double();
            period_node_run_sec            = DEFAULT_PERIOD_NODE_RUN_SEC.to_double();
            period_snapshot_sec            = DEFAULT_PERIOD_SNAPSHOT_SEC.to_double();
            time_add_sim_sec               = DEFAULT_TIME_ADD_SIM_SEC.to_double();
            wait_rx_ack_sec                = DEFAULT_TIMEOUT_RX_ACK.to_double();
            wait_rx_successor_on_join_sec  = DEFAULT_TIMEOUT_RX_SUCCESSOR_ON_JOIN.to_double();
            wait_rx_successor_sec          = DEFAULT_TIMEOUT_RX_SUCCESSOR.to_double();
            wait_rx_predecessor_sec        = DEFAULT_TIMEOUT_RX_PREDECESSOR.to_double();
            shuffle                        = DEFAULT_SHUFFLE;            
        }
    };

    void to_json(json& j, const parser_parameters& p) {
        j = json{{"arrayID",                        p.arrayID},
                 {"gen_random",                     p.gen_random},
                 {"log_chord_debug",                p.log_chord_debug},
                 {"nodes",                          p.nodes},
                 {"period_fingers_update_sec",      p.period_fingers_update_sec},
                 {"period_node_run_sec",            p.period_node_run_sec},
                 {"snapshot_period_sec",            p.period_snapshot_sec},
                 {"time_add_sim_sec",               p.time_add_sim_sec},
                 {"wait_rx_ack_sec",                p.wait_rx_ack_sec},
                 {"wait_rx_predecessor_sec",        p.wait_rx_predecessor_sec},
                 {"wait_rx_successor_on_join_sec",  p.wait_rx_successor_on_join_sec},
                 {"wait_rx_successor_sec",          p.wait_rx_successor_sec},                 
                 {"shuffle",                        p.shuffle}                 
                };
    }

    void from_json(const json& j, parser_parameters& p) {
        j.at("arrayID")                       .get_to(p.arrayID);
        j.at("gen_random")                    .get_to(p.gen_random);        
        j.at("log_chord_debug")               .get_to(p.log_chord_debug);
        j.at("nodes")                         .get_to(p.nodes);                    
        j.at("period_fingers_update_sec")     .get_to(p.period_fingers_update_sec);
        j.at("period_node_run_sec")           .get_to(p.period_node_run_sec);
        j.at("period_snapshot_sec")           .get_to(p.period_snapshot_sec);
        j.at("time_add_sim_sec")              .get_to(p.time_add_sim_sec);
        j.at("wait_rx_ack_sec")               .get_to(p.wait_rx_ack_sec);
        j.at("wait_rx_successor_on_join_sec") .get_to(p.wait_rx_successor_on_join_sec);
        j.at("wait_rx_successor_sec")         .get_to(p.wait_rx_successor_sec);
        j.at("wait_rx_predecessor_sec")       .get_to(p.wait_rx_predecessor_sec);        
        j.at("shuffle")                       .get_to(p.shuffle);        

        set<uint> s(p.arrayID.begin(), p.arrayID.end()); //here we remove duplicates
        p.arrayID.assign(s.begin(), s.end());            //I will be back )

        if (false == p.gen_random)
            p.nodes = (uint)(p.arrayID.size());
    }
}


bool jsonParser(const string path, ns::parser_parameters& p) {    
    ifstream file(path.c_str());
    json j;
    try {
        file >> j; 
        
        p = j.get<ns::parser_parameters>();        
        cout << endl << endl;

        
        stringstream bufH;
        bufH << "./log";
        if (!std::experimental::filesystem::exists(bufH.str()))
        {
            std::experimental::filesystem::create_directories(bufH.str());
        }

        return true;
    }
    catch (json::parse_error& e) {
        cout << setw(4) << j << endl << endl;
        cout << "message: " << e.what() << '\n'
             << "exception id: " << e.id << '\n'
             << "byte position of error: " << e.byte << endl;
    }
    catch (json::invalid_iterator& e)
    {
        cout << setw(4) << j << endl << endl;
        cout << "message: " << e.what() << '\n'
             << "exception id: " << e.id << endl;
    }
    catch (json::type_error& e)
    {
        cout << setw(4) << j << endl << endl;
        cout << "message: " << e.what() << '\n'
             << "exception id: " << e.id << std::endl;
    }
    catch (json::out_of_range& e)
    {
        cout << setw(4) << j << endl << endl;
        cout << "message: " << e.what() << '\n'
             << "exception id: " << e.id << endl;
    }
    catch (json::exception& e)
    {
        cout << setw(4) << j << endl << endl;
        cout << e.what() << '\n';
    }
    catch (...) {
        cout << setw(4) << j << endl << endl;
    }

    p.default();
    cout << endl << "File is invalid. Check lines in JSON file !!!" << endl;
    return false;
}

string pathGoodBrackets(string src) {
    string newStr = src;
    replace(newStr.begin(), newStr.end(), '/', '\\');
    return newStr;
}



int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");
    _setmaxstdio(2048);

    cout << boolalpha;
    cout << "   CHORD " << P2P_MODEL::W << " bit model " << "v20220131" << endl;
    cout << "_______________________________________" << endl << endl;

    vector<string> argvStr;
    for (int i = 0; i < argc; ++i) {
        argvStr.push_back(argv[i]);
        auto itBegin = argvStr.at(i).begin();
        auto itEnd = argvStr.at(i).end();
        auto itRemoveSymbol = remove(itBegin, itEnd, ',');
        argvStr[i].erase(itRemoveSymbol, itEnd);
    }

        
    string configPath = "/config.txt";
    string snapshotPath = "/log/snapshot.txt";
    string currPath = argvStr[0].substr(0, argvStr[0].find_last_of("\\"));    
    

    cout << "Read config file: " << (currPath + pathGoodBrackets(configPath)) << endl;


    ns::parser_parameters p;
    if (jsonParser(string(".")+configPath, p) == false) {
        cout << "Please, input 'y' to exit " << endl; getchar();
        return JSON_FAILED;
        //cout << "Pre-difined parameters will be used" << endl;      
    }
    
    //Gen random IDs for nodes or do shuffling
    auto rng = std::default_random_engine{(unsigned int)time(0)};
    if (p.gen_random) {
        vector<uint> possibleNodes;
        
        uint168 maxNodesNumbers = MAX_UINT160;
        maxNodesNumbers++;

        if (p.nodes > maxNodesNumbers)
            p.nodes = maxNodesNumbers.to_uint();

        possibleNodes.reserve(p.nodes);
        for (uint i = 0; i <= MAX_UINT160; ++i) {
            possibleNodes.push_back(i);
        }
        
        for (int i = 0; i < 5; ++i) {
            shuffle(std::begin(possibleNodes), std::end(possibleNodes), rng);
        }

        //auto it = max_element(possibleNodes.begin(), possibleNodes.end());
        //size_t i1 = rand() % 4;
        //swap(possibleNodes.begin()+i1, it);
        //cout << "i1 = " <<  i1 << endl;
        
        p.arrayID.assign(possibleNodes.begin(), possibleNodes.begin()+p.nodes);
    }
    else if (p.shuffle) {
        for (int i = 0; i < 5; ++i) {
            shuffle(std::begin(p.arrayID), std::end(p.arrayID), rng);
        }
    }

    
    


    
    //Calc finger size and sim time
    uint    FINGER_SIZE  = (uint) ceil(log10(MAX_UINT160.to_uint64()+1) / log10(2));
    sc_time SIM_TIME     = sc_time((p.nodes+1)*p.period_node_run_sec + p.time_add_sim_sec, SC_SEC);
    TRP_PERIOD_SNAPSHOTS = sc_time(p.period_snapshot_sec, SC_SEC);
    SHOW_MOTIVE = (p.log_chord_debug ? true : false);





    //LOG  
    stringstream arrayIDss;
    arrayIDss << p.arrayID.size() << " nodes = { ";
    for (auto it = p.arrayID.begin(); (it != p.arrayID.end()) && (distance(p.arrayID.begin(), it) < p.nodes); ++it) {
        arrayIDss << *it << " ";
    }
    arrayIDss << "}" << endl;
    cout << arrayIDss.str();
    //LOG


    vector<application*>    applications; applications.resize(p.nodes, nullptr);
    vector<transport_plus*> transports;   transports.resize(p.nodes, nullptr);
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

    network1 = new network("netw", p.nodes);
    monitor1 = new monitor("monitor", p.nodes, FINGER_SIZE, SHOW_MOTIVE, SHOW_CCW_FINGERS, MONITOR_MODE);


    for (uint i = 0; i < p.nodes; ++i) {
        //ID for node
        chord_conf_parameters params;
        params.netwAddr.set(p.arrayID.at(i), string("192.168.0.") + to_string(i), 1111, 2222);
        params.setDefaultTimersCountersFingersSize();
        params.setTimers(sc_time(p.wait_rx_ack_sec, SC_SEC),
                         sc_time(p.wait_rx_successor_on_join_sec, SC_SEC),
                         sc_time(p.wait_rx_successor_sec, SC_SEC),
                         sc_time(p.wait_rx_predecessor_sec, SC_SEC),
                         sc_time(p.period_fingers_update_sec, SC_SEC));

        params.fingersSize = FINGER_SIZE;

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

        transports[i]->setLogMode(DISABLED_LOG);
        if (p.log_chord_debug)
            transports[i]->setLogMode(ALL_LOG);

        transports[i]->setEnabledLog();        
        str = "./log/trp" + params.netwAddr.id.to_string(SC_DEC) + string(".txt");
        transports[i]->setPathLog(str);
        str = string(".") + snapshotPath;
        transports[i]->setSnapshotPathLog(str);
        transports[i]->msgLog(transports[i]->name(), LOG_TXRX, LOG_INFO, "create", ALL_LOG);
        

        if (testNonContinuosNumbers == true) {
            switch (i) {
            case 0:
                mess.firstDelay = sc_time(0, SC_SEC);
                break;

            default:
                mess.firstDelay = (SIM_TIME-sc_time(p.time_add_sim_sec, SC_SEC))-i*sc_time(p.period_node_run_sec, SC_SEC);
                break;
            }
            applications[i]->pushSimulatingMess(mess);
        }
        else {
            applications[i]->pushSimulatingMess(mess);
            mess.firstDelay += sc_time(p.period_node_run_sec, SC_SEC);                   //sc_time(10, SC_SEC);
        }
                
        addrs.push_back(params.netwAddr);
        
        
        if (i >= 1) {            
            //uint loadNode = i/10*10;
            //if (i == loadNode)
            //    loadNode = (i-1)/10*10;
            //params.seed.push_back(addrs.at(loadNode));
            uint loadNode = 0;
            params.seed.push_back(addrs.at(loadNode));
        }
        transports[i]->setConfParameters(params);
       
        monitor1->setSnapshotUnderTest(transports[i]->snapshot_pointers());
    }




    network1->setEnabledLog();
    network1->setLogMode(DISABLED_LOG);
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
    monitor1->msgLog(monitor1->name(), LOG_TXRX, LOG_INFO, arrayIDss.str(), ALL_LOG);
    monitor1->setSimTime(SIM_TIME);



    // current date/time based on current system
    time_t now = time(0);
#pragma warning(suppress : 4996)
    char* dt = ctime(&now);           // convert now to string form


    
    //Run simulation    
    cout << endl << "Run simulation " << SIM_TIME.to_string() << "ec" << endl;
    cout << "" << dt << endl;

    
    //Model will start
    time_progress timeProgress(SIM_TIME, SIM_TIME/10);
    sc_start(SIM_TIME);


    
    cout << endl << "Write results: " << (currPath + pathGoodBrackets(snapshotPath)) << endl;


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

        
    now = time(0);                          // current date/time based on current system
#pragma warning(suppress : 4996)
    dt = ctime(&now);                       // convert now to string form
    cout << "" << dt << endl;

    getchar();    
    cout << "*** Enter 'y' to exit: "; //cin >> c;
    return 0;
}



//EXAMPLES OTHER MESSAGES FOR APPLICATION
// 
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

