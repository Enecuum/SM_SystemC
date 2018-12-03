#pragma once

SC_MODULE(WSA)
{
    std::vector<WnodeP> Wnodes;
    std::vector<SnodeP> Snodes;
    std::vector<AnodeP> Anodes;

    int Wnum;
    int Snum;
    int Anum;

    void BroadcastK(WnodeP w, k_block k);

    SC_HAS_PROCESS(WSA);
    WSA(sc_module_name name, int Wnum_, int Snum_, int Anum_);
    ~WSA();

    void main();
};
typedef WSA* WSAP;
