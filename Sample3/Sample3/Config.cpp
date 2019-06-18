#include "stdafx.h"

void Config::ParseConfig(std::string fileName)
{
    std::ifstream file(fileName);
    std::string line;
    while (std::getline(file, line))
    {
        if ((0 >= line.length()) || (';' == line.at(0)))
            continue;
        std::istringstream is_line(line);
        std::string key;
        if (std::getline(is_line, key, '='))
        {
            std::string value;
            if (std::getline(is_line, value))
            {
            }
            else
            {
                value = "";
            }
            ConfigStorage[key] = value;
            cout << "Trace,Config,key " << key << " = value " << value << endl;
        }
    }
    file.close();
    // filling default values
    for (auto dit : Defaults)
    {
        auto key = dit.first;
        auto fit = ConfigStorage.find(key);
        if (fit == ConfigStorage.end())
        {
            std::string value = dit.second.first;
            ConfigStorage[key] = value;
            cout << "Trace,Config,key " << key << " = default value " << value << endl;
        }
    }
}

void Config::WriteDefaultConfig(std::string fileName)
{
    std::ofstream file(fileName);
    if (!file.is_open())
    {
        cout << "Error,Config,can't write default config to " << fileName << endl;
        return;
    }
    for (auto dit : Defaults)
    {
        file << ';' << dit.second.second << endl;
        file << dit.first << '=' << dit.second.first << endl;
    }
    file.close();
}

template<>
int Config::getValue(std::string key)
{
    auto it = ConfigStorage.find(key);
    if (it == ConfigStorage.end())
    {
        cout << "InternalError,Config,int not found " << key << endl;
        return 0;
    }
    else
    {
        int res = std::stoi(it->second);
        cout << "Trace,Config,int " << key << " is " << res << endl;
        return res;
    }
}

template<>
std::string Config::getValue(std::string key)
{
    auto it = ConfigStorage.find(key);
    if (it == ConfigStorage.end())
    {
        cout << "InternalError,Config,string not found " << key << endl;
        return "";
    }
    else
    {
        cout << "Trace,Config,string " << key << " is " << it->second << endl;
        return it->second;
    }
}

template<>
double Config::getValue(std::string key)
{
    auto it = ConfigStorage.find(key);
    if (it == ConfigStorage.end())
    {
        cout << "InternalError,Config,double not found " << key << endl;
        return 0;
    }
    else
    {
        double res = std::stod(it->second);
        cout << "Trace,Config,double " << key << " is " << res << endl;
        return res;
    }
}
