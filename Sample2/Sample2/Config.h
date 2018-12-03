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
        { "n", {"500", "���������� W � S �����"} },
        { "k", {"100", "���������� A ����� �� ���� S ����"} },
        { "tkblock", {"650", "����� �� ����������� k-�����"} },
        { "tshreq", {"1", "����� �� ����������� shadow_request"} },
        { "tshresp", {"1", "����� �� ����������� shadow_response"} },
        { "tleadbeac", {"1", "����� �� ����������� leader_beacon"} },
        { "tmblock_sign", {"1", "����� �� ����������� mblock_sign"} },
        { "tmblock", {"1", "����� �� ����������� mblock"} }
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

