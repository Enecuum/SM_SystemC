#pragma once

SC_MODULE(WSA)
{
    std::vector<SnodeP> Snodes;
    std::vector<AnodeP> Anodes;

    int Snum;
    int Anum;

    int Wno;
    int S1no;
    int Sleaderno;

    void BroadcastK(SnodeP w, k_block k);

    SC_HAS_PROCESS(WSA);
    WSA(sc_module_name name, int Snum_, int Anum_, int Wno_, int S1no_, int Sleaderno_);
    ~WSA();

    void main();
};
typedef WSA* WSAP;
