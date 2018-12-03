// W-S-A.
//
// examples: simple_perf

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
        std::cout << "Usage: <sample1> <nanoseconds to run (default=" << Ttrdef << "> <mu (default=" << Mudef << ")> <sigma (default=" << Sigmadef <<
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




int VectorSampleWithMultipleModules_sc_main(int argc, char *argv[])
{
	std::cout << "Usage: <sample1> <mu (default=" << Mudef << ")> <sigma (default=" << Sigmadef << ")> <Wnum (default=" << Wnumdef <<
		")> <Snum (default=" << Snumdef << ")> <Anum (default=" << Anumdef << ")>" << endl;

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

	baseMu = Mu;
	baseSigma = Sigma;

	std::cout << "   mu = " << Mu << "   sigma = " << Sigma << endl <<
		"   Wnum = " << Wnum << "   Snum = " << Snum << "   Anum = " << Anum << endl;

	top top1("Top1", 10);
	sc_start();
	return 0;
}

int ExampleSampleScfifo_sc_main(int argc, char *argv[])
{
	example_fifo ex_fifo("ex_fifo0");
	sc_start(3000, SC_NS);
	return 0;
}
