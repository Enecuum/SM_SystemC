#include "log.h"

namespace P2P_MODEL
{

    log::log() {
        m_maxLenMethodName = MAX_LEN_METHOD_NAME;
        m_maxLenTime = MAX_LEN_TIME;
        m_isEnabled = false;
        m_logMode = DEFAULT_LOG_MODE;
    }

    log::~log() {   }


    void log::setMaxLengthMethodAndTimeLog(const int methodNameLen, const int timeLen) {
        m_maxLenMethodName = methodNameLen;
        int m_maxLenTime = timeLen;        
    }


    void log::setPathLog(const string& pathLog)
    {
        m_logPath = pathLog;
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
        char c;
        if (m_isEnabled) {
            std::stringstream ss;
            uint isMatched = primary & secondary;
            if (isMatched > 0) {
                
                ss << std::setw(m_maxLenTime) << std::setiosflags(std::ios::left) << (sc_time_stamp().to_seconds()) /* << " " << (sc_time_stamp().to_seconds() * 1000)*/;
                ss << LOG_TAB;
                ss << std::setw(m_maxLenMethodName) << std::setiosflags(std::ios::left) << methodName;
                ss << LOG_TAB;
                ss << logRxTx << LOG_TAB;
                ss << logInOut << LOG_TAB;

                static std::map<std::string, std::ofstream*> mapFile;
                ofstream* file;
                
                bool isOverWrite = false;
                auto it = mapFile.find(filePath.data());
                if (it == mapFile.end()) {
                    file = new ofstream(filePath.c_str(), std::ofstream::out);                                        
                    
                    if (file->bad()) {
                        cout << "log file `" << filePath.data() << "` not openned. Press text 'y' to abort simulating...";
                        cin >> c; getchar();
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
                        cout << "log file `" << filePath.data() << "` not re-openned. Press text 'y' to abort simulating...";
                        cin >> c; getchar();
                        exit(-1);
                    }
                    *file << ss.str() << text << std::endl;
                }
            }

            if (logInOut == LOG_ERROR) {                
                cout << ss.str() << text << std::endl;
                cout << "Press text 'y' to abort simulating...";
                cin >> c; getchar();
                exit(-1);
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
