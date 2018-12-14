#include "stdafx.h"

double Mu;
const double Mudef = 10000;
double Sigma;
const double Sigmadef = 10000;
int Wnum;
const int Wnumdef = 2;
int Snum;
const int Snumdef = 2;
int Anum;
const int Anumdef = 10;
double Ttr;
const double Ttrdef = 1000000;
int logging_level;
const int logging_leveldef = 10;


int sc_main(int argc, char *argv[])
{
    sc_set_time_resolution(1, SC_NS);
    //sc_set_default_time_unit(1, SC_NS);

    if (argc <= 1)
    {
        const std::string defConfigFileName = "config.txt";
        cout << "Usage: <sample2> <config file name>" << endl <<
            "Otherwise tries to write '" << defConfigFileName << "' config file with default values." << endl;
        ifstream f(defConfigFileName);
        if (f.good())
        {
            cout << "Error,Config,default config file seems to be existing" << endl;
        }
        else
        {
            Config::get().WriteDefaultConfig(defConfigFileName);
        }
        return 1;
    }

    Config::get().ParseConfig(argv[1]);
    // init stuff using config
    log_fifo_logging_level = confInt(cnf_LoggingLevel);
    redefineRandomLatency(confInt(cnf_rndlatfrom), confInt(cnf_rndlatto));
    InitBWDB();
    //

    int n = confInt("n");
    int k = confInt("k");

    WSA wsa("WSA", n, k*n, confInt(cnf_W), confInt(cnf_S1), confInt(cnf_Sleader));

    double Ttr = 1000e9;
    if (Ttr > 0)
        sc_start(Ttr, SC_NS, SC_RUN_TO_TIME);
    else
        sc_start();

    return 0;
}
