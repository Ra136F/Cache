#ifndef _RANDOM_SIMULATOR_HPP_INCLUDED_
#define _RANDOM_SIMULATOR_HPP_INCLUDED_
#include "../cache/cache.hpp"
#include "../cache/random.h"
#include "sl.h"
using namespace std;

class RandomSl : public Sl
{
public:
    RandomSl();
private:
    RANDOM cache_map;
    RANDOM cache_map_w;
    bool isCached(const ll &key);
    void accessKey(const ll &key, const bool &isGet);
    void accessKey_W(const ll &key, const bool &isGet);
    ll getVictim();
};

RandomSl::RandomSl():Sl(){
    st.caching_policy = "random";
}


bool RandomSl::isCached(const ll &key)
{
    return cache_map.Cached(key);
}

void RandomSl::accessKey(const ll &key, const bool &isGet)
{
    cache_map.Touch(key);
}

void RandomSl::accessKey_W(const ll &key, const bool &isGet)
{
    cache_map_w.Touch(key);
}

ll RandomSl::getVictim(){
    return cache_map.getVictim();
}

#endif /*_RANDOM_SIMULATOR_HPP_INCLUDED_*/
