#include "stdafx.h"

int log_fifo_logging_level = 8;

char NTTS(NodeType::NodeType nt)
{
	if (NodeType::Unknown == nt) return '?';
	if (NodeType::A == nt) return 'A';
	if (NodeType::S1 == nt) return '1'; // S1
	if (NodeType::W == nt) return 'W'; // only one W (the one sending k_block)
    if (NodeType::SL == nt) return 'L'; // S_leader
    if (NodeType::S == nt) return 'S'; // typical chord node
	return '*';
}

char BTTS(BlockType::BlockType bt)
{
    if (BlockType::Unknown == bt) return '?';
    if (BlockType::K == bt) return 'K';
    if (BlockType::ShRq == bt) return 'Q';
    if (BlockType::ShPn == bt) return 'P';
    if (BlockType::LB == bt) return 'L';
    if (BlockType::MSgn == bt) return 'G';
    if (BlockType::M == bt) return 'M';
    return '*';
}

routing_block create_routing_block(int P_, int R_, int token_, BlockType::BlockType bt_, NodeType::NodeType nt_, int info1_, int info2_)
{
    routing_block rb;
    rb.P = P_;
    rb.R = R_;
    rb.token = token_;
    rb.bt = bt_;
    rb.sender = nt_;
    rb.info1 = info1_;
    rb.info2 = info2_;
    return rb;
}
k_block create_k_block(int inf1, int inf2, NodeType::NodeType nt) { k_block a; a.sender = nt; a.info1 = inf1; a.info2 = inf2; return a; }
send_shadow_rq create_send_shadow_rq(int inf1, int inf2, NodeType::NodeType nt) { send_shadow_rq a; a.sender = nt; a.info1 = inf1; a.info2 = inf2; return a; }
recv_shadow_rq create_recv_shadow_rq(int inf1, int inf2, NodeType::NodeType nt) { recv_shadow_rq a; a.sender = nt; a.info1 = inf1; a.info2 = inf2; return a; }
lb_block create_lb_block(int inf1_, int inf2_, NodeType::NodeType nt) { lb_block a; a.sender = nt; a.info1 = inf1_; a.info2 = inf2_; return a; }
msgn_block create_msgn_block(int inf1_, int inf2_, NodeType::NodeType nt) { msgn_block a; a.sender = nt; a.info1 = inf1_; a.info2 = inf2_; return a; }
m_block create_m_block(int inf1, int inf2, NodeType::NodeType nt) { m_block a; a.sender = nt; a.info1 = inf1; a.info2 = inf2; return a; }

ostream& operator << (ostream& os, const routing_block& k)
{
    return os << "routing_block(" <<
        k.P << ";" << k.R << ";" << k.token << ";" << BTTS(k.bt) << "; " <<
        NTTS(k.sender) << ";" << k.info1 << ";" << k.info2 << ")";
}
ostream& operator << (ostream& os, const k_block& k) { return os << "k_block(" << NTTS(k.sender) << ";" << k.info1 << ";" << k.info2 << ")"; }
ostream& operator << (ostream& os, const send_shadow_rq& k) { return os << "send_shadow_rq(" << NTTS(k.sender) << ";" << k.info1 << ";" << k.info2 << ")"; }
ostream& operator << (ostream& os, const recv_shadow_rq& k) { return os << "recv_shadow_rq(" << NTTS(k.sender) << ";" << k.info1 << ";" << k.info2 << ")"; }
ostream& operator << (ostream& os, const lb_block& k) { return os << "lb_block(" << NTTS(k.sender) << ";" << k.info1 << ";" << k.info2 << ")"; }
ostream& operator << (ostream& os, const msgn_block& k) { return os << "msgn_block(" << NTTS(k.sender) << ";" << k.info1 << ";" << k.info2 << ")"; }
ostream& operator << (ostream& os, const m_block& k) { return os << "m_block(" << NTTS(k.sender) << ";" << k.info1 << ";" << k.info2 << ")"; }
