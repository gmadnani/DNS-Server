#ifndef RESOLVER_H_
#define RESOLVER_H_

#include "server.h"

typedef struct _Resolve
{
    int serverfd; // key
    ClientNode client;
    Packet packet;
} Resolve;

typedef Resolve* ResolverNode;

typedef struct _Resolver
{
    struct sockaddr_in addr;
    LISTNode root;
    Cache cache[NUM_CACHE];
} Resolver;

/**
 * @brief Initializes a  resolver object
 * 
 * @param serverIp pointer to a string of the server Ip
 * @return Resolver initialized  resolver object
 */
Resolver InitResolver(char *serverIp, uint16_t serverPort);

/**
 * @brief Clears memory used by the resolver
 * 
 * @param resolver pointer to the resolver object
 */
void CloseResolver(Resolver *resolver);

/**
 * @brief Resolves the packet using the  resolver server
 * 
 * @param packet pointer to the  packet object
 * @param resolver pointer to the  resolver object
 */
void ResolvePacket(Packet *packet, Resolver *resolver);

/**
 * @brief Make a connection to the resolver.
 * 
 * @param resolver pointer to the dns resolver
 * @param client client node associated to this resolver node
 * @return ResolverNode the resolver node created
 */
ResolverNode ConnectResolver(Resolver *resolver, ClientNode client);

/**
 * @brief Delete the resovler object
 * 
 * @param node node to delete
 */
void DeleteResolver(ResolverNode node);

/**
 * @brief Forward the query to the resolver.
 * 
 * @param node pointer to the resolver node
 * @return int error code
 */
int ResolveQuery(ResolverNode node);

/**
 * @brief Resolve response of server.
 * 
 * @param node pointer to the resolver node
 * @param resolver pointer to resovler object
 * @return int error code
 */
int ResolveResponse(ResolverNode node, Resolver* resolver);

#endif // RESOLVER_H_
