#include "server.h"

// At the start data should be NULL and length should be zero
int ReadTCP(int sockfd, uint8_t **data, uint16_t *length, uint16_t *nread)
{
    int resp;

    // Read the packet size first
    if (*length == 0)
    {
        resp = read(sockfd, length, 2);

        // Failed to read packet size
        if (resp != 2)
        {
            ERR("Failed to read packet size.");

            return -1;
        }

        *length = ntohs(*length);

        // Before Read TCP is called, the data should be assumed NULL
        *data = (uint8_t *)malloc(*length);

        if (!*data)
        {
            ERR("Failed to allocate memory.");
            return -1;
        }

        // Initialize nread
        *nread = 0;
    }

    // Try to read full data
    resp = read(sockfd, *data + *nread, *length - *nread);

    // Failed to read the query
    if (resp < 0)
    {
        if (errno != EWOULDBLOCK)
            ERR("Failed to read TCP query.");
        return -1;
    }

    // Update total read bytes counter
    *nread += resp;

    // Check if reading is incomplete
    if (*nread < *length)
    {
        errno = EWOULDBLOCK; // read again on the next loop
        return -1;
    }

    // Reset counter
    *nread = 0;

    return 0; // success
}

// Assumed nwritten is zero
int WriteTCP(int sockfd, uint8_t **data, uint16_t *length, uint16_t *nwritten)
{
    int resp;

    if (!*nwritten)
    {
        uint16_t packetSize = htons(*length);
        resp = write(sockfd, &packetSize, 2);

        if (resp != 2)
        {
            ERR("Failed to write packet size.");
            return -1;
        }
    }

    // Try to write full data
    resp = write(sockfd, *data + *nwritten, *length - *nwritten);

    if (resp < 0)
        return -1;

    // Update total written bytes
    *nwritten += resp;

    // Check if writing is complete
    if (*nwritten < *length)
    {
        errno = EWOULDBLOCK; // try again.
        return -1;
    }

    // Reset counter
    *nwritten = 0;

    // Done
    return 0;
}

void UpdatePacketData(ResolverNode node)
{
    // Write the packet to the client data
    // Measure the size of the packet
    node->client->packetLength = WritePacket(&node->packet, NULL, 0);

    // Allocate packet data
    if (node->client->packetData)
        free(node->client->packetData);

    node->client->packetData = (uint8_t *)malloc(node->client->packetLength);
    memset(node->client->packetData, 0, node->client->packetLength);

    // Failed to allocate memory
    if (!node->client->packetData)
    {
        node->client->packetLength = 0;
    }

    // Failed to write data to packet
    if (WritePacket(&node->packet, node->client->packetData, node->client->packetLength) < 0)
    {
        free(node->client->packetData);
        node->client->packetLength = 0;
        node->client->packetData = NULL;
    }
}
