#include "server.h"

Cache MakeCache()
{
    Cache cache;

    cache.timeSaved = 0;
    cache.answer.name = NULL;
    cache.answer.class = 0;
    cache.answer.ttl = 0;
    cache.answer.type = 0;
    cache.answer.rdlength = 0;
    cache.answer.rdata = NULL;

    cache.header.id = 0;
    cache.header.ancount = 0;
    cache.header.qdcount = 0;
    cache.header.nscount = 0;
    cache.header.arcount = 0;
    cache.header.tc = 0;
    cache.header.opcode = 0;
    cache.header.ra = 0;
    cache.header.rd = 0;
    cache.header.z = 0;

    return cache;
}

int IsCacheExpired(Cache *cache, time_t time)
{
    return (cache->timeSaved + cache->answer.ttl) < time;
}

Cache *FindCache(Cache cache[NUM_CACHE], const char *name)
{
    for (int i = 0; i < NUM_CACHE; i++)
    {
        if (!cache[i].answer.name) continue;

        if (strcmp(name, cache[i].answer.name) == 0)
            return &cache[i];
    }

    return NULL;
}

int UpdatePacket(Packet *packet, Cache *cache, time_t time)
{   
    // Save id
    uint16_t id = packet->header.id;

    // Clear packet
    ClearPacket(packet);

    // Update header
    memcpy(&packet->header, &cache->header, sizeof(Header));

    // Fix header
    packet->header.id = id;
    packet->header.qdcount = 0;
    packet->header.ancount = 1;
    packet->header.arcount = 0;
    packet->header.nscount = 0;

    packet->answers = (Record*)malloc(sizeof(Record));
    memcpy(packet->answers, &cache->answer, sizeof(Record));

    // Update TTL
    packet->answers[0].ttl = cache->answer.ttl + cache->timeSaved - time;

    // Copy pointers
    if (cache->answer.name) 
    {
        int len = strlen(cache->answer.name) + 1;
        packet->answers[0].name = (char*)malloc(len);
        memcpy(packet->answers[0].name, cache->answer.name, len);
    }

    if (cache->answer.rdata)
    {
        packet->answers[0].rdata = (uint8_t*)malloc(cache->answer.rdlength);
        memcpy(packet->answers[0].rdata, cache->answer.rdata, cache->answer.rdlength);
    }

    return 0;
}

Cache* FindExpiredCache(Cache cache[NUM_CACHE], time_t time)
{
    for (int i = 0; i < NUM_CACHE; i++)
    {
        if (IsCacheExpired(&cache[i], time))
            return &cache[i];
    }

    return NULL;
}

void UpdateCache(Cache* cache, Packet* packet, time_t time)
{
    // Clear cache first
    ClearCache(cache);

    // Copy contents
    memcpy(&cache->answer, &packet->answers[0], sizeof(Record));
    memcpy(&cache->header, &packet->header, sizeof(Header));

    // Copy pointers
    if (packet->answers[0].name)
    {
        int len = strlen(packet->answers[0].name) + 1;
        cache->answer.name = (char*)malloc(len);
        memcpy(cache->answer.name, packet->answers[0].name, len);
    }
    if (packet->answers[0].rdata)
    {
        cache->answer.rdata = (uint8_t*)malloc(packet->answers[0].rdlength);
        memcpy(cache->answer.rdata, packet->answers[0].rdata, packet->answers[0].rdlength);
    }

    cache->timeSaved = time;

    fprintf(stderr, "Updated cache: %s\n", cache->answer.name);
}

void ClearCache(Cache* cache)
{
    if (cache->answer.name)
    {
        free(cache->answer.name);
        cache->answer.name = NULL;
    }

    if (cache->answer.rdata)
    {
        free(cache->answer.rdata);
        cache->answer.rdata = NULL;
    }

    memset(&cache->header, 0, sizeof(cache->header));
    memset(&cache->answer, 0, sizeof(cache->answer));
}

Cache* FindCacheToEvict(Cache cache[NUM_CACHE])
{
    int j = 0;
    time_t minTime = INT64_MAX;

    for (int i = 0; i < NUM_CACHE; i++)
    {
        if (cache[i].timeSaved < minTime)
        {
            j = i;
            minTime = cache[i].timeSaved;
        }
    }

    return &cache[j];
}

Cache* FindEmptyCache(Cache cache[NUM_CACHE])
{
    for (int i = 0; i < NUM_CACHE; i++)
    {
        if (!cache[i].answer.name)
            return &cache[i];
    }

    return NULL;
}
