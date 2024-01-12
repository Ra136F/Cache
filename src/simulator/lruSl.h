#ifndef _LRU_SIMULATOR_HPP_INCLUDED_
#define _LRU_SIMULATOR_HPP_INCLUDED_
#include "../cache/cache.hpp"
#include "../cache/lru.hpp"
#include "sl.h"
using namespace std;

class LruSl : public Sl
{
public:
    LruSl();
private:
    template <typename Key, typename Value>
    using lru_cache_t = typename caches::fixed_sized_cache<Key, Value, caches::LRU>;
    //定义读缓存

    //read/write mixed cache
    lru_cache_t<long long, bool> ssdCache_map{CACHE_SIZE};
    // bigger cache
    lru_cache_t<long long, bool> hotCache_map{P_SIZE};

    bool isCached(const ll &key);
    bool isWriteCached(const ll &key);
    bool removeKey(const ll &key,int isReadCache);
    void accessKey(const ll &key, const bool &isGet);
    void accessWriteKey(const ll &key, const bool &isGet);
    bool compareHotCache(const ll &key1, const ll &key2);
    void accessHotCacheKey(const ll &key, const bool &isGet);
    void accessSSDCacheKey(const ll &key, const bool &isGet);
    bool isSSDCached(const ll &key);
    ll getPrevoisKey(const ll &key);
    ll getVictim();
    ll getVictim2();
    ll getWriteVictim();
    ll getSSDVictim();
};

LruSl::LruSl():Sl(){
    st.caching_policy = "lru";
}






bool LruSl::removeKey(const ll &key,int isReadCache)
{
    if(isReadCache==1){
        return ssdCache_map.Remove(key);
    } else{
        return ssdCache_map.Remove(key);
    }
}



bool LruSl::compareHotCache(const ll &key1, const ll &key2)
{
    return hotCache_map.compareHotness(key1, key2);
}

void LruSl::accessSSDCacheKey(const ll &key, const bool &isGet)
{

    ssdCache_map.Put(key, 0);
    // cout<<"执行put后:"<<cache_map.Size()<<endl;
}

void LruSl::accessHotCacheKey(const ll &key, const bool &isGet)
{

    hotCache_map.Put(key, 0);
    // cout<<"执行put后:"<<cache_map.Size()<<endl;
}

bool LruSl::isSSDCached(const ll &key)
{
    return ssdCache_map.Cached(key);
}


ll LruSl::getSSDVictim(){
    return ssdCache_map.getVictim2();
}

ll LruSl::getPrevoisKey(const ll &key) {
    return ssdCache_map.getPrevoisKey(key);
}




#endif /*_LRU_SIMULATOR_HPP_INCLUDED_*/
