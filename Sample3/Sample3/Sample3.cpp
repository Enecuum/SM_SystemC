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
        cout << "Usage: <sample3> <config file name>" << endl <<
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
    redefineRandomLatency(confInt(cnf_rndlatfrom), confInt(cnf_rndlatto), confInt(cnf_n), confDbl(cnf_voteTime));
    InitBWDB();
    //

    int n = confInt(cnf_n);
    int k = confInt(cnf_k);
    int r = confInt(cnf_r);
    int rqV = confInt(cnf_rqV);

    WSA wsa("WSA", n, k*n, confInt(cnf_W), confInt(cnf_S1), confInt(cnf_Sleader), n, r, rqV);

    double Ttr = 1000e9;
    if (Ttr > 0)
        sc_start(Ttr, SC_NS, SC_RUN_TO_TIME);
    else
        sc_start();

    return 0;
}
