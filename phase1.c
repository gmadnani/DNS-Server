// The phase 1 converts the packet file into an entry into the log file.
#include "server.h"

Log dnsLog;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        WARN("Usage: ./phase1 [query|response] < <path-to-packet-file>\n");
        return -1;
    }

    char *type = argv[1];

    if (strcmp(type, "query") && strcmp(type, "response"))
    { 
        // request type is neither query nor response
        WARN("Usage: ./phase1 [query|response] < <path-to-packet-file>");
        return -1;
    }

    uint16_t packetLength;
    if (read(STDIN_FILENO, &packetLength, 2) != 2)
    {
        WARN("Failed to read the packet size.");
        return -1;
    }
    packetLength = ntohs(packetLength);

    printf("Packet size: %u\n", packetLength);

    // Create buffer to hold the data
    uint8_t *data = (uint8_t *)malloc(packetLength);

    // Read the data using a loop
    for (int totalRead = 0, nread = 0; totalRead < packetLength; totalRead += nread)
    {
        nread = read(STDIN_FILENO, data + totalRead, packetLength - totalRead);
    }

    // Test the packet
    Packet packet;
    if (ParsePacket(&packet, data, packetLength, 0) < 0)
    {
        ERR("Failed to read packet.");
        return -1;
    }
    free(data);

    // Initialize the log
    dnsLog = OpenLog("dns_svr.log");

    // Print log
    if (strcmp(type, "query") == 0)
    {
        for (uint16_t i = 0; i < packet.header.qdcount; i++)
        {
            PrintLog(&dnsLog, "requested %s", packet.questions[i].qname);

            if (packet.questions[i].qtype != RECORD_TYPE_AAAA)
            {
                PrintLog(&dnsLog, "unimplemented request");
            }
        }
    }
    else if (strcmp(type, "response") == 0)
    {
        for (uint16_t i = 0; i < packet.header.ancount; i++)
        {
            if (packet.answers[i].type == RECORD_TYPE_AAAA)
            {
                char ipBuff[INET6_ADDRSTRLEN];
                PrintLog(&dnsLog, "%s is at %s", packet.answers[i].name,
                             inet_ntop(AF_INET6, packet.answers[i].rdata, ipBuff, INET6_ADDRSTRLEN));
            }
            else
            {
                PrintLog(&dnsLog, "unimplemented request");
            }
            break;
        }
    }

    // Clear memory
    ClearPacket(&packet);
    CloseLog(&dnsLog);

    return 0;
}
