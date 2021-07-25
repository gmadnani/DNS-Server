#ifndef CACHE_H_
#define CACHE_H_

#include "server.h"

#define NUM_CACHE   5

typedef struct _Cache {
    time_t timeSaved;
    Record answer;
    Header header;
} Cache;

/**
 * @brief Initialize cache.
 * 
 * @return Cache cache object
 */
Cache MakeCache();

/**
 * @brief Check if cache the cache has expred
 * 
 * @param cache pointer to the cache object
 * @param time time of the request
 * @return int returns 1 if true, 0 otherwise
 */
int IsCacheExpired(Cache* cache, time_t time);

/**
 * @brief Check if the name is the same as the cache data stored.
 * 
 * @param cache cache array
 * @param name name of the request
 * @return Cache* pointer to the cache found
 */
Cache* FindCache(Cache cache[NUM_CACHE], const char* name);

/**
 * @brief Update the packet with the cache.
 * 
 * @param packet pointer to the packet
 * @param cache pointer to the cache
 * @param time time of request
 * @return int error code: -1 for error
 */
int UpdatePacket(Packet* packet, Cache* cache, time_t time);

/**
 * @brief Finds an expired cache to replace.
 * 
 * @param cache cache array
 * @param time time of request
 * @return Cache* pointer to the expired cache
 */
Cache* FindExpiredCache(Cache cache[NUM_CACHE], time_t time);

/**
 * @brief Find an empty cache
 * 
 * @param cache cache array
 * @return Cache* pointer to the empty cache
 */
Cache* FindEmptyCache(Cache cache[NUM_CACHE]);

/**
 * @brief Update the cache.
 * 
 * @param cache pointer to the cache
 * @param packet pointer to the packet
 * @param time current time
 */
void UpdateCache(Cache* cache, Packet* packet, time_t time);

/**
 * @brief Clear the contents of the cache.
 * 
 * @param cache pointer to the cache
 */
void ClearCache(Cache* cache);

/**
 * @brief Find a cache to evict. Use FIFO.
 * 
 * @param cache cache array
 * @return Cache* pointer of cache to delete
 */
Cache* FindCacheToEvict(Cache cache[NUM_CACHE]);

#endif
