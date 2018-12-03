#pragma once

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
        { "n", {"500", "количество W и S узлов"} },
        { "k", {"100", "количество A узлов на один S узел"} },
        { "tkblock", {"650", "время на отправление k-блока"} },
        { "tshreq", {"1", "время на отправление shadow_request"} },
        { "tshresp", {"1", "время на отправление shadow_response"} },
        { "tleadbeac", {"1", "время на отправление leader_beacon"} },
        { "tmblock_sign", {"1", "время на отправление mblock_sign"} },
        { "tmblock", {"1", "время на отправление mblock"} }
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

