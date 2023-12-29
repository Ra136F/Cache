#ifndef _LRU_SIMULATORHPP_INCLUDED_
#define _LRU_SIMULATORHPP_INCLUDED_

#include <iostream>
#include <fstream>
#include <streambuf>
#include <fstream>
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

using namespace std;

class Sl
{
public:
    void test();
    void statistic();

protected:
    int fd_cache, fd_disk,fd_cache_w;

    long long curKey;
    vector<long long> free_cache;
    vector<long long> free_cache_w;
    map<long long, chunk> chunk_map;
    Statistic st;

    Sl();
    ~Sl();
    void init();
    void initFreeCache();
    void initFile();
    void closeFile();

    bool readItem(vector<ll> &keys);
    bool writeItem(vector<ll> &keys);
    virtual void writeCache(const ll &key,int isReadCache);

    void writeBack(chunk *arg);

    void readDisk(const long long &key); // disk->cache disk->buffer
    void writeDisk(const long long &key);

    void readCache(const ll &offset_cache,int isReadCache); // cache->buffer
    void coverageCache(chunk *arg);

    void normRead(bool isCache, const long long &offset, const long long &size);
    void normWrite(bool isCache, const long long &offset, const long long &size);
    void odirectRead(int isCache, const long long &offset, const long long &size);
    void odirectWrite(int isCache, const long long &offset, const long long &size);

    void readChunk(int isCache, const long long &offset, const long long &size);
    void writeChunk(int isCache, const long long &offset, const long long &size);

    void printChunk(chunk *arg);
    void printChunkMap();
    void printFreeCache();

    bool isWriteCache(); 
    virtual  bool isWriteCached(const ll &key) =0 ;
    void checkFile(fstream &file);

    virtual bool isCached(const ll &key) = 0;
    virtual void accessKey(const ll &key, const bool &isGet) = 0;
    virtual void accessWriteKey(const ll &key, const bool &isGet) = 0 ;
    virtual bool removeKey(const ll &key,int isReadCache)=0;
    virtual ll getVictim() = 0;
    virtual ll getWriteVictim()=0 ;
};

bool Sl::readItem(vector<ll> &keys)
{
    // 
    int isReadCache=0;
    bool isTraceHit = true;

    st.read_nums += keys.size();
    // cache hit
    for (int i = 0; i < keys.size(); i++)
    {
       
        if (isCached(keys[i]))
        {
            // ͳ��������
            isReadCache=1;
            st.read_hit_nums += 1;
          
            accessKey(keys[i], true); // [lirs] cache_map.Get(keys[i]);
            readCache(chunk_map[keys[i]].offset_cache,isReadCache);
           
            keys[i] = -1;
        }
        else if (isWriteCached(keys[i]))
        {
            isReadCache=2;
            accessWriteKey(keys[i], true);
          
            readCache(chunk_map[keys[i]].offset_cache,isReadCache);
            keys[i] = -1;
        }
        
    }
    // cache miss
    for (int i = 0; i < keys.size(); i++)
    {
        if (keys[i] != -1)
        {
            isTraceHit = false;
           
            accessKey(keys[i], false); // [lirs] cache_map.Add(keys[i],0);
            readDisk(keys[i]);
            //д����
            writeCache(keys[i],1);
        }
    }
    return isTraceHit;
}

bool Sl::writeItem(vector<ll> &keys)
{
    bool isTraceHit = true;
    st.write_nums += keys.size();
    // cache hit
    for (int i = 0; i < keys.size(); i++)
    {
        if (isCached(keys[i]))
        {
            //���������У������ƶ���д����
            //����Ӧ��key���µ�д������
            // accessWriteKey(keys[i], true)
            //�Ӷ�����LRU�������Ӧkey
            // keys[i] = -1;


            //
            st.write_hit_nums += 1;
            accessKey(keys[i], false); // [lirs] cache_map.Add(keys[i],0);
            // 淘汰
            // coverageCache(&chunk_map[keys[i]]);
            keys[i] = -1;
        }
        else if (isWriteCached(keys[i]))
        {
            st.write_hit_nums += 1;
            accessWriteKey(keys[i], true);
            //�޸�readCache��readChunk
            // readCache(chunk_map[keys[i]].offset_cache);
            coverageCache(&chunk_map[keys[i]]);
            keys[i] = -1;
        }
    }
    // cache miss
    for (int i = 0; i < keys.size(); i++)
    {
        if (keys[i] != -1)
        {
            isTraceHit = false;


            accessWriteKey(keys[i], false); 
            // accessKey(keys[i], false); // [lirs] cache_map.Add(keys[i],0);

            writeCache(keys[i],2);
        }
    }
    return isTraceHit;
}

//�������isReadCache��д�������?1��д��д����2
void Sl::writeCache(const ll &key,int isReadCache)
{
    // cout << "writeCache: ";
    // ���д��?
    if (!isWriteCache())
        return;

    // cache not full
    if(isReadCache==1){
        if (!free_cache.empty())
        {
            // cout << "cache not full" << endl;
            ll offset_cache = free_cache.back();
            chunk item = {key, offset_cache};
            chunk_map[key] = item;
            free_cache.pop_back();
            writeChunk(2, offset_cache, CHUNK_SIZE);
        }
        // cache full
        else
        {
            ll victim=getVictim();
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
            writeChunk(1, offset_cache, CHUNK_SIZE);
        }

        // cout << "cache full" << endl;
        // ll victim = getVictim(); // [lirs] ll victim = cache_map.getCurVictim();
        // assert(victim != -1);
        // ll offset_cache = chunk_map[victim].offset_cache;
        // chunk_map[victim].offset_cache = -1;
        // if (chunk_map.count(key) == 0)
        // {
        //     chunk item = {key, offset_cache};
        //     chunk_map[key] = item;
        // }
        // else
        // {
        //     chunk_map[key].offset_cache = offset_cache;
        // }
        // //����д���д�����޸Ĳ���true����Ϊint����
        // writeChunk(true, offset_cache, CHUNK_SIZE);
        // writeBack(&chunk_map[victim]);
    }else if (isReadCache==2) {
        if (!free_cache_w.empty())
        {
            // cout << "cache not full" << endl;
            ll offset_cache = free_cache_w.back();
            chunk item = {key, offset_cache};
            chunk_map[key] = item;
            free_cache_w.pop_back();
            writeChunk(2, offset_cache, CHUNK_SIZE);
        }
        // cache full
        else
        {
            ll victim=getWriteVictim();
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
            writeChunk(2, offset_cache, CHUNK_SIZE);
            writeBack(&chunk_map[victim]);
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
            isTraceHit = readItem(keys);
            break;
        case 1:
            isTraceHit = writeItem(keys);
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
    random_device seed;                           // Ӳ���������������?
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
fd_cache_w = open(CACHE_PATH, O_RDWR | O_DIRECT, 0664);
    assert(fd_cache_w>=0);
    fd_cache = open(CACHE_PATH, O_RDWR | O_DIRECT, 0664);
    assert(fd_cache >= 0);

    //д����
    // write_fd_cache = open(WRITE_CACHE_PATH, O_RDWR | O_DIRECT, 0664);
    // assert(write_fd_cache >= 0);

    fd_disk = open(DISK_PATH, O_RDWR | O_DIRECT, 0664);
    assert(fd_disk >= 0);
}

void Sl::closeFile()
{
    close(fd_cache);
    // close(write_fd_cache)
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

// д�أ��ж����ݿ��Ƿ�Ϊ��
void Sl::writeBack(chunk *arg)
{
    if (arg->dirty == 1)
    {
        arg->dirty = 0;
        writeDisk(arg->key);
    }
    // ��飺Q-Learning����SMR�����ж��Ƿ�д��
    // SMRæ����һ��ֵ��ѡ��ɾ���?
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
    if (isCache==1)
        fd = fd_cache;
    else if(isCache==2)
        fd = fd_cache_w;
    else
        fd = fd_disk;
    assert(fd >= 0);

    char *buffer = nullptr;
    int res = posix_memalign((void **)&buffer, CHUNK_SIZE, size);
    assert(res == 0);



    //��¼ʱ�䡢��������Ϣ
    res = pread64(fd, buffer, size, offset);



    // printf("odirectRead: %d\n",res);
    assert(res == size);

    free(buffer);
    // close(fd);
}

//isCache�޸�Ϊint���ͣ�1��ʾ�����棬2��ʾд��������
void Sl::odirectWrite(int isCache, const long long &offset, const long long &size)
{
    assert(offset != -1);
    int fd = -1;
    if (isCache==1)
        fd = fd_cache;
    else if(isCache==2)
        fd = fd_cache_w;
    else
        fd = fd_disk;
    // cout<<filePath<<endl;
    assert(fd >= 0);
    char *buffer = nullptr;


    // assert(offset != -1);
    // int fd = -1;
    // if (isCache)
    //     fd = fd_cache_w;
    // else
    //     fd = fd_disk;
    // // cout<<filePath<<endl;
    // assert(fd >= 0);
    // char *buffer = nullptr;

    //����ȫ�ֱ�������¼֮ǰ�Ķ�������һ��I/O��ʱ�䡢��ǰI/Oʱ�䡢���ϴ�I/Oʱ�䡢�ִ�
    //���SMR������д�룬����read������������δ���е���������Ȼ��break
    // else
    // {
    //     // cout << "cache full" << endl;
    //     ll victim = getVictim(); // [lirs] ll victim = cache_map.getCurVictim();
    //     assert(victim != -1);
    //     ll offset_cache = chunk_map[victim].offset_cache;
    //     chunk_map[victim].offset_cache = -1;
    //     if (chunk_map.count(key) == 0)
    //     {
    //         chunk item = {key, offset_cache};
    //         chunk_map[key] = item;
    //     }
    //     else
    //     {
    //         chunk_map[key].offset_cache = offset_cache;
    //     }
    //     writeChunk(true, offset_cache, CHUNK_SIZE);
    //     writeBack(&chunk_map[victim]);
    // }

    //д��
    int res = posix_memalign((void **)&buffer, CHUNK_SIZE, size);

    assert(res == 0);

    strcpy(buffer, "Ram15978");
    res = pwrite64(fd, buffer, size, offset);
    // cout<<"odirectWrite: res: "<<res<<", fd: "<<fd<<", buffer: "<<buffer<<", size: "<<size<<", offset: "<<offset<<endl;
    // printf("odirectWrite: %d\n",res);
    assert(res == size);

    free(buffer);
    // close(fd);
}

//isCache�޸�Ϊint���ͣ�1��ʾ�����棬2��ʾд��������
void Sl::readChunk(int isCache, const long long &offset, const long long &size)
{
    assert(offset != -1);
    if (O_DIRECT_ON)
        odirectRead(isCache, offset, size);
    else
        normRead(isCache, offset, size);
}

//�޸�isCache//
void Sl::writeChunk(int isCache, const long long &offset, const long long &size)
{
    assert(offset != -1);
    if (O_DIRECT_ON)
        odirectWrite(isCache, offset, size);
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
        free_cache_w.push_back(i * CHUNK_SIZE);
        // cout<<i * CHUNK_SIZE<<" has pushed in to free cache"<<endl;
    }
}

void Sl::readCache(const ll &offset_cache,int isReadCache)
{
    // printf("readCache\n");
    assert(offset_cache != -1);
    // readChunk(true, offset_cache, CHUNK_SIZE);
    //����������ʱcached=1�� д��������ʱcached=2��readCache��һ������isReadCache��readItem������
    if (isReadCache==1) {
        readChunk(1, offset_cache, CHUNK_SIZE);
    }
    else if(isReadCache==2){
        readChunk(2, offset_cache, CHUNK_SIZE);
    }
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

void Sl::coverageCache(chunk *arg)
{
    // cout << "coverageCache" << endl;
    arg->dirty = 1;
    writeChunk(true, arg->offset_cache, CHUNK_SIZE);
}
// void Sl::coverageWriteCache(chunk *arg)
// {
//     // cout << "coverageCache" << endl;
//     arg->dirty = 1;
//     writeChunk(2, arg->offset_cache, CHUNK_SIZE);
// }

void Sl::writeDisk(const long long &key)
{
    // cout << "writeDisk" << endl;
    writeChunk(false, key, CHUNK_SIZE);
}

void Sl::statistic()
{
    // st.printStatistic();
    st.writeStatistic();
}

#endif /*_LRU_SIMULATORHPP_INCLUDED_*/
