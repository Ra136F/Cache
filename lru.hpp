#ifndef LRU_HPP
#define LRU_HPP

#include "cache_policy.hpp"
#include <list>
#include <unordered_map>

namespace caches
{
/**
 * \brief LRU (Least Recently Used) cache policy
 * \details LRU policy in the case of replacement removes the least recently used element.
 * That is, in the case of replacement necessity, that cache policy returns a key that
 * has not been touched recently. For example, cache maximum size is 3 and 3 elements have
 * been added - `A`, `B`, `C`. Then the following actions were made:
 * ```
 * Cache placement order: A, B, C
 * Cache elements: A, B, C
 * # Cache access:
 * - A touched, B touched
 * # LRU element in the cache: C
 * # Cache access:
 * - B touched, C touched
 * # LRU element in the cache: A
 * # Put new element: D
 * # LRU replacement candidate: A
 *
 * Cache elements: B, C, D
 * ```
 * \tparam Key Type of a key a policy works with
 */
template <typename Key>
class LRU : public ICachePolicy<Key>
{
  public:
    using lru_iterator = typename std::list<Key>::iterator;

    LRU() = default;
    ~LRU() = default;

    void Insert(const Key &key) override
    {
        lru_queue.emplace_front(key);
        key_finder[key] = lru_queue.begin();
    }

    void Touch(const Key &key) override
    {
        // move the touched element at the beginning of the lru_queue
        lru_queue.splice(lru_queue.begin(), lru_queue, key_finder[key]);
    }

    void Erase(const Key &) noexcept override
    {
        // remove the least recently used element
        key_finder.erase(lru_queue.back());
        lru_queue.pop_back();
    }

    void EraseKey(const Key &key) noexcept override
    {
        // remove the key
        key_finder.erase(key);
        lru_queue.remove(key);
    }

    // return a key of a displacement candidate
    const Key &ReplCandidate() const noexcept override
    {
        return lru_queue.back();
    }

    bool compareHotness(const Key &key1, const Key &key2) 
    {
        auto it_key1 = std::find(lruQueue.begin(), lruQueue.end(), key1);
        auto it_key2 = std::find(lruQueue.begin(), lruQueue.end(), key2);

        
        if (std::distance(lru_queue.begin(), it_key1) < std::distance(lru_queue.begin(), it_key2))
        {
            // hotness: key1 > key2
            return 1;
        }
        else
        {
            // hotness: key1 < key2
            return 0;
        }
    }

    ll lruS1::getPrevoisKey(const Key &key)
    {
        auto it = key_finder.find(key);
        auto previousIt = std::prev(it->second);
        return *previousIt;
    }

  private:
    std::list<Key> lru_queue;
    std::unordered_map<Key, lru_iterator> key_finder;
};
} // namespace caches

#endif // LRU_HPP
