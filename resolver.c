#include "server.h"

Resolver InitResolver(char *serverIp, uint16_t serverPort)
{
    Resolver resolver;

    resolver.addr.sin_addr.s_addr = inet_addr(serverIp);
    resolver.addr.sin_family = AF_INET;
    resolver.addr.sin_port = htons(serverPort);
    resolver.root = NULL;

    for (int i = 0; i < NUM_CACHE; i++)
    {
        resolver.cache[i] = MakeCache();
    }

    return resolver;
}

void ResolvePacket(Packet *packet, Resolver *resolver)
{
    if (packet->header.qr != HEADER_QR_QUERY)
    {
        WARN("Received a non-query request.");

        packet->header.qr = HEADER_QR_RESPONSE;
        packet->header.rcode = HEADER_RCODE_SERVFAIL;

        return;
    }

    if (packet->header.qdcount == 0)
    {
        WARN(" Query is empty.");

        packet->header.qr = HEADER_QR_RESPONSE;
        packet->header.rcode = HEADER_RCODE_SERVFAIL;

        return;
    }

    // Log  Query
    printf("Client requested %s\n", packet->questions[0].qname);
    PrintLog(&dnsLog, "requested %s", packet->questions[0].qname);

    // Check if query is type AAAA
    if (packet->questions[0].qtype != RECORD_TYPE_AAAA)
    {
        PrintLog(&dnsLog, "unimplemented request");
        WARN("Request type is not implemented.");

        // Modify the query packet
        packet->header.rcode = HEADER_RCODE_NOTIMPL;
        packet->header.qr = HEADER_QR_RESPONSE;
        packet->header.opcode = HEADER_OPCODE_QUERY;
        packet->header.ra = 0;
        packet->header.rd = 0;

        return;
    }

    // Forward query to remote server
    int sockfd;
    int totalRead, nread;
    uint8_t packetData[1024];
    uint16_t packetLength;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        ERR("Failed to open socket for resolver.");

        goto fail;
    }

    puts("Opened socket for resolver.");

    if (connect(sockfd, (struct sockaddr *)&resolver->addr, sizeof(resolver->addr)) < 0)
    {
        ERR("Failed to connect to resolver.");

        goto fail;
    }

    packetLength = WritePacket(packet, packetData, 1024);
    if (packetLength < 0)
    {
        ERR("Failed to write packet.");

        goto fail;
    }

    int resp = htons(packetLength);
    if (write(sockfd, &resp, 2) != 2)
    {
        ERR("Failed to send packet size.");

        goto fail;
    }

    puts("Sent packet size to resolver.");

    if (write(sockfd, packetData, packetLength) != packetLength)
    {
        ERR("Failed to send packet data.");

        goto fail;
    }

    puts("Forwarded query to resolver.");

    // Read the reply packet size
    if (read(sockfd, &packetLength, 2) != 2)
    {
        ERR("Failed to read packet size.");

        goto fail;
    }

    packetLength = ntohs(packetLength);

    // Read the packet data in a loop
    for (totalRead = 0; totalRead < packetLength; totalRead += nread)
    {
        nread = read(sockfd, packetData + totalRead, 1024 - totalRead);
    }

    puts("Received response from resolver.");

    close(sockfd);
    ClearPacket(packet); // clear packet before reading

    // Process reply from server
    if (ParsePacket(packet, packetData, 1024, 0) < 0)
    {
        ERR("Invalid dns packet received.");
        goto fail;
    }

    if (packet->header.qr != HEADER_QR_RESPONSE)
    {
        WARN("Remote server sent a non-response packet.");
        goto fail;
    }

    return;

fail:
    packet->header.qr = HEADER_QR_RESPONSE;
    packet->header.rcode = HEADER_RCODE_SERVFAIL;
}

ResolverNode ConnectResolver(Resolver *resolver, ClientNode client)
{
    ResolverNode node = (ResolverNode)malloc(sizeof(Resolve));

    if (!node)
        return NULL;

    // Initialize values of the node.
    node->client = client;
    node->packet = MakePacket();

    // Read the packet data from the client node.
    // It is expected that the packet has been read prior to calling of this function.
    if (ParsePacket(&node->packet, client->packetData, client->packetLength, 0) < 0)
    {
        ERR("Invalid query.");

        // Reply a form error to the client.
        node->packet.header.qr = HEADER_QR_RESPONSE;
        node->packet.header.rcode = HEADER_RCODE_FORMERR;

        goto fail;
    }

    // Check if there is a question in the query.
    if (!node->packet.header.qdcount)
    {
        ERR("Query is empty.");

        // Reply a form error to the client.
        node->packet.header.qr = HEADER_QR_RESPONSE;
        node->packet.header.rcode = HEADER_RCODE_FORMERR;

        goto fail;
    }

    // Log the request
    PrintLog(&dnsLog, "requested %s", node->packet.questions[0].qname);

    // Check if the query is an AAAA query.
    if (node->packet.questions[0].qtype != RECORD_TYPE_AAAA)
    {
        WARN(" Query is not an AAAA type question.");

        PrintLog(&dnsLog, "unimplemented request");

        // Reply a not imlemented error to the client.
        node->packet.header.qr = HEADER_QR_RESPONSE;
        node->packet.header.rcode = HEADER_RCODE_NOTIMPL;
        node->packet.header.ra = 1;

        goto fail;
    }

    // Check cache here
    Cache *cache = FindCache(resolver->cache, node->packet.questions[0].qname);
    if (cache)
    {
        WARN("Request is in cache.");

        // Prepare response
        time_t curTime;
        time(&curTime);

        if (!IsCacheExpired(cache, curTime))
        {
            // Copy data to the packet
            if (UpdatePacket(&node->packet, cache, curTime) == 0)
            {
                // Log the domain
                char timeBuff[TS_BUFF_SIZE];
                WriteTimestamp(timeBuff, cache->timeSaved + cache->answer.ttl);

                PrintLog(&dnsLog, "%s expires at %s", cache->answer.name, timeBuff);

                // Log the answer
                char ipBuff[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, node->packet.answers[0].rdata, ipBuff, INET6_ADDRSTRLEN);

                PrintLog(&dnsLog, "%s is at %s", node->packet.answers[0].name, ipBuff);

                // Display response of the resolver
                fprintf(stderr, "%s is at %s\n", node->packet.answers[0].name, ipBuff);
            }

            goto fail;
        }
        else
        {
            WARN("Cache expired.");
        }
    }

    // Create server socket
    node->serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (node->serverfd < 0)
    {
        ERR("Failed to open resolver socket.");

        node->packet.header.qr = HEADER_QR_RESPONSE;
        node->packet.header.rcode = HEADER_RCODE_SERVFAIL;
        goto fail;
    }

    // Make this socket non blocking.
    int flags = fcntl(node->serverfd, F_GETFL, 0);
    flags |= O_NONBLOCK;

    if (fcntl(node->serverfd, F_SETFL, flags) < 0)
    {
        ERR("Failed to change settings of resolver socket.");

        node->packet.header.qr = HEADER_QR_RESPONSE;
        node->packet.header.rcode = HEADER_RCODE_SERVFAIL;
        goto fail;
    }

    // Connect to the resolver server
    if (connect(node->serverfd, (struct sockaddr *)&resolver->addr, sizeof(resolver->addr)) < 0)
    {
        if (errno != EINPROGRESS)
        {
            ERR("Failed to connect to resolver.");

            node->packet.header.qr = HEADER_QR_RESPONSE;
            node->packet.header.rcode = HEADER_RCODE_SERVFAIL;
            goto fail;
        }
    }

    client->serverfd = node->serverfd;
    client->totalWritten = 0;

    // Add to tree
    // Insert to the tree for forwarding of the query.
    LISTNode bnode = LIST_MakeNode(node->serverfd, node);

    if (!bnode)
    {
        ERR("Failed to add the resolver node to tree.");
        goto fail;
    }

    resolver->root = LIST_InsertNode(resolver->root, bnode);

    WARN("Successfully connected to resolver.");

    return node;

fail:
    node->packet.header.ra = 1;
    UpdatePacketData(node);
    ClearPacket(&node->packet);
    free(node);
    return NULL;
}

void DeleteResolver(ResolverNode node)
{
    ClearPacket(&node->packet);

    if (node->serverfd >= 0)
        close(node->serverfd);

    free(node);
}

int ResolveQuery(ResolverNode node)
{
    if (WriteTCP(node->serverfd, &node->client->packetData,
                     &node->client->packetLength, &node->client->totalWritten) < 0)
    {
        if (errno != EWOULDBLOCK)
        {
            // This is really an error
            node->packet.header.qr = HEADER_QR_RESPONSE;
            node->packet.header.rcode = HEADER_RCODE_SERVFAIL;

            UpdatePacketData(node);
        }
        return -1;
    }

    // Prepare client for receiving data
    node->client->packetLength = 0;
    free(node->client->packetData);
    node->client->packetData = NULL;

    WARN("Forwarded client query to resolver.");

    // Done
    return 0;
}

int ResolveResponse(ResolverNode node, Resolver *resolver)
{
    if (ReadTCP(node->serverfd, &node->client->packetData,
                    &node->client->packetLength, &node->client->totalRead) < 0)
    {
        if (errno != EWOULDBLOCK)
            goto fail;

        return -1;
    }

    // Clear previously read packet.
    ClearPacket(&node->packet);

    // Convert the resolver response to packet object
    if (ParsePacket(&node->packet, node->client->packetData, node->client->packetLength, 0) < 0)
    {
        ERR("Invalid resolver response.");
        goto fail;
    }

    // Check answer
    if (node->packet.header.ancount == 0)
    {
        WARN("No answer received.");
        goto update;
    }

    if (node->packet.answers[0].type != RECORD_TYPE_AAAA)
    {
        WARN("Answer is not type AAAA.");
        node->packet.header.qr = HEADER_QR_RESPONSE;
        node->packet.header.rcode = HEADER_RCODE_NOTIMPL;
        node->packet.header.ra = 1;
        goto update;
    }

    WARN("Received response from the resolver.");

    // Save to cache
    time_t curTime;
    time(&curTime);
    int emptyCache = 0;
    Cache *cacheToReplace = FindCache(resolver->cache, node->packet.answers[0].name);

    if (!cacheToReplace)
    {
        cacheToReplace = FindEmptyCache(resolver->cache);
        emptyCache = 1;
    }

    if (!cacheToReplace)
    {
        cacheToReplace = FindExpiredCache(resolver->cache, curTime);
        emptyCache = 0;
    }

    if (!cacheToReplace)
    {
        // Evict one
        cacheToReplace = FindCacheToEvict(resolver->cache);
        emptyCache = 0;
    }

    if (!emptyCache)
        PrintLog(&dnsLog, "replacing %s by %s", cacheToReplace->answer.name,
                     node->packet.answers[0].name);

    UpdateCache(cacheToReplace, &node->packet, curTime);

    // Write the log for this query
    char ipBuff[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, node->packet.answers[0].rdata, ipBuff, INET6_ADDRSTRLEN);

    PrintLog(&dnsLog, "%s is at %s", node->packet.answers[0].name, ipBuff);

    // Display response of the resolver
    fprintf(stderr, "%s is at %s\n", node->packet.answers[0].name, ipBuff);

    return 0; // success

fail:
    // This is a real error
    node->packet.header.qr = HEADER_QR_RESPONSE;
    node->packet.header.rcode = HEADER_RCODE_SERVFAIL;
    node->packet.header.ra = 1;

update:
    // Update packet data
    UpdatePacketData(node);

    return -1;
}

void delResolver(void *res)
{
    DeleteResolver(res);
}

void CloseResolver(Resolver *resolver)
{
    LIST_DeleteNodeRecur(resolver->root, delResolver);

    for (int i = 0; i < NUM_CACHE; i++)
    {
        ClearCache(&resolver->cache[i]);
    }
}
