#include "stdafx.h"

int log_fifo_logging_level = 10;

char NTTS(NodeType nt)
{
	if (NodeType::Unknown == nt) return '?';
	if (NodeType::A == nt) return 'A';
	if (NodeType::S == nt) return 'S';
	if (NodeType::W == nt) return 'W';
	return '*';
}

k_block create_k_block(int pkbl, int inf1, int inf2, NodeType nt) { k_block a; a.sender = nt; a.info1 = inf1; a.info2 = inf2; a.p_kblock = pkbl; return a; }
m_block create_m_block(int inf1, int inf2, NodeType nt) { m_block a; a.sender = nt; a.info1 = inf1; a.info2 = inf2; return a; }
send_shadow_rq create_send_shadow_rq(int leader, int inf1, int inf2, int inf3, NodeType nt) { send_shadow_rq a; a.sender = nt; a.info1 = inf1; a.info2 = inf2; a.info3 = inf3; a.leader = leader; return a; }
recv_shadow_rq create_recv_shadow_rq(int inf1, int inf2, NodeType nt) { recv_shadow_rq a; a.sender = nt; a.info1 = inf1; a.info2 = inf2; return a; }

ostream& operator << (ostream& os, const k_block& k) { return os << "k_block(" << NTTS(k.sender) << ";" << k.info1 << ";" << k.info2 << ";" << k.p_kblock << ")"; }
ostream& operator << (ostream& os, const m_block& k) { return os << "m_block(" << NTTS(k.sender) << ";" << k.info1 << ";" << k.info2 << ")"; }
ostream& operator << (ostream& os, const send_shadow_rq& k) { return os << "send_shadow_rq(" << NTTS(k.sender) << ";" << k.info1 << ";" << k.info2 << ";" << k.info3 << ";" << k.leader << ")"; }
ostream& operator << (ostream& os, const recv_shadow_rq& k) { return os << "recv_shadow_rq(" << NTTS(k.sender) << ";" << k.info1 << ";" << k.info2 << ")"; }
