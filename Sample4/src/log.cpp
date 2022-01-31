#include "log.h"

namespace P2P_MODEL
{
    std::map<std::string, std::ofstream*> log::mapFile;

    log::log() {
        m_maxLenMethodName = MAX_LEN_METHOD_NAME;
        m_maxLenTime = MAX_LEN_TIME;
        m_isEnabled = false;
        m_logMode = DEFAULT_LOG_MODE;
    }

    log::~log() {
        if (mapFile.size() > 0) {
            for (auto it = mapFile.begin(); it != mapFile.end(); ++it) {
                if ((it->first.length() > 0) && (it->second != nullptr)) {
                    delete it->second;
                    it->second = nullptr;
                }
            }
            mapFile.clear();
        }
    }


    void log::setMaxLengthMethodAndTimeLog(const int methodNameLen, const int timeLen) {
        m_maxLenMethodName = methodNameLen;
        int m_maxLenTime = timeLen;        
    }


    void log::setPathLog(const string& pathLog)
    {
        m_logPath = pathLog;
        m_logSnapshotPath = pathLog;
    }


    void log::setSnapshotPathLog(const string& pathLog) {
        m_logSnapshotPath = pathLog;
    }


    void log::setLogMode(const log_mode& logMode)
    {
        m_logMode = logMode;
    }


    void log::setDisabledLog()
    {
        m_isEnabled = false;
    }


    void log::setEnabledLog()
    {
        m_isEnabled = true;
    }


    bool log::isEnabled()
    {
        return m_isEnabled;
    }


    void log::clearLog() {
        ofstream file;
        file.open(m_logPath.c_str(), std::ofstream::out);
        file.close();
    }


    void log::msgLog(const string& methodName, const string& text, const int &secondaryLogMode)
    {
        messageLog(m_logPath, methodName, m_logMode, static_cast<log_mode>(secondaryLogMode), string(""), string(""), text);
    }

    void log::msgLog(const string& methodName, const string& logRxTx, const string& logInOut, const string& text, const int &secondaryLogMode)
    {
        messageLog(m_logPath, methodName, m_logMode, static_cast<log_mode>(secondaryLogMode), logRxTx, logInOut, text);
    }

    void log::msgLog(const string& methodName, const string& logRxTx, const string& logInOut, const string& text)   
    {
        messageLog(m_logPath, methodName, m_logMode, INTERNAL_LOG, logRxTx, logInOut, text);
    }


    void log::messageLog(const string& filePath, const string& methodName, const log_mode &primary,
                            const log_mode &secondary, const string& logRxTx,
                            const string& logInOut,
                            const string& text)
    {                
        if (m_isEnabled) {
            m_ssLog = stringstream(); //clear variable to default value

            char c;
            std::stringstream ss;
            uint isMatched = primary & secondary;
            if (isMatched > 0) {
                
                ss << std::setw(m_maxLenTime) << std::setiosflags(std::ios::left) << (sc_time_stamp().to_seconds()) /* << " " << (sc_time_stamp().to_seconds() * 1000)*/;
                ss << LOG_TAB;
                ss << std::setw(m_maxLenMethodName) << std::setiosflags(std::ios::left) << methodName;
                ss << LOG_TAB;
                ss << logRxTx << LOG_TAB;
                ss << logInOut << LOG_TAB;

                
                ofstream* file;                
                bool isOverWrite = false;
                auto it = mapFile.find(filePath.data());
                if (it == mapFile.end()) {
                    file = new ofstream(filePath.c_str(), std::ofstream::out);                                        
                    
                    if (file->bad()) {
                        cout << "log file `" << filePath.data() << "` not openned. Enter 'y' to abort simulating...";
                        cin >> c; //getchar();
                        exit(-1);
                    }

                    mapFile[filePath.data()] = file;
                    isOverWrite = true;                    
                }
                else {
                    file = it->second;
                }

                if(!isOverWrite) {
                    *file << ss.str() << text << std::endl;
                }

                if (isOverWrite) {
                    file->close();
                    file->open(filePath.data(), std::ofstream::app);
                    if (file->bad()) {
                        cout << "log file `" << filePath.data() << "` not re-openned. Enter 'y' to abort simulating...";
                        cin >> c; //getchar();
                        exit(-1);
                    }
                    *file << ss.str() << text << std::endl;
                }
            }

            if (logInOut == LOG_ERROR) {                
                cout << ss.str() << text << std::endl;
                cout << "Press 'y' to abort simulating...";
                cin >> c; //getchar();
                exit(-1);
            }
        }
    }


    void log::snapshotLog(const uint activeNodes, const node_address_latency& nodeAddr, const vector<node_address_latency>& cwFingers, const vector<node_address_latency>& ccwFingers, const sc_time& timestamp) {
        static sc_time lastCallTime = SC_ZERO_TIME;
        static bool isSnapshotHead = true;
        string filePath = m_logSnapshotPath;
        std::stringstream ss;

        if ((m_isEnabled) && (filePath.size() > 0) /* && (sc_time_stamp() > SC_ZERO_TIME)*/) {
            if (lastCallTime != sc_time_stamp()) {
                isSnapshotHead = true;                
            }

            //Get pointer on file               
            ofstream* file;
            bool isOverWrite = false;
            auto it = mapFile.find(filePath.data());
            if (it == mapFile.end()) {
                file = new ofstream(filePath.c_str(), std::ofstream::out);
                mapFile[filePath.data()] = file;
                isOverWrite = true;
            }
            else {
                file = it->second;
            }


            ss << endl;
            if (isSnapshotHead == true) {
                isSnapshotHead = false;
                lastCallTime = sc_time_stamp();

                //Head title for printing
                ss << "*****************************" << endl;
                ss << "curr time: " << timestamp.to_string() << endl << endl;
            }


            //Text for printing                
            ss << "node: " << nodeAddr.toNodeAddress() << endl;
            if (cwFingers.size() > 0)
                ss << "succ: " << cwFingers.at(0).toStrIDonly() << endl;
            if (ccwFingers.size() > 0)
                ss << "pred: " << ccwFingers.at(0).toStrIDonly() << endl;

            for (uint i = 0; i < cwFingers.size(); ++i) {
                ss << "cw finger[" << to_string(i) << "] : " << cwFingers.at(i).toStrFinger()/*toStrIDonly()*/ << endl;
            }
            
            for (uint i = 0; i < ccwFingers.size(); ++i) {
                ss << "ccw finger[" << to_string(i) << "] : " << ccwFingers.at(i).toStrFinger()/*toStrIDonly()*/ << endl;
            }
            


            if (!isOverWrite) {
                *file << ss.str() << std::endl;
            }

            if (isOverWrite) {
                file->close();
                file->open(filePath.data(), std::ofstream::app);
                *file << ss.str() << std::endl;
            }
        }
    }


    void log::snapshotLogJSON(const json& j) {
        static sc_time lastCallTime = SC_ZERO_TIME;
        static bool isSnapshotHead = true;
        string filePath = m_logSnapshotPath;
        std::stringstream ss;

        if ((m_isEnabled) && (filePath.size() > 0) /* && (sc_time_stamp() > SC_ZERO_TIME)*/) {
            if (lastCallTime != sc_time_stamp()) {
                isSnapshotHead = true;
            }

            //Get pointer on file               
            ofstream* file;
            bool isOverWrite = false;
            auto it = mapFile.find(filePath.data());
            if (it == mapFile.end()) {
                file = new ofstream(filePath.c_str(), std::ofstream::out);
                mapFile[filePath.data()] = file;
                isOverWrite = true;
            }
            else {
                file = it->second;
            }

            
            if (isSnapshotHead == true) {
                ss << "*****************************" << endl;
                ss << setw(4) << j;

                isSnapshotHead = false;
                lastCallTime = sc_time_stamp();
            }


            if (!isOverWrite) {
                *file << ss.str() << std::endl;
            }

            if (isOverWrite) {
                file->close();
                file->open(filePath.data(), std::ofstream::app);
                *file << ss.str() << std::endl;
            }
        }
    }
   

    string log::logText(const char logRxTx[], const char logInOut[], const string &text)
    {
        string tmpCharLog = "";
        tmpCharLog.clear();
        tmpCharLog += logRxTx;
        tmpCharLog += LOG_TAB;
        tmpCharLog += logInOut;
        tmpCharLog += LOG_TAB;
        tmpCharLog += text;
        return tmpCharLog;
    }

}
