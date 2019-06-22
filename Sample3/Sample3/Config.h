#pragma once

#define cnf_LoggingLevel "logging_level"

#define cnf_n "n"
#define cnf_k "k"
#define cnf_r "r"

#define cnf_voteTime "voteTime"
#define cnf_rqV "rqVotes"
#define cnf_dumpVotes "dumpVotes"

#define cnf_stopAtWinners "stopAtWinners"

#define cnf_W "W"
#define cnf_S1 "S1"
#define cnf_Sleader "Sleader"

#define cnf_taprocessk "taprocessk"
#define cnf_taprocesslb "taprocesslb"
#define cnf_rndlatfrom "rndlatfrom"
#define cnf_rndlatto "rndlatto"

#define cnf_bandwidth "bandwidth"
#define cnf_ksize "ksize"
#define cnf_ShRqsize "rqsize"
#define cnf_ShPnsize "pnsize"
#define cnf_lbsize "lbsize"
#define cnf_msgnsize "msgnsize"
#define cnf_msize "msize"
#define cnf_diskbw "diskbw"

struct Config
{
public:
    static Config& get()
    {
        static Config instance;
        return instance;
    }
    void ParseConfig(std::string fileName);
    void WriteDefaultConfig(std::string fileName);
    template<typename _T>
    _T getValue(std::string key);

    Config(Config const&) = delete;            // avoid copies
    Config& operator=(Config const&) = delete; // avoid copies

private:
    Config() {};
    
    std::unordered_map<std::string, std::string> ConfigStorage;

    const std::unordered_map<std::string, std::pair<std::string, std::string>> Defaults{
        { "logging_level", { "8", "подробность сообщений (точнее их обилие)" } },
        { "n", { "200", "количество (W и) S узлов" } },
        { "k", { "500", "количество A узлов на один S узел" } },
        { "r", { "100", "количество раундов" } },

        { "voteTime", { "40000000", "время до очередного голоса (за случайный узел)" } },
        { "rqVotes", { "50", "необходимое для победы число голосов" } },
        { "dumpVotes", { "0", "подробность выдачи списка поданных голосов и статистики" } },

        { "stopAtWinners", { "5", "остановить симуляцию, когда достигнуто указанное число победителей" } },

        { "W", { "0", "номер узла W" } },
        { "S1", { "1", "номер узла S1" } },
        { "Sleader", { "2", "номер узла S_leader" } },

        { "taprocessk", { "10000", "время на обработку узлом A блока K (нс)" } },
        { "taprocesslb", { "10000", "время на обработку узлом A блока Leader_beacon (нс)" } },
        { "rndlatfrom", { "100000", "случайные задержки: от (нс)" } },
        { "rndlatto", { "40000000", "случайные задержки: до (нс)" } },

        { "ksize",  { "800000", "размер блока K (в битах)" } },
        { "rqsize", { "1000", "размер блока shadow_request (в битах)" } },
        { "pnsize", { "1000", "размер блока shadow_response (в битах)" } },
        { "lbsize", { "1000", "размер блока LeaderBeacon (в битах)" } },
        { "msgnsize", { "1000", "размер блока M-sign (в битах)" } },
        { "msize", { "1000", "размер блока M (в битах)" } },
        { "bandwidth", { "0.1", "сетевая пропускная способность (каждого) узла (бит/нс, так что 100 Мбит/с = 0.1)" } },
        { "diskbw", { "0.02", "быстродействие диска Sleader-а (бит/нс, так что 18000*144*8/с = 0.02); записываются приходящие лидеру сообщения и посылаемые им (но не проходящее через него)" } }
    };
};

template<>
int Config::getValue(std::string key);

template<>
std::string Config::getValue(std::string key);

template<>
double Config::getValue(std::string key);

inline int confInt(std::string key) { return Config::get().getValue<int>(key); }
inline std::string confStr(std::string key) { return Config::get().getValue<std::string>(key); }
inline double confDbl(std::string key) { return Config::get().getValue<double>(key); }

