#ifndef _LRU_SIMULATORHPP_INCLUDED_
#define _LRU_SIMULATORHPP_INCLUDED_

#include <iostream>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <cassert>
#include <set>
#include <map>
#include <random>
#include <string> // for to_string()

// for file_open
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include "../utils/bitmap.h"
#include "../utils/chunk.h"
#include "../utils/config.h"
#include "../utils/statistic.h"

#include "./global_variables.h"

using namespace std;

int llAction = 0;
int lAction = 0;
int cAction = 0;
int turn = 0; // q-learning训练轮次
int currentAction = 0;
int lastAction = 0;        // 上一次执行的动作，1为写入smr，0为其他
long long currentTime = 0; // 当前动作时间
long long lastTime = 0;    // 上一次动作时间
long long llTime = 0;      // 上上次动作时间
double qTable[8][8][2][2] = {0};
double rate = 0.1;
// 探索因子
double greedy = 0.8;
// 奖励函数时间阈值
////t1=90,t2=40000,t3=70000,t4=200000,t5=3000000
// 80 15000 25000 35000 100000
double t1 = 300;

double t2 = 2000;

int backTime=0;
int b1=0;
int b2=0;
struct CurrentState
{
    // 当前I/O间隔
    int current_IO;
    // 上一个I/O间隔
    int last_IO;
    // 上一次执行的动作
    int last_Action;

    int ll;

    int l;

    int c;
};

CurrentState currentState;

int calculate_timeInterval(long long currentTime, long long lastTime)
{
    long long interval = currentTime - lastTime;
    // cout<<"间隔:"<<interval<<endl;
    if (interval <= 200)
    {
        return 0;
    }
    else if (interval > 200 && interval <= 500)
    {
        return 1;
    }
    else if (interval > 500 && interval <= 1000)
    {
        return 2;
    }
    else if (interval > 1000 && interval <= 2000)
    {
        return 3;
    }
    else if (interval > 2000 && interval <= 3000)
    {
        return 4;
    }
    else if (interval > 3000 && interval <= 5000)
    {
        return 5;
    }
    else if (interval > 5000 && interval <= 10000)
    {
        return 6;
    }
    else
    {
        return 7;
    }
}

double get_expected_max_score(CurrentState currentState)
{
    double s = -10000;
    for (int i = 0; i < 2; i++)
    {
        s = std::max(s, qTable[currentState.current_IO][currentState.last_IO][currentState.last_Action][i]);
    }
    return s;
}

// 返回当前状态下执行动作的相应奖励
double returnAward(CurrentState currentState, long long response_time)
{

    // 得到执行动作后的预期奖励，见函数说明
    double excepted_max_score = get_expected_max_score(currentState);
    // 对应马尔可夫决策过程
    // Qtable[x][y][i] = Qtable[x][y][i] + rate * ( reward[x1][y1] + max( Qtable[x1][y1][i] ))
    // 中的 reward[x1][y1] + max( Qtable[x1][y1][i] )
    //  t1=80,t2=500,t3=1000,t4=2000,t5=3000
    if (response_time <= t1)
    {
        return 1.0 + 1.0 * rate * excepted_max_score;
    }
    else if (response_time > t1 && response_time <= t2)
    {
        return 0 + 1.0 * rate * excepted_max_score;
    }
    else
    {
        return -1 + 1.0 * rate * excepted_max_score;
    }
}

// 选择动作
int chooseAction(CurrentState currentState)
{
    // 生成随机数
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    double randomNuber = static_cast<double>(std::rand()) / RAND_MAX;

    if (turn > 250 && turn <= 500)
    {
        greedy = 0.6;
    }
    else if (turn > 500 && turn <= 1000)
    {
        greedy = 0.4;
    }
    else if (turn > 1000 && turn <= 2000)
    {
        greedy = 0.2;
    }
    else if (turn > 2000 && turn <= 5000)
    {
        greedy = 0.1;
    }
    else
    {
        greedy = 0.05;
    }

    if (randomNuber >= greedy)
    {
        currentAction = std::rand() % 2;
    }
    else
    {
        if (qTable[currentState.current_IO][currentState.last_IO][currentState.last_Action][0] > qTable[currentState.current_IO][currentState.last_IO][currentState.last_Action][1])
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    return currentAction;
}


class Sl
{
public:
    void test();
    void statistic();

protected:
    int fd_cache, fd_disk, fd_cache_w;

    long long curKey;
    vector<long long> free_cache;
    vector<long long> free_cache_w;
    vector<char *> cl;
    map<long long, chunk> chunk_map;
    map<long long, chunk> chunk_map_w;
    map<long long, chunk> chunk_map_ssd;
    map<long long, chunk> chunk_map_hotcache;
    Statistic st;

    Sl();
    ~Sl();
    void init();
    void initFreeCache();
    void initFile();
    void closeFile();

    bool readItem(vector<ll> &keys, struct timeval t);
    bool writeItem(vector<ll> &keys, struct timeval t);
    virtual void writeCache(const ll &key, int isReadCache, struct timeval t);

    void write_to_readCache(const ll &key, int isReadCache, struct timeval t);

    void writeBack(chunk *arg, struct timeval t);

    void readDisk(const long long &key); // disk->cache disk->buffer
    void writeDisk(const long long &key, struct timeval t);

    void readCache(const ll &offset_cache, int isReadCache); // cache->buffer
    void coverageCache(chunk *arg, struct timeval t);
    void isDirty(chunk *arg);
    bool Dirty(chunk *arg);

    void normRead(bool isCache, const long long &offset, const long long &size);
    void normWrite(bool isCache, const long long &offset, const long long &size);
    void odirectRead(int isCache, const long long &offset, const long long &size);
    void odirectWrite(int isCache, const long long &offset, const long long &size, const ll &key, struct timeval t);

    void readChunk(int isCache, const long long &offset, const long long &size);
    void writeChunk(int isCache, const long long &offset, const long long &size, const ll &key, struct timeval t);

    void printChunk(chunk *arg);
    void printChunkMap();
    void printFreeCache();

    bool isWriteCache();
    virtual bool isWriteCached(const ll &key) = 0;
    void checkFile(fstream &file);
    void remove(const ll &key, int isReadCache, struct timeval t);
    virtual bool isCached(const ll &key) = 0;
    virtual bool isSSDCached(const ll &key) = 0;
    virtual void accessKey(const ll &key, const bool &isGet) = 0;
    virtual void accessWriteKey(const ll &key, const bool &isGet) = 0;
    virtual void accessSSDCacheKey(const ll &key, const bool &isGet) = 0;
    virtual void accessHotCacheKey(const ll &key, const bool &isGet) = 0;
    virtual bool removeKey(const ll &key, int isReadCache) = 0;
    virtual bool compareHotCache(const ll &key1, const ll &key2) = 0;
    virtual ll getVictim() = 0;
    virtual ll getVictim2() = 0;
    virtual ll getWriteVictim() = 0;
    virtual ll getSSDVictim() = 0;
    virtual ll getPrevoisKey(const ll &key) = 0;

    int qLearn(struct timeval t);
    void updateQtable(struct timeval t, int action);
};

bool Sl::readItem(vector<ll> &keys, struct timeval t)
{
    //
    int isReadCache = 0;
    bool isTraceHit = true;
    cAction = 0;
    lAction = cAction;
    llAction = lAction;

    st.read_nums += keys.size();
    // cache hit
    for (int i = 0; i < keys.size(); i++)
    {
        if (isSSDCached(keys[i]))
        {
            st.read_hit_nums += 1;
            accessSSDCacheKey(keys[i], true);
            accessHotCacheKey(keys[i], true);
            readCache(chunk_map_ssd[keys[i]].offset_cache, isReadCache);
            keys[i] = -1;
        }
        else
        {
            for (int i = 0; i < keys.size(); i++)
            {
                if (keys[i] != -1)
                {
                    // cout<<"读未命中"<<keys[i]<<endl;
                    isTraceHit = false;
                    // removeKey(keys[i], 1);
                    // accessSSDCacheKey(keys[i], false);
                    accessHotCacheKey(keys[i], true);
                    // cout<<"accessKey"<<endl;
                    readDisk(keys[i]);
                    writeCache(keys[i], 1, t);
                }
            }
        }
    }
    return isTraceHit;
}

bool Sl::writeItem(vector<ll> &keys, struct timeval t)
{

    cAction = 1;
    lAction = cAction;
    llAction = lAction;
    bool isTraceHit = true;
    st.write_nums += keys.size();
    // cache hit
    for (int i = 0; i < keys.size(); i++)
    {
        if (isSSDCached(keys[i]))
        {
            st.write_hit_nums += 1;
            accessSSDCacheKey(keys[i], true);
            accessHotCacheKey(keys[i], true);
            coverageCache(&chunk_map_ssd[keys[i]], t);
            // chunk_map[keys[i]].offset_cache=-1;
            keys[i] = -1;
        }
    }
    // cache miss
    for (int i = 0; i < keys.size(); i++)
    {
        if (keys[i] != -1)
        {
            isTraceHit = false;
            // accessSSDCacheKey(keys[i], false);
            accessHotCacheKey(keys[i], false);
            // isDirty(&chunk_map_ssd[keys[i]]);
            writeCache(keys[i], 2, t);
            
        }
    }
    return isTraceHit;
}

void Sl::writeCache(const ll &key, int isReadCache, struct timeval t)
{
    // cout << "writeCache: ";
    if (!isWriteCache())
        return;
    // cache not full

    if (!free_cache.empty())
    {
        accessSSDCacheKey(key, false);
        ll offset_cache = free_cache.back();
        chunk item = {key, offset_cache};
        chunk_map_ssd[key] = item;
        free_cache.pop_back();
        writeChunk(1, offset_cache, CHUNK_SIZE, key, t);
        if(isReadCache==2){
            chunk_map_ssd[key].dirty=1;
        }
    }
    // cache full
    else
    {
        ll victim = getSSDVictim();
        assert(victim != -1);
        ll offset_cache = chunk_map_ssd[victim].offset_cache;
        int dirty = Dirty(&chunk_map_ssd[victim]);
        int action = qLearn(t);
        // victim is dirty and smr is busy, then find next victim
        if (action == 0)
        {
            if (dirty == 1)
            {
                int turn2 = 0;
                while (true)
                {
                    // find next victim
                    victim = getPrevoisKey(victim);
                    offset_cache = chunk_map_ssd[victim].offset_cache;
                    if (Dirty(&chunk_map_ssd[victim]) == 0 && compareHotCache(key, victim) == 1)
                    {
                        removeKey(victim, 1);
                        accessSSDCacheKey(key, false);
                        if (chunk_map_ssd.count(key) == 0)
                        {
                            chunk item = {key, offset_cache};
                            chunk_map_ssd[key] = item;
                        }
                        else
                        {
                            chunk_map_ssd[key].offset_cache = offset_cache;
                        }
                        chunk_map_ssd[victim].offset_cache = -1;
                        writeChunk(isReadCache, offset_cache, CHUNK_SIZE, key, t);
                        if (isReadCache == 2)
                        {
                            chunk_map_ssd[key].dirty = 1;
                        }
                        updateQtable(t, action);
                        break;
                    }
                    else if (turn2 >= 2000)
                    {
                        struct timeval current_t;
                        if (isReadCache == 2)
                        {
                            b1++;
                            writeDisk(key, t);
                            gettimeofday(&current_t, NULL);
                            currentTime = current_t.tv_sec * 1000000 + current_t.tv_usec;
                            lastTime = currentState.c;
                            llTime = currentState.l;
                            lastAction = 1;
                        }
                        break;
                    }
                    turn2 += 1;
                }
            }
            else
            {
                removeKey(victim, 1);
                accessSSDCacheKey(key, false);
                if (chunk_map_ssd.count(key) == 0)
                {
                    chunk item = {key, offset_cache};
                    chunk_map_ssd[key] = item;
                }
                else
                {
                    chunk_map_ssd[key].offset_cache = offset_cache;
                }
                chunk_map_ssd[victim].offset_cache = -1;
                writeChunk(2, offset_cache, CHUNK_SIZE, key, t);
                if (isReadCache == 2)
                {
                    chunk_map_ssd[key].dirty = 1;
                }
                updateQtable(t, action);
            }
        }
        else{
            removeKey(victim, 1);
            accessSSDCacheKey(key, false);
            if (chunk_map_ssd.count(key) == 0)
            {
                chunk item = {key, offset_cache};
                chunk_map_ssd[key] = item;
            }
            else
            {
                chunk_map_ssd[key].offset_cache = offset_cache;
            }
            chunk_map_ssd[victim].offset_cache = -1;
            writeChunk(2, offset_cache, CHUNK_SIZE, key, t);
            if (isReadCache == 2)
            {
                chunk_map_ssd[key].dirty = 1;
            }
            writeBack(&chunk_map_ssd[victim], t);
            updateQtable(t, action);
        }
    }
}

void Sl::test()
{
    cout << "-----------------------------------------------------------------" << endl;
    cout << "test start" << endl;
    st.getStartTime();

    fstream fin(TRACE_PATH);
    checkFile(fin);

    ll curSize;
    int type;
    char c;
    string s;
    getline(fin, s);

    struct timeval t0, t3, t1, t2;
    gettimeofday(&t0, NULL);
    while (fin >> curKey >> c >> curSize >> c >> type)
    {
        // if (st.total_trace_nums > 10) break;

        // cout << "----------" << curKey << ' ' << curSize << ' ' << type << "----------" << endl;

        st.total_trace_nums++;
        bool isTraceHit;

        ll begin = curKey / CHUNK_SIZE;
        ll end = (curKey + curSize - 1) / CHUNK_SIZE;
        st.request_size_v.push_back(end - begin + 1);
        st.total_request_size += end - begin + 1;

        vector<ll> keys;
        for (ll i = begin; i <= end; i++)
        {
            keys.push_back(i * CHUNK_SIZE);
        }

        gettimeofday(&t1, NULL);

        switch (type)
        {
        case 0:
            cout << "read" << endl;
            isTraceHit = readItem(keys, t1);
            break;
        case 1:
            cout << "write" << endl;
            isTraceHit = writeItem(keys, t1);
            break;
        }

        gettimeofday(&t2, NULL);
        long long deltaT = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
        st.latency_v.push_back(deltaT);
        st.total_latency += deltaT;
        printf("trace: %llu time: %lld us total: %lld us\n", st.total_trace_nums, deltaT, st.total_latency); // printf("trace: %llu time: %llu ns\n", st.total_trace_nums, deltaT);
        if (isTraceHit)
            st.hit_trace_nums++;
        // printChunkMap();
    }
    gettimeofday(&t3, NULL);
    st.total_time = (t3.tv_sec - t0.tv_sec) * 1000000 + (t3.tv_usec - t0.tv_usec);
    cout<<"写回磁盘次数:"<<backTime<<":b1:"<<b1<<":b2:"<<b2<<endl;
    st.getEndTime();
}

void Sl::checkFile(fstream &file)
{
    if (!file.is_open())
    {
        cout << "Error: opening trace file fail" << endl;
        exit(1);
    }
}

bool Sl::isWriteCache()
{
    if (!RANDOM_ON)
        return true;

    int min = 1, max = 100;
    random_device seed;
    ranlux48 engine(seed());                      // ���������������������?
    uniform_int_distribution<> distrib(min, max); // �����������Χ����Ϊ���ȷֲ�?
    int random = distrib(engine);                 // �����?
    // cout<<"random: "<<random<<endl;
    // printf("random: %d\n",random);
    if (random <= RANDOM_THRESHOLD)
        // printf("do write cache\n");
        return true;

    // printf("don't write cache\n");
    return false;
}

Sl::Sl()
{
    init();
    cout << "CachePolicy Construct success" << endl;
}

Sl::~Sl()
{
    closeFile();
}

void Sl::init()
{
    initFreeCache();
    initFile();
    cout << "init success" << endl;
}

void Sl::initFile()
{
    fd_cache_w = open(CACHE_PATH_W, O_RDWR | O_DIRECT, 0664);
    assert(fd_cache_w >= 0);
    fd_cache = open(CACHE_PATH, O_RDWR | O_DIRECT, 0664);
    assert(fd_cache >= 0);
    fd_disk = open(DISK_PATH, O_RDWR | O_DIRECT, 0664);
    assert(fd_disk >= 0);
}

void Sl::closeFile()
{
    close(fd_cache);
    close(fd_disk);
    close(fd_cache_w);
}

void Sl::printFreeCache()
{
    cout << "free cache: ";
    for (int i = 0; i < free_cache.size(); i++)
    {
        cout << free_cache[i] << ' ';
    }
    cout << endl;
    // cout << free_cache.back() << endl;
}

void Sl::writeBack(chunk *arg, struct timeval t)
{
    if (arg->dirty == 1)
    {
        arg->dirty = 0;
        b2++;
        writeDisk(arg->key, t);
    }
}

void Sl::normRead(bool isCache, const long long &offset, const long long &size)
{
    int fd = -1;
    if (isCache)
        fd = fd_cache;
    else
        fd = fd_disk;
    assert(fd >= 0);
    char buffer[CHUNK_SIZE];

    int res = pread64(fd, buffer, size, offset);
    assert(res == size);

    // close(fd);
}

void Sl::normWrite(bool isCache, const long long &offset, const long long &size)
{
    int fd = -1;
    if (isCache)
        fd = fd_cache;
    else
        fd = fd_disk;
    assert(fd >= 0);
    char buffer[CHUNK_SIZE] = "Ram15978";

    int res = pwrite64(fd, buffer, size, offset);
    assert(res == size);

    // close(fd);
}

void Sl::odirectRead(int isCache, const long long &offset, const long long &size)
{
    assert(offset != -1);
    int fd = -1;
    if (isCache == 1)
        fd = fd_cache;
    else if (isCache == 2)
        fd = fd_cache;
    else
        fd = fd_disk;
    assert(fd >= 0);

    char *buffer = nullptr;
    int res = posix_memalign((void **)&buffer, CHUNK_SIZE, size);
    assert(res == 0);
    res = pread64(fd, buffer, size, offset);
    assert(res == size);
    free(buffer);
}

void Sl::odirectWrite(int isCache, const long long &offset, const long long &size, const ll &key, struct timeval t)
{
    CurrentState currentState;
    assert(offset != -1);
    int fd = -1;
    if (isCache == 1)
        fd = fd_cache;
    else if (isCache == 2)
        fd = fd_cache;
    else
        fd = fd_disk;
    assert(fd>0);
    char *buffer = nullptr;
    int res = posix_memalign((void **)&buffer, CHUNK_SIZE, size);
    assert(res == 0);
    strcpy(buffer, "Ram15978");
    res = pwrite64(fd, buffer, size, offset);
    assert(res == size);
    free(buffer);
    // if (isCache == 1)
    // {
    //     fd = fd_cache;

    //     assert(fd >= 0);
    //     char *buffer = nullptr;
    //     int res = posix_memalign((void **)&buffer, CHUNK_SIZE, size);
    //     assert(res == 0);
    //     strcpy(buffer, "Ram15978");
    //     res = pwrite64(fd, buffer, size, offset);
    //     assert(res == size);
    //     free(buffer);
    // }

    // else if (isCache == 2)
    // {
    //     fd = fd_cache;

    //     assert(fd >= 0);
    //     char *buffer = nullptr;
    //     int res = posix_memalign((void **)&buffer, CHUNK_SIZE, size);
    //     assert(res == 0);
    //     strcpy(buffer, "Ram15978");
    //     res = pwrite64(fd, buffer, size, offset);
    //     assert(res == size);
    //     free(buffer);
    // }
    // else
    // {
    //     fd = fd_disk;
    //     char *buffer = nullptr;
    //     int res = posix_memalign((void **)&buffer, CHUNK_SIZE, size);
    //     assert(res == 0);
    //     strcpy(buffer, "Ram15978");
    //     res = pwrite64(fd, buffer, size, offset);
    //     assert(res == size);
    //     free(buffer);
    // }
}

void Sl::write_to_readCache(const ll &key, int isReadCache, struct timeval t)
{
    accessWriteKey(key, false);
    ll victim = getWriteVictim();
    chunk_map_w[victim].offset_cache = -1;
    // cout << "writeCache: ";
    if (!isWriteCache())
        return;
    // cache not full
    if (isReadCache == 1)
    {
        if (!free_cache.empty())

        {
            // cout << "cache not full" << endl;
            ll offset_cache = free_cache.back();
            chunk item = {key, offset_cache};
            chunk_map[key] = item;
            chunk_map[key].dirty = 1;
            free_cache.pop_back();
            writeChunk(1, offset_cache, CHUNK_SIZE, key, t);
        }
        // cache full
        else
        {
            ll victim = getVictim();
            // removeKey(victim, 1);
            assert(victim != -1);
            ll offset_cache = chunk_map[victim].offset_cache;
            chunk_map[victim].offset_cache = -1;
            if (chunk_map.count(key) == 0)

            {
                chunk item = {key, offset_cache};
                chunk_map[key] = item;
            }
            else
            {
                chunk_map[key].offset_cache = offset_cache;
            }
            chunk_map[key].dirty = 1;
            writeChunk(1, offset_cache, CHUNK_SIZE, key, t);
            writeBack(&chunk_map[victim], t);
        }
    }
}

// isCache�޸�Ϊint���ͣ�1��ʾ�����棬2��ʾд��������
void Sl::readChunk(int isCache, const long long &offset, const long long &size)
{
    assert(offset != -1);
    if (O_DIRECT_ON)
        odirectRead(isCache, offset, size);
    else
        normRead(isCache, offset, size);
}

// �޸�isCache//
void Sl::writeChunk(int isCache, const long long &offset, const long long &size, const ll &key, struct timeval t)
{
    assert(offset != -1);
    if (O_DIRECT_ON)
        odirectWrite(isCache, offset, size, key, t);
    else
        normWrite(isCache, offset, size);
}

void Sl::printChunk(chunk *arg)
{
    cout << "key: " << arg->key << endl;
    cout << "offset_cache: " << arg->offset_cache << endl;
    cout << "dirty: " << arg->dirty << endl;
}

void Sl::initFreeCache()
{
    for (long long i = 0; i < CACHE_SIZE; i++)
    {
        free_cache.push_back(i * CHUNK_SIZE);
    }
    // for(long long i=CACHE_SIZE/2;i< CACHE_SIZE;i++)
    // {
    //     free_cache_w.push_back(i * CHUNK_SIZE);
    // }
}

void Sl::readCache(const ll &offset_cache, int isReadCache)
{
    // printf("readCache\n");
    assert(offset_cache != -1);
    readChunk(isReadCache, offset_cache, CHUNK_SIZE);
}

void Sl::readDisk(const long long &key)
{
    // printf("readDisk\n");
    assert(key != -1);
    readChunk(0, key, CHUNK_SIZE);
}

void Sl::printChunkMap()
{
    cout << "chunk map:" << endl;
    map<long long, chunk>::iterator it;
    for (it = chunk_map.begin(); it != chunk_map.end(); it++)
        cout << it->second;
}

void Sl::coverageCache(chunk *arg, struct timeval t)
{
    // cout << "coverageCache" << endl;
    arg->dirty = 1;
    writeChunk(2, arg->offset_cache, CHUNK_SIZE, arg->key, t);
}

void Sl::isDirty(chunk *arg)
{
    arg->dirty = 1;
}

bool Sl::Dirty(chunk *arg)
{
    if (arg->dirty == 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void Sl::writeDisk(const long long &key, struct timeval t)
{

    backTime++;
    writeChunk(0, key, CHUNK_SIZE, key, t);
}

void Sl::statistic()
{
    // st.printStatistic();
    st.writeStatistic();
}

void Sl::remove(const ll &key, int isReadCache, struct timeval t)
{
    auto status = removeKey(key, 1);
    assert(status);
    // assert(isCached(key));
    accessWriteKey(key, false);
    // if (isReadCache == 1)
    // {
    //     free_cache.push_back(chunk_map[key].offset_cache);
    // }
    // else
    // {
    //     free_cache_w.push_back(chunk_map_w[key].offset_cache);
    // }
    // free_cache.push_back(chunk_map[key].offset_cache);

    if (!free_cache_w.empty())
    {

        ll offset_cache = chunk_map[key].offset_cache;
        free_cache.push_back(free_cache_w.back());
        chunk item = {key, offset_cache};
        chunk_map_w[key] = item;
        free_cache_w.pop_back();
        writeChunk(2, offset_cache, CHUNK_SIZE, key, t);
        chunk_map[key].offset_cache = -1;
    }
    else
    {
        ll victim = getWriteVictim();
        assert(victim != -1);
        ll offset_cache = chunk_map[key].offset_cache;
        free_cache.push_back(chunk_map_w[victim].offset_cache);
        chunk_map_w[victim].offset_cache = -1;
        if (chunk_map_w.count(key) == 0)
        {
            chunk item = {key, offset_cache};
            chunk_map_w[key] = item;
        }
        else
        {
            chunk_map_w[key].offset_cache = offset_cache;
        }
        chunk_map[key].offset_cache = -1;
        // chunk_map[key].dirty=1;
        writeChunk(2, offset_cache, CHUNK_SIZE, key, t);
        writeBack(&chunk_map_w[victim], t);
    }
}

int Sl::qLearn(struct timeval t)
{
    currentState.current_IO = 0;
    currentState.last_Action = 0;
    currentState.last_IO = 0;
    int action = 0;
    struct timeval current_t;
    gettimeofday(&current_t, NULL);
    currentTime = current_t.tv_sec * 1000000 + current_t.tv_usec;
    // record time interval
    currentState.c = currentTime;
    currentState.l = lastTime;
    currentState.ll = llTime;
    currentState.current_IO = calculate_timeInterval(currentTime, lastTime);
    currentState.last_IO = calculate_timeInterval(lastTime, llTime);
    currentState.last_Action = lastAction;
    if (turn != 0)
    {
        action = chooseAction(currentState);
    }
    else
    {
        action = 1;
    }

    return action;
}

void Sl::updateQtable(struct timeval t, int action)
{
    struct timeval t2;
    gettimeofday(&t2, NULL);
    long long deltaT = (t2.tv_sec - t.tv_sec) * 1000000 + (t2.tv_usec - t.tv_usec);
    // 奖励
    double award = returnAward(currentState, deltaT);
    // 更新Q表
    qTable[currentState.current_IO][currentState.last_IO][currentState.last_Action][action] += award * 0.1;
    // 记录当前动作作为下一状态的前一次执行动作
    lastAction = action;
    turn++;
    llTime = lastTime;
    lastTime = currentTime;
}

#endif /*_LRU_SIMULATORHPP_INCLUDED_*/
