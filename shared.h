#ifndef SHARED_H
#define SHARED_H

#include <sys/types.h>

const int TABLE_SIZE = 20;
const unsigned int BILLION = 1000000000;
const unsigned int QUANTUM_NS = 25000000; // 25 ms

struct SimClock {
    unsigned int seconds;
    unsigned int nanoseconds;
};

struct Message {
    long mtype;
    int index;       // PCB slot or local pid
    int quantum;     // time quantum sent by oss
    int usedTime;    // actual time used by worker
    int action;      // 0 = full quantum, 1 = blocked, 2 = terminated
};

struct PCB {
    bool occupied;
    pid_t pid;
    int localPid;

    unsigned int startSeconds;
    unsigned int startNano;

    unsigned int serviceTimeSeconds;
    unsigned int serviceTimeNano;

    unsigned int eventWaitSec;
    unsigned int eventWaitNano;

    bool blocked;
};

#endif
