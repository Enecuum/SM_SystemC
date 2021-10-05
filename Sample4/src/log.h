#ifndef __LOG_H__
#define __LOG_H__

#include "inc.h"

#define LOG_TX  "Tx  "
#define LOG_RX  "  Rx"
#define LOG_TXRX   "TxRx"
#define LOG_IN    "< < <"
#define LOG_OUT    "> > >"
#define LOG_INFO   ":::::"
#define LOG_WARNING_INDICATOR  "!!!!!"
#define LOG_ERROR_INDICATOR   LOG_WARNING_INDICATOR
#define LOG_SPACER  "\t"

namespace P2P_MODEL
{
    const int MAX_LEN_METHOD_NAME = 30;
    const int MAX_LEN_TIME = 20;

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

    protected:
        string m_pathLog;
        log_mode m_logMode;
        string m_strLogText;

    public:
        
        log();
        ~log();

        void setMaxLengthMethodAndTimeLog(const int methodNameLen, const int timeLen);
        void setPathLog(const string& pathLog);
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

        const string& logText(const char logRxTx[]
                                , const char logInOut[]
                                , const string& text);
    };
}

#endif