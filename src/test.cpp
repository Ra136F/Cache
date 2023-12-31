#include "simulator/randomSl.h"
#include "simulator/fifoSl.h"
#include "simulator/lruSl.h"
#include "simulator/lfuSl.h"
#include "simulator/lirsSl.h"
#include "simulator/arcSl.h"
#include "simulator/clockproSl.h"
#include "simulator/2qSl.h"
// #include "simulator/tinylfuSl.h"

using namespace std;

int main(){
    Sl *sim = nullptr;
    switch(POLICY){
        case PolicyType::RANDOM: 
            sim = new RandomSl();
            break;
        case PolicyType::FIFO:
            sim = new FifoSl();
            break;
        case PolicyType::LFU:
            sim = new LfuSl();
            break;
        case PolicyType::LRU:
            sim = new LruSl();
            break;
        case PolicyType::LIRS:
            sim = new LirsSl();
            break;
        case PolicyType::ARC:
            sim = new ArcSl();
            break;
        case PolicyType::CLOCKPRO:
            sim = new ClockproSl();
            break;
        case PolicyType::TQ:
            sim = new TqSl();
            break;
        // case PolicyType::TINYLFU:
        //     sim = new TinylfuSl();
        //     break;
    }

    sim->test();
    sim->statistic();
    return 0;
}
/*
sudo mount /dev/mmcblk0p1 /mnt/eMMC
lsblk
g++ -std=c++17 -o test test.cpp
./test
*/
