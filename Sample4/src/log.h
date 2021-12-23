#ifndef __LOG_H__
#define __LOG_H__

#include "inc.h"


namespace P2P_MODEL
{
    const string LOG_SPACE                = string(" ");
    const string LOG_TAB                  = string("\t");
    const string LOG_TX                   = string("Tx  ");
    const string LOG_RX                   = string("  Rx");
    const string LOG_TXRX                 = string("TxRx");
    const string LOG_IN                   = string("< < <");
    const string LOG_OUT                  = string("> > >");
    const string LOG_INFO                 = string("     "); /*string(":::::");*/
    const string LOG_WARNING              = string("!!!!!");
    const string LOG_ERROR                = string("ERROR");
    const string LOG_ERROR_OVERFLOW       = string("OVERFLOW BUFFER");
    const string LOG_ERROR_NO_ADDR        = string("NO ADDR");
    const string LOG_ERROR_NULLPTR        = string("NULLPTR");
    const string LOG_ERROR_NOT_RECOGNIZED = string("UNKNOWN, NOT RECOGNIZED");    
    const string LOG_ERROR_INVALID_RANGE  = string("OUT OF RANGE");
    const string LOG_ERROR_SIZE_MISMATCHED = string("OUT OF RANGE");
    const string LOG_ERROR_INVALID_ADDR    = string("DROP MESS, UNKNOWN ADDR");
    const string LOG_ERROR_INVALID_MESS_ID = string("DROP MESS, UNKNOWN MESS ID");    
    const string LOG_ERROR_INVALID_SEARCHED_ID = string("DROP MESS, INVALID SEARCHED ID");


    #define LOG_MESS(m)  string("m");
    #define LOG_BOOL(m)   (m == true ? string("yes") : string("no"))
    #define LOG_DEC_BOOL(m)   (m == 1 ? string("yes") : string("no"))

    const int MAX_LEN_METHOD_NAME = 30/2;
    const int MAX_LEN_TIME = 20/2;

    enum log_mode {
        LOG_DISABLED = 0,
        ERROR_LOG = 1,
        INTERNAL_LOG = 2,
        EXTERNAL_LOG = 4,
        DEBUG_LOG = 8,
        STATISTICS_LOG = 16,
        ALL_LOG = ERROR_LOG + INTERNAL_LOG + EXTERNAL_LOG + DEBUG_LOG + STATISTICS_LOG,
        DEFAULT_LOG_MODE = EXTERNAL_LOG
    };

    class log
    {
    private:
        bool m_isEnabled;
        int m_maxLenMethodName;
        int m_maxLenTime;
        static std::map<std::string, std::ofstream*> mapFile;
        


    protected:
        string m_logPath;
        string m_logSnapshotPath;
        log_mode m_logMode;
        string m_logText;
        stringstream m_ssLog;

    public:
        
        log();
        ~log();
        
        void setMaxLengthMethodAndTimeLog(const int methodNameLen, const int timeLen);
        void setPathLog(const string& pathLog);
        void setSnapshotPathLog(const string& pathLog);
        void setLogMode(const log_mode& logMode);
        void setDisabledLog();
        void setEnabledLog();

        bool isEnabled();

        void clearLog();

        void msgLog(const string& methodName, const string& text, const int &secondaryLogMode);
        void msgLog(const string& methodName, const string& logRxTx, const string& logInOut, const string& text, const int &secondaryLogMode);
        void msgLog(const string& methodName, const string& logRxTx, const string& logInOut, const string& text);      //uses a router

        void messageLog(const string& filePath, const string& methodName, const log_mode &primary,
                        const log_mode &secondary, const string& logRxTx, const string& logInOut,
                        const string& text = "");

        string logText(const char logRxTx[]
                                , const char logInOut[]
                                , const string& text);
       
       void snapshotLog(const uint activeNodes, const node_address_latency& nodeID, const vector<node_address_latency>& cwFingers, const vector<node_address_latency>& ccwFingers, const sc_time& timestamp);
       void snapshotLogJSON(const json& j);
       
    };
}

#endif