#ifndef CLIENT_H_
#define CLIENT_H_

#include "server.h"

/**
 * @brief  Client data structure.
 * 
 */
typedef struct _Client
{
    int clientfd;           // socket fd of the client
    int serverfd;           // socket fd of the resolver
    uint8_t* packetData;    // packet data buffer
    uint16_t packetLength;       // packet length

    uint16_t totalRead;
    uint16_t totalWritten;

    struct _Client* left;     // left node
    struct _Client* right;    // right node
} Client;

typedef Client* ClientNode;

/**
 * @brief Accepts a client connection.
 * 
 * @param Listener* pointer to listener object
 * @return ClientNode client node
 */
ClientNode Accept(Listener* listener);

/**
 * @brief Deletes a client.
 * 
 * @param node the  client node to delete
 */
void DeleteClient(ClientNode node);

/**
 * @brief Reads the query from the client
 * 
 * @param clientNode the client node
 * @return int -1 for error
 */
int ReadQuery(ClientNode clientNode);

/**
 * @brief Writes the query to the client
 * 
 * @param clientNode the client node
 * @return int -1 for error
 */
int WriteResponse(ClientNode clientNode);

#endif // CLIENT_H_
