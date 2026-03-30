#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <ctime>
#include "shared.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: ./worker <localPid> <totalCpuBurstNs>\n";
        return 1;
    }

    int localPid = atoi(argv[1]);
    unsigned int totalCpuBurstNs = (unsigned int)strtoul(argv[2], nullptr, 10);
    unsigned int totalCpuUsed = 0;

    key_t shmKey = ftok(".", 'C');
    if (shmKey == -1) {
        cerr << "Worker: ftok for shared memory failed: " << strerror(errno) << "\n";
        return 1;
    }

    int shmid = shmget(shmKey, sizeof(SimClock), 0666);
    if (shmid == -1) {
        cerr << "Worker: shmget failed: " << strerror(errno) << "\n";
        return 1;
    }

    SimClock* clk = (SimClock*)shmat(shmid, nullptr, 0);
    if (clk == (SimClock*)-1) {
        cerr << "Worker: shmat failed: " << strerror(errno) << "\n";
        return 1;
    }

    key_t msgKey = ftok(".", 'Q');
    if (msgKey == -1) {
        cerr << "Worker: ftok for message queue failed: " << strerror(errno) << "\n";
        shmdt(clk);
        return 1;
    }

    int msgid = msgget(msgKey, 0666);
    if (msgid == -1) {
        cerr << "Worker: msgget failed: " << strerror(errno) << "\n";
        shmdt(clk);
        return 1;
    }

    srand(getpid() ^ time(nullptr));

    Message msg;

    while (true) {
        if (msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), getpid(), 0) == -1) {
            if (errno == EIDRM || errno == EINTR) {
                shmdt(clk);
                return 0;
            }
            cerr << "Worker: msgrcv failed: " << strerror(errno) << "\n";
            shmdt(clk);
            return 1;
        }

        unsigned int quantum = msg.quantum;
        unsigned int remaining = totalCpuBurstNs - totalCpuUsed;

        Message reply;
        reply.mtype = 1;
        reply.index = localPid;

        // terminate if remaining CPU needed is less than or equal to quantum
        if (remaining <= quantum) {
            reply.usedTime = remaining;
            reply.action = 2; // terminated
            totalCpuUsed += remaining;

            if (msgsnd(msgid, &reply, sizeof(Message) - sizeof(long), 0) == -1) {
                cerr << "Worker: msgsnd failed: " << strerror(errno) << "\n";
                shmdt(clk);
                return 1;
            }
            break;
        }

        // 20% chance to block
        int chance = rand() % 100;
        if (chance < 20) {
            reply.usedTime = 1 + (rand() % quantum);
            reply.action = 1; // blocked
            totalCpuUsed += reply.usedTime;
        } else {
            reply.usedTime = quantum;
            reply.action = 0; // used full quantum
            totalCpuUsed += quantum;
        }

        if (msgsnd(msgid, &reply, sizeof(Message) - sizeof(long), 0) == -1) {
            cerr << "Worker: msgsnd failed: " << strerror(errno) << "\n";
            shmdt(clk);
            return 1;
        }
    }

    shmdt(clk);
    return 0;
}
