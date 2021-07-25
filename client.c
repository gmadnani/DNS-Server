#include "server.h"

ClientNode Accept(Listener *listener)
{
    ClientNode client = (ClientNode)malloc(sizeof(Client));

    if (!client)
        return NULL;

    // Initialize variables.
    client->clientfd = -1;
    client->serverfd = -1;
    client->packetData = NULL;
    client->packetLength = 0;
    client->totalRead = 0;
    client->totalWritten = 0;

    struct sockaddr_in clientAddr = {0};
    socklen_t sockLength = 0;
    char str[64];

    // Accept the client socket.
    client->clientfd = accept(listener->socketFD, (struct sockaddr *)&clientAddr, &sockLength);

    if (client->clientfd < 0)
    {
        if (errno != EWOULDBLOCK)
        {
            ERR("Failed to accept client connection.");
        }
        goto fail;
    }

    // Set client socket as non-blocking
    int flags = fcntl(client->clientfd, F_GETFL, 0);
    flags |= O_NONBLOCK;

    if (fcntl(client->clientfd, F_SETFL, flags) < 0)
    {
        ERR("Failed to set client to non-blocking mode.");
        goto fail;
    }

    printf("Accepted client at %s.\n",
           inet_ntop(AF_INET, &clientAddr.sin_addr, str, sockLength));

    // Add to the new client to the tree
    LISTNode node = LIST_MakeNode(client->clientfd, client);

    if (!node)
    {
        ERR("Failed to insert client to tree.");
        goto fail;
    }

    listener->root = LIST_InsertNode(listener->root, node);

    return client;

fail:

    DeleteClient(client);
    return NULL;
}

void DeleteClient(ClientNode node)
{
    if (!node)
        return;

    free(node->packetData);

    if (node->clientfd >= 0)
        close(node->clientfd);

    free(node);
}

int ReadQuery(ClientNode clientNode)
{
    return ReadTCP(clientNode->clientfd, &clientNode->packetData,
                       &clientNode->packetLength, &clientNode->totalRead);
}

int WriteResponse(ClientNode clientNode)
{
    return WriteTCP(clientNode->clientfd, &clientNode->packetData,
                        &clientNode->packetLength, &clientNode->totalWritten);
}
