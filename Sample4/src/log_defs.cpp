#include "log_defs.h"

namespace P2P_MODEL
{

    MsgLog::MsgLog()
    {
        m_maxLenMethodName = MAX_LEN_METHOD_NAME;
        m_maxLenTime = MAX_LEN_TIME;
        m_isEnabled = false;
        m_logMode = DEFAULT_LOG_MODE;
    }

    MsgLog::~MsgLog() {   }


    void MsgLog::setMaxLengthMethodAndTimeLog(const int methodNameLen, const int timeLen) {
        m_maxLenMethodName = methodNameLen;
        m_maxLenTime = timeLen;
    }


    void MsgLog::setPathLog(const string& pathLog)
    {
        m_pathLog = pathLog;
    }


    void MsgLog::setLogMode(const log_mode& logMode)
    {
        m_logMode = logMode;
    }


    void MsgLog::setDisabledLog()
    {
        m_isEnabled = false;
    }


    void MsgLog::setEnabledLog()
    {
        m_isEnabled = true;
    }


    bool MsgLog::isEnabled()
    {
        return m_isEnabled;
    }


    void MsgLog::clearLog() {
        ofstream file;
        file.open(m_pathLog.c_str(), std::ofstream::out);
        file.close();
    }


    void MsgLog::msgLog(const string& methodName, const string& text, const int &secondaryLogMode)
    {
        messageLog(m_pathLog, methodName, m_logMode, static_cast<log_mode>(secondaryLogMode), string(""), string(""), text);
    }

    void MsgLog::msgLog(const string& methodName, const string& logRxTx, const string& logInOut, const string& text, const int &secondaryLogMode)
    {
        messageLog(m_pathLog, methodName, m_logMode, static_cast<log_mode>(secondaryLogMode), logRxTx, logInOut, text);
    }

    void MsgLog::msgLog(const string& methodName, const string& logRxTx, const string& logInOut, const string& text)   
    {
        messageLog(m_pathLog, methodName, m_logMode, INTERNAL_LOG, logRxTx, logInOut, text);
    }


    void MsgLog::messageLog(const string& filePath, const string& methodName, const log_mode &primary,
                            const log_mode &secondary, const string& logRxTx,
                            const string& logInOut,
                            const string& text)
    {
        if (m_isEnabled) {
            if (primary & secondary) {
                std::stringstream ss;
                ss << std::setw(m_maxLenTime) << std::setiosflags(std::ios::left) << (sc_time_stamp().to_string());
                ss << LOG_SPACER;
                ss << std::setw(m_maxLenMethodName) << std::setiosflags(std::ios::left) << methodName;
                ss << LOG_SPACER;
                ss << logRxTx << LOG_SPACER;
                ss << logInOut << LOG_SPACER;

                static std::map<std::string, std::ofstream*> mapFile;
                ofstream *file;
                bool isOverWrite = false;
                auto it = mapFile.find(filePath.data());
                if (it == mapFile.end()) {
                    file = new ofstream(filePath.data(), std::ofstream::out);
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
                    *file << ss.str() << text << std::endl;
                }
            }
        }
    }

   

    const string &MsgLog::logText(const char logRxTx[], const char logInOut[], const string &text)
    {
        static string tmpCharLog = "";
        tmpCharLog.clear();
        tmpCharLog += logRxTx;
        tmpCharLog += LOG_SPACER;
        tmpCharLog += logInOut;
        tmpCharLog += LOG_SPACER;
        tmpCharLog += text;
        return tmpCharLog;
    }

}