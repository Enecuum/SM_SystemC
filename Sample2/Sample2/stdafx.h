// stdafx.h: включаемый файл дл€ стандартных системных включаемых файлов
// или включаемых файлов дл€ конкретного проекта, которые часто используютс€, но
// не часто измен€ютс€
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <random>
#include <cmath>
#include <sstream>
#include <string>
#include <unordered_map>
#include <chrono>

#include <systemc.h>

// forward struct declarations
struct WSA;
struct Wnode;
struct Snode;
struct Anode;
typedef WSA* WSAP;
typedef Wnode* WnodeP;
typedef Snode* SnodeP;
typedef Anode* AnodeP;
struct k_block;
struct m_block;
struct send_shadow_rq;
struct recv_shadow_rq;

#include "Config.h"
#include "Latency.h"

#include "Base.h"
#include "Node.h"
#include "Wnode.h"
#include "Anode.h"
#include "Snode.h"
#include "Top.h"

// TODO: ”становите здесь ссылки на дополнительные заголовки, требующиес€ дл€ программы
