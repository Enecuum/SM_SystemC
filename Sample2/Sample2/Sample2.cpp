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

    int n = confInt("n");
    std::string k = confStr("k");
    double tkblock = confDbl("tkblock");
    int kkk = confInt("kkk");

    LatNode l1, l2, l3;
    l1.distToMS(l2);
    distMS(l1, l2);
    l3.distToMS(l1);
    l1.distToMS(l3);

    l1.distTo(l2);
    dist(l1, l2);
    l3.distTo(l1);
    l1.distTo(l3);

    return 0;
}


int Sample1_sc_main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        std::cout << "Usage: <sample2> <nanoseconds to run (default=" << Ttrdef << "> <mu (default=" << Mudef << ")> <sigma (default=" << Sigmadef <<
            ")> <Wnum (default=" << Wnumdef << ")> <Snum (default=" << Snumdef << ")> <Anum (default=" << Anumdef <<
            ")> <logging level (10 is full, default=" << logging_leveldef << ">" << endl;
        return 1;
    }

    if (argc > 1)
    {
        double newTtr = atof(argv[1]);
        Ttr = (newTtr >= 0) ? newTtr : Ttrdef;
    }
    else
    {
        Ttr = Ttrdef;
    }

    if (argc > 3)
    {
        double newMu = atof(argv[2]);
        double newSigma = atof(argv[3]);
        Mu = (newMu > 0) ? newMu : Mudef;
        Sigma = (newSigma > 0) ? newSigma : Sigmadef;
    }
    else
    {
        Mu = Mudef;
        Sigma = Sigmadef;
    }
    baseMu = Mu;
    baseSigma = Sigma;

    if (argc > 6)
    {
        int newWnum = atoi(argv[4]);
        int newSnum = atoi(argv[5]);
        int newAnum = atoi(argv[6]);
        Wnum = (newWnum > 0) ? newWnum : Wnumdef;
        Snum = (newSnum > 0) ? newSnum : Snumdef;
        Anum = (newAnum > 0) ? newAnum : Anumdef;
    }
    else
    {
        Wnum = Wnumdef;
        Snum = Snumdef;
        Anum = Anumdef;
    }

    if (argc > 7)
    {
        int newll = atoi(argv[7]);
        logging_level = (newll >= 0) ? newll : logging_leveldef;
    }
    else
    {
        logging_level = logging_leveldef;
    }
    log_fifo_logging_level = logging_level;

    cout << "Parameters," << Ttr << "," << Mu << "," << Sigma << "," << Wnum << "," << Snum << "," << Anum << "," << logging_level << endl;
    //std::cout << " Time to run (nanoseconds; 0 --- without a stop): " << Ttr << endl <<
    //    "   mu = " << Mu << "   sigma = " << Sigma << endl <<
    //    "   Wnum = " << Wnum << "   Snum = " << Snum << "   Anum = " << Anum << endl <<
    //    "   logging level = " << logging_level << endl;

    WSA wsa("WSA", Wnum, Snum, Anum);
    if (Ttr > 0)
        sc_start(Ttr, SC_NS);
    else
        sc_start();
    return 0;
}
