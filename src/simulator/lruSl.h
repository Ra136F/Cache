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
    lru_cache_t<long long, bool> cache_map{HALF_SIZE};
    //定义写缓存,CACHE_SIZE+write_CACHE_SIZE为一个固定值
    lru_cache_t<long long, bool> write_cache_map{HALF_SIZE};

    bool isCached(const ll &key);
    bool isWriteCached(const ll &key);
    bool removeKey(const ll &key,int isReadCache);
    void accessKey(const ll &key, const bool &isGet);
    void accessWriteKey(const ll &key, const bool &isGet);
    ll getVictim();
    ll getVictim2();
    ll getWriteVictim();
};

LruSl::LruSl():Sl(){
    st.caching_policy = "lru";
}

bool LruSl::isCached(const ll &key)
{
    return cache_map.Cached(key);
}

void LruSl::accessKey(const ll &key, const bool &isGet)
{

    cache_map.Put(key, 0);
    // cout<<"执行put后:"<<cache_map.Size()<<endl;
}

ll LruSl::getVictim(){
    return cache_map.getVictim();
}

ll LruSl::getVictim2(){
    return cache_map.getVictim2();
}


//判断写缓存中数据是否命中
bool LruSl::isWriteCached(const ll &key)
{
    return write_cache_map.Cached(key);
}

bool LruSl::removeKey(const ll &key,int isReadCache)
{
    if(isReadCache==1){
        return cache_map.Remove(key);
    } else{
        return write_cache_map.Remove(key);
    }
}

void LruSl::accessWriteKey(const ll &key, const bool &isGet)
{
    write_cache_map.Put(key, 0);
}

ll LruSl::getWriteVictim(){
    return write_cache_map.getVictim();
}




#endif /*_LRU_SIMULATOR_HPP_INCLUDED_*/
