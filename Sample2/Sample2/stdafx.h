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
#include <map>
#include <chrono>

#include <systemc.h>

// forward struct declarations
struct WSA;
struct Snode;
struct Anode;
typedef WSA* WSAP;
typedef Snode* SnodeP;
typedef Anode* AnodeP;
struct k_block;
struct m_block;
struct send_shadow_rq;
struct recv_shadow_rq;
namespace BlockType { enum BlockType; }

#include "Config.h"
#include "Latency.h"
#include "Node.h"
#include "Base.h"

#include "Anode.h"
#include "Top.h"
#include "Snode.h"

// TODO: ”становите здесь ссылки на дополнительные заголовки, требующиес€ дл€ программы
