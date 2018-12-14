#pragma once

#include "stdafx.h"

#define bigFifo 1000000

extern double taprocessk;
extern double taprocesslb;

extern double bwKdelay;
extern double bwShRqdelay;
extern double bwShPndelay;
extern double bwLBdelay;
extern double bwMSGNdelay;
extern double bwMdelay;

extern double dbK;
extern double dbShRq;
extern double dbShPn;
extern double dbLB;
extern double dbMS;
extern double dbM;

extern double diskspeed;

void InitBWDB();
double bwDelay(BlockType::BlockType bt);
