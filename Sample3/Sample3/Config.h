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
        { "logging_level", { "8", "����������� ��������� (������ �� ������)" } },
        { "n", { "200", "���������� (W �) S �����" } },
        { "k", { "500", "���������� A ����� �� ���� S ����" } },
        { "r", { "100", "���������� �������" } },

        { "voteTime", { "40000000", "����� �� ���������� ������ (�� ��������� ����)" } },
        { "rqVotes", { "50", "����������� ��� ������ ����� �������" } },
        { "dumpVotes", { "0", "����������� ������ ������ �������� ������� � ����������" } },

        { "stopAtWinners", { "5", "���������� ���������, ����� ���������� ��������� ����� �����������" } },

        { "W", { "0", "����� ���� W" } },
        { "S1", { "1", "����� ���� S1" } },
        { "Sleader", { "2", "����� ���� S_leader" } },

        { "taprocessk", { "10000", "����� �� ��������� ����� A ����� K (��)" } },
        { "taprocesslb", { "10000", "����� �� ��������� ����� A ����� Leader_beacon (��)" } },
        { "rndlatfrom", { "100000", "��������� ��������: �� (��)" } },
        { "rndlatto", { "40000000", "��������� ��������: �� (��)" } },

        { "ksize",  { "800000", "������ ����� K (� �����)" } },
        { "rqsize", { "1000", "������ ����� shadow_request (� �����)" } },
        { "pnsize", { "1000", "������ ����� shadow_response (� �����)" } },
        { "lbsize", { "1000", "������ ����� LeaderBeacon (� �����)" } },
        { "msgnsize", { "1000", "������ ����� M-sign (� �����)" } },
        { "msize", { "1000", "������ ����� M (� �����)" } },
        { "bandwidth", { "0.1", "������� ���������� ����������� (�������) ���� (���/��, ��� ��� 100 ����/� = 0.1)" } },
        { "diskbw", { "0.02", "�������������� ����� Sleader-� (���/��, ��� ��� 18000*144*8/� = 0.02); ������������ ���������� ������ ��������� � ���������� �� (�� �� ���������� ����� ����)" } }
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

