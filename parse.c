#include "server.h"

int ParsePacket(Packet *packet, uint8_t *buffer, int length, int offset)
{
    int read = 0, i = 0, totalParse = 0;

    read = ParseHeader(&packet->header, buffer, length, offset);
    if (read < 0)
        return -1;
    totalParse += read;

    // Parse the questions section
    if (packet->header.qdcount > 0)
    {
        packet->questions = (Question *)malloc(sizeof(Question) * packet->header.qdcount);
        if (!packet->questions)
            return -1;

        for (i = 0; i < packet->header.qdcount; i++)
        {
            read = ParseQuestion(&packet->questions[i], buffer, length, offset + totalParse);
            if (read < 0)
                return -1;
            totalParse += read;
        }
    }

    // Parse the answers section
    if (packet->header.ancount > 0)
    {
        packet->answers = (Record *)malloc(sizeof(Record) * packet->header.ancount);
        if (!packet->answers)
            return -1;

        for (i = 0; i < packet->header.ancount; i++)
        {
            read = ParseRecord(&packet->answers[i], buffer, length, offset + totalParse);
            if (read < 0)
                return -1;
            totalParse += read;
        }
    }

    // Parse the authorities section
    if (packet->header.nscount > 0)
    {
        packet->authorities = (Record *)malloc(sizeof(Record) * packet->header.nscount);
        if (!packet->authorities)
            return -1;

        for (i = 0; i < packet->header.nscount; i++)
        {
            read = ParseRecord(&packet->authorities[i], buffer, length, offset + totalParse);
            if (read < 0)
                return -1;
            totalParse += read;
        }
    }

    // Parse the additional records section
    if (packet->header.arcount > 0)
    {
        packet->additional = (Record *)malloc(sizeof(Record) * packet->header.arcount);
        if (!packet->additional)
            return -1;

        for (i = 0; i < packet->header.arcount; i++)
        {
            read = ParseRecord(&packet->additional[i], buffer, length, offset + totalParse);
            if (read < 0)
                return -1;
            totalParse += read;
        }
    }

    return totalParse;
}

int ParseHeader(Header *header, uint8_t *buffer, int length, int offset)
{
    int totalParse = 12;

    if (offset + totalParse > length)
    {
        errno = ENOBUFS;
        return -1;
    }

    uint16_t flags = ntohs(*((uint16_t *)(buffer + offset + 2)));
    header->rcode = flags & 0xF;
    header->z = (flags >> 4) & 0x7;
    header->ra = (flags >> 7) & 0x1;
    header->rd = (flags >> 8) & 0x1;
    header->tc = (flags >> 9) & 0x1;
    header->aa = (flags >> 10) & 0x1;
    header->opcode = (flags >> 11) & 0xf;
    header->qr = (flags >> 15) & 0x1;

    header->id = ntohs(*((uint16_t *)(buffer + offset)));
    header->qdcount = ntohs(*((uint16_t *)(buffer + offset + 4)));
    header->ancount = ntohs(*((uint16_t *)(buffer + offset + 6)));
    header->nscount = ntohs(*((uint16_t *)(buffer + offset + 8)));
    header->arcount = ntohs(*((uint16_t *)(buffer + offset + 10)));

    return totalParse;
}

int ParseQuestion(Question *question, uint8_t *buffer, int length, int offset)
{
    int read, totalParse = 0, nameLength;

    nameLength = ParseNameLength(buffer, length, offset);
    question->qname = (char *)malloc(nameLength + 1);
    if (!question->qname)
    {
        return -1;
    }

    read = ParseName(question->qname, buffer, length, offset);
    if (read < 0)
        return -1;
    totalParse += read;

    if (offset + totalParse + 4 > length)
    {
        errno = ENOBUFS;
        return -1;
    }

    question->qtype = ntohs(*((uint16_t *)(buffer + offset + totalParse)));
    question->qclass = ntohs(*((uint16_t *)(buffer + offset + totalParse + 2)));

    return totalParse + 4;
}

int ParseRecord(Record *record, uint8_t *buffer, int length, int offset)
{
    int read, totalParse = 0, nameLength;

    nameLength = ParseNameLength(buffer, length, offset);
    record->name = (char *)malloc(nameLength + 1);
    if (!record->name)
    {
        return -1;
    }

    read = ParseName(record->name, buffer, length, offset);
    if (read < 0)
        return -1;
    totalParse += read;

    if (offset + totalParse + 10 > length)
    {
        errno = ENOBUFS;
        return -1;
    }

    record->type = ntohs(*((uint16_t *)(buffer + offset + totalParse)));
    record->class = ntohs(*((uint16_t *)(buffer + offset + totalParse + 2)));
    record->ttl = ntohl(*((uint32_t *)(buffer + offset + totalParse + 4)));
    record->rdlength = ntohs(*((uint16_t *)(buffer + offset + totalParse + 8)));
    totalParse += 10;

    if (offset + totalParse + record->rdlength > length)
    {
        errno = ENOBUFS;
        return -1;
    }

    record->rdata = (uint8_t *)malloc(record->rdlength);
    if (!record->rdata)
        return -1;

    memcpy(record->rdata, buffer + offset + totalParse, record->rdlength);

    return totalParse + record->rdlength;
}

int ParseName(char *name, uint8_t *buffer, int length, int offset)
{
    uint8_t labelLength = 0;
    uint16_t pointer = 0;
    int nlength = 0;

    while ((labelLength = buffer[offset + nlength]))
    {
        if (labelLength & 0xC0)
        { // this is a pointer
            if (offset + nlength + 2 > length)
            {
                errno = ENOBUFS;
                return 0;
            }
            pointer = ntohs(*((uint16_t *)(buffer + offset + nlength))) & ~0xC000;

            if (ParseName(name + nlength, buffer, length, pointer) < 0)
                return -1;
            return nlength + 2;
        }
        else
        {
            if (offset + nlength + labelLength + 1 > length)
            {
                errno = ENOBUFS;
                return -1;
            }

            memcpy(name + nlength, buffer + offset + nlength + 1, labelLength);
            name[nlength + labelLength] = '.';
            nlength += labelLength + 1;
        }
    }
    
    // set null character
    if (nlength)
        name[nlength - 1] = 0;
    else
        name[0] = 0; 

    return nlength + 1; // include the null byte at the end
}

int ParseNameLength(uint8_t *buffer, int length, int offset)
{
    uint8_t labelLength = 0;
    uint16_t pointer = 0;
    int nlength = 0;

    while ((labelLength = buffer[offset + nlength]))
    {
        if (labelLength & 0xC0)
        { // this is a pointer

            if (offset + nlength + 2 > length)
            {
                errno = ENOBUFS;
                return 0;
            }

            pointer = ntohs(*((uint16_t *)(buffer + offset + nlength))) & ~0xC000;
            nlength += ParseNameLength(buffer, length, pointer);
            return nlength;
        }
        else
        {
            if (offset + nlength + labelLength + 1 > length)
            {
                errno = ENOBUFS;
                return 0;
            }

            nlength += labelLength + 1;
        }
    }

    if (nlength)
        return nlength - 1; // subtract one for the excess dot
    else
        return 0;
}
