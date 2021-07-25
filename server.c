#include "server.h"

Packet MakePacket()
{
    Packet packet;

    // Set all fields to zero
    packet.header.qr = 0;
    packet.header.id = 0;
    packet.header.qdcount = 0;
    packet.header.ancount = 0;
    packet.header.nscount = 0;
    packet.header.arcount = 0;
    packet.header.opcode = 0;
    packet.header.ra = 0;
    packet.header.rd = 0;
    packet.header.rcode = 0;
    packet.header.tc = 0;
    packet.header.z = 0;

    // Set all pointers to NULL
    packet.questions = NULL;
    packet.answers = NULL;
    packet.authorities = NULL;
    packet.additional = NULL;

    return packet;
}



Listener OpenListener(char *serverIp, uint16_t serverPort)
{
    Listener list = {0};

    // Initialize resolver
    Resolver* resolver = (Resolver*)malloc(sizeof(Resolver));
    if (!resolver) return list;

    *resolver = InitResolver(serverIp, serverPort);
    list.resolver = resolver;

    FD_ZERO(&list.masterReadFS);
    FD_ZERO(&list.masterWriteFS);

    list.root = NULL;

    // Create the socket
    list.socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (list.socketFD < 0)
    {
        ERR("Failed to open socket of listener.");
        CloseListener(&list);
        return list;
    }

    puts("Server socket opened successfully.");

    // Prepare the sockaddr_in structure
    list.server.sin_family = AF_INET;
    list.server.sin_addr.s_addr = INADDR_ANY;
    list.server.sin_port = htons(LISTENER_PORT);

    // Allow port and interface to be reusable
    int enable = 1;
    if (setsockopt(list.socketFD, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        ERR("Failed to override socket settings.");
        CloseListener(&list);
        return list;
    }

    puts("Server socket address set as re-usable.");

    // Set socket as non-blocking
    int flags = fcntl(list.socketFD, F_GETFL, 0);
    flags |= O_NONBLOCK;
    if (fcntl(list.socketFD, F_SETFL, flags) < 0)
    {
        ERR("Failed to set socket as non-blocking.");
        CloseListener(&list);
        return list;
    }

    // Bind the server to the socket
    if (bind(list.socketFD, (struct sockaddr *)&list.server, sizeof(list.server)) < 0)
    {
        ERR("Failed to bind the socket to server.");
        CloseListener(&list);
        return list;
    }

    puts("Server bind success.");

    // Start listening
    if (listen(list.socketFD, 32) < 0)
    {
        ERR("Failed to listen socket.");
        CloseListener(&list);
        return list;
    }

    puts("Accepting connections.");

    // Add this socket to the reading list
    FD_SET(list.socketFD, &list.masterReadFS);
    list.maxFD = list.socketFD;

    return list;
}

void clientDelHandler(void *client)
{
    DeleteClient(client);
}

void CloseListener(Listener *listener)
{
    if (!listener)
        return;

    CloseResolver(listener->resolver);

    free(listener->resolver);

    // Close all the sockets
    close(listener->socketFD);

    // Close all client in the tree
    LIST_DeleteNodeRecur(listener->root, &clientDelHandler);
}

void RunListener(Listener *listener)
{
    // Execute one step of the server
    fd_set readfs = listener->masterReadFS;
    fd_set writefs = listener->masterWriteFS;

    Resolver *resolver = (Resolver*)listener->resolver;

    LISTNode node;
    ClientNode client;
    ResolverNode resNode;

    if (select(listener->maxFD + 1, &readfs, &writefs, NULL, NULL) < 0)
    {
        ERR("Select failed.");
        return;
    }

    // Check all the scoekts
    for (int sockfd = 0; sockfd <= listener->maxFD; sockfd++)
    {
        // Socket is available for reading
        if (FD_ISSET(sockfd, &readfs))
        {
            // Active socket is the listener. Accept new connections.
            if (sockfd == listener->socketFD)
            {
                ClientNode newClient;

                while ((newClient = Accept(listener)))
                {
                    // Add the client to the reading list
                    FD_SET(newClient->clientfd, &listener->masterReadFS);

                    // Update max fd
                    if (newClient->clientfd > listener->maxFD)
                        listener->maxFD = newClient->clientfd;
                }
            }
            // Read query from client
            else
            {
                node = LIST_FindNode(listener->root, sockfd);

                // Current socket is a client node
                if (node)
                {
                    client = (ClientNode)node->value;

                    // Read the query from the client
                    if (ReadQuery(client) < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            // If client is unreadble, its unlikely that we can write to it.
                            // End session with the client

                            listener->root = LIST_RemoveNode(node, sockfd);

                            DeleteClient(client);
                            LIST_DeleteNode(node);

                            FD_CLR(client->clientfd, &listener->masterReadFS);
                        }
                    }
                    else
                    {
                        // Connect to resolver for this query
                        ResolverNode resNode = ConnectResolver(resolver, client);

                        if (!resNode) // Failed to connect to resolver
                        {
                            // Reply to the client with the error.
                            FD_SET(sockfd, &listener->masterWriteFS);
                            FD_CLR(sockfd, &listener->masterReadFS);
                        }
                        else
                        {
                            // Remove client socket from reading list.
                            FD_CLR(sockfd, &listener->masterReadFS);

                            // Add resolver socket to writing list.
                            FD_SET(resNode->serverfd, &listener->masterWriteFS);

                            if (resNode->serverfd > listener->maxFD)
                                listener->maxFD = resNode->serverfd;
                        }
                    }

                    continue; // proceed to next
                }

                node = LIST_FindNode(resolver->root, sockfd);

                // Current socket is a resolver node
                if (node)
                {
                    // Read reply of resolver
                    resNode = (ResolverNode)node->value;

                    if (ResolveResponse(resNode, resolver) < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            // Remove resolver socket from reading list.
                            FD_CLR(resNode->serverfd, &listener->masterReadFS);

                            // Add client socket to writing list.
                            FD_SET(resNode->client->clientfd, &listener->masterWriteFS);
                        }
                    }
                    else
                    {
                        // Remove resolver socket from reading list.
                        FD_CLR(resNode->serverfd, &listener->masterReadFS);

                        // Add client socket to writing list.
                        FD_SET(resNode->client->clientfd, &listener->masterWriteFS);

                        // Remove resolver from the tree
                        resolver->root = LIST_RemoveNode(resolver->root, resNode->serverfd);

                        // Delete resolver
                        LIST_DeleteNode(node);
                        DeleteResolver(resNode);
                    }
                }
            }
        }
        // Socket is available for writing
        else if (FD_ISSET(sockfd, &writefs))
        {
            node = LIST_FindNode(listener->root, sockfd);

            // Current socket is a client node
            if (node)
            {
                client = (ClientNode)node->value;

                // Write packet to client
                if (WriteResponse(client) < 0)
                {
                    if (errno != EWOULDBLOCK)
                    {
                        ERR("Failed to write response to client.");

                        listener->root = LIST_RemoveNode(listener->root, sockfd);

                        LIST_DeleteNode(node);
                        DeleteClient(client);

                        FD_CLR(sockfd, &listener->masterWriteFS);
                    }
                }
                // Sent reply to client
                else
                {
                    WARN("Sent reply to client.");

                    listener->root = LIST_RemoveNode(listener->root, sockfd);

                    LIST_DeleteNode(node);
                    DeleteClient(client);

                    FD_CLR(sockfd, &listener->masterWriteFS);
                }

                continue; // proceed to next
            }

            node = LIST_FindNode(resolver->root, sockfd);

            // Current socket is a resolver node
            if (node)
            {
                // Forward query to resolver
                resNode = (ResolverNode)node->value;

                if (ResolveQuery(resNode) < 0)
                {
                    if (errno != EWOULDBLOCK)
                    {
                        ERR("Failed to forward query to resolver.");

                        // The error response is expected to be written to the client's packet data.
                        // Add the client to the writing list.
                        FD_CLR(resNode->serverfd, &listener->masterWriteFS);
                        FD_SET(resNode->client->clientfd, &listener->masterWriteFS);
                    }
                }
                else
                {
                    // Move server from writing to reading list.
                    FD_CLR(resNode->serverfd, &listener->masterWriteFS);
                    FD_SET(resNode->serverfd, &listener->masterReadFS);
                }
            }
        }
    }
}
