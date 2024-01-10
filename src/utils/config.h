#ifndef _CONFIG_HPP_INCLUDED_
#define _CONFIG_HPP_INCLUDED_
#include "policy.h"

const PolicyType POLICY = policyTypes[3]; // 0~8

const long long DISK_SIZE = 975396; // disk_capacity=DISK_SIZE*CHUNK_SIZE
const long long CHUNK_NUM = 975396;

const char *TRACE_PATH = "../trace/zipfian/10w_82_4k/tracea_run.txt";
const char *CACHE_PATH = "/data2/Cache/cache_0.1.bin";
const char *CACHE_PATH_W = "/data2/Cache/cache_0.2.bin";
const long long CACHE_SIZE = CHUNK_NUM * 0.1;

const long long HALF_SIZE = CACHE_SIZE*0.5;
const long long P_SIZE = CACHE_SIZE*2;
const char *DISK_PATH = "/SMR/db/storage/disk.bin";

typedef long long ll;
const long long CHUNK_SIZE = 4 * 1024; // 4kB


const bool O_DIRECT_ON = false;
const bool RANDOM_ON = false;
const int RANDOM_THRESHOLD = 0; 

#endif /*_CONFIG_INCLUDED_*/
