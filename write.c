#include "server.h"

int WritePacket(Packet *packet, uint8_t *data, int length)
{
    int written = 0;
    int totalWritten = 0;
    uint8_t *ptr = data;

    // Header
    if ((written = WriteHeader(&packet->header, data, length)) < 0)
    {
        return -1;
    }
    totalWritten += written;

    // Write questions (no pointer used)
    for (int i = 0; i < packet->header.qdcount; i++)
    {
        if (data) ptr = data + totalWritten;
        if ((written = WriteQuestion(&packet->questions[i], ptr, length - totalWritten)) < 0)
        {
            return -1;
        }
        totalWritten += written;
    }

    // Write answers (no pointer used)
    for (int i = 0; i < packet->header.ancount; i++)
    {
        if (data) ptr = data + totalWritten;
        if ((written = WriteRecord(&packet->answers[i], ptr, length - totalWritten)) < 0)
        {
            return -1;
        }
        totalWritten += written;
    }

    // Write authorities records (no pointer used)
    for (int i = 0; i < packet->header.nscount; i++)
    {
        if (data) ptr = data + totalWritten;
        if ((written = WriteRecord(&packet->authorities[i], ptr, length - totalWritten)) < 0)
        {
            return -1;
        }
        totalWritten += written;
    }

    // Write additional records (no pointer used)
    for (int i = 0; i < packet->header.arcount; i++)
    {
        if (data) ptr = data + totalWritten;
        if ((written = WriteRecord(&packet->additional[i], ptr, length - totalWritten)) < 0)
        {
            return -1;
        }
        totalWritten += written;
    }

    return totalWritten;
}

int WriteHeader(Header *header, uint8_t *data, int length)
{
    if (!data)
        return 12; // if data pointer is null, just return the number of bytes that will be written

    // Check if there is enough space to write the dns header
    if (length < 12)
    {
        errno = ENOBUFS;
        return -1;
    }

    uint16_t word;

    // ID
    word = htons(header->id);
    memcpy(data, &word, 2);

    // Flags
    word = header->qr << 15;
    word |= header->opcode << 11;
    word |= header->aa << 10;
    word |= header->tc << 9;
    word |= header->rd << 8;
    word |= header->ra << 7;
    word |= header->z << 4;
    word |= header->rcode;
    word = htons(word);
    memcpy(data + 2, &word, 2);

    // QD count
    word = htons(header->qdcount);
    memcpy(data + 4, &word, 2);

    // AN count
    word = htons(header->ancount);
    memcpy(data + 6, &word, 2);

    // NS count
    word = htons(header->nscount);
    memcpy(data + 8, &word, 2);

    // AR count
    word = htons(header->arcount);
    memcpy(data + 10, &word, 2);

    return 12;
}

int WriteQuestion(Question *question, uint8_t *data, int length)
{
    char strBuffer[WRITE_STR_BUFF];
    char *token = NULL;

    uint8_t byte;
    uint16_t word;

    int totalWritten = 0;

    strcpy(strBuffer, question->qname);

    // Write q name
    token = strtok(strBuffer, ".");
    while (token)
    {
        byte = strlen(token);

        if (data)
        {
            if (totalWritten + 1 > length)
            {
                errno = ENOBUFS;
                return -1;
            }
            memcpy(data + totalWritten, &byte, 1);
        }

        totalWritten++;

        if (data)
        {
            if (totalWritten + byte > length)
            {
                errno = ENOBUFS;
                return -1;
            }
            memcpy(data + totalWritten, token, byte);
        }

        totalWritten += byte;

        token = strtok(NULL, ".");
    }
    byte = 0; // write terminating byte for qname

    if (data)
    {
        if (totalWritten + 5 > length) // check if the rest of the data can be written to the buffer
        {
            errno = ENOBUFS;
            return -1;
        }
        memcpy(data + totalWritten, &byte, 1);
        totalWritten++;

        // Write q type
        word = htons(question->qtype);
        memcpy(data + totalWritten, &word, 2);
        totalWritten += 2;

        // Write q class
        word = htons(question->qclass);
        memcpy(data + totalWritten, &word, 2);
        totalWritten += 2;
    }
    else
    {
        totalWritten += 5;
    }

    return totalWritten;
}

int WriteRecord(Record *record, uint8_t *data, int length)
{
    char strBuffer[WRITE_STR_BUFF] = {0};
    char *token = NULL;

    uint8_t byte;
    uint16_t word;
    uint32_t dword;

    int totalWritten = 0;

    strcpy(strBuffer, record->name);

    // Write q name
    token = strtok(strBuffer, ".");
    while (token)
    {
        byte = strlen(token);
        if (data)
        {
            if (totalWritten + 1 > length)
            {
                errno = ENOBUFS;
                return -1;
            }
            memcpy(data + totalWritten, &byte, 1);
        }

        totalWritten++;

        if (data)
        {
            if (totalWritten + byte > length)
            {
                errno = ENOBUFS;
                return -1;
            }
            memcpy(data + totalWritten, token, byte);
        }
        totalWritten += byte;

        token = strtok(NULL, ".");
    }
    byte = 0; // write terminating byte for qname

    

    if (data)
    {
        if (totalWritten + 11 > length)
        {
            errno = ENOBUFS;
            return -1;
        }
        memcpy(data + totalWritten, &byte, 1);
        totalWritten++;

        // Write type
        word = htons(record->type);
        memcpy(data + totalWritten, &word, 2);
        totalWritten += 2;

        // Write class
        word = htons(record->class);
        memcpy(data + totalWritten, &word, 2);
        totalWritten += 2;

        // Write ttl
        dword = htonl(record->ttl);
        memcpy(data + totalWritten, &dword, 4);
        totalWritten += 4;

        // Write rdlength
        word = htons(record->rdlength);
        memcpy(data + totalWritten, &word, 2);
        totalWritten += 2;
    }
    else 
    {
        totalWritten += 11;
    }

    if (data)
    {
        // Write rdata
        if (totalWritten + record->rdlength > length)
        {
            errno = ENOBUFS;
            return -1;
        }
        memcpy(data + totalWritten, record->rdata, record->rdlength);
    }
    
    totalWritten += record->rdlength;

    return totalWritten;
}

void ClearPacket(Packet *packet)
{
    if (packet->header.qdcount > 0)
    {
        for (int i = 0; i < packet->header.qdcount; i++)
        {
            free(packet->questions[i].qname);
        }
        free(packet->questions);
        packet->questions = NULL;
    }

    if (packet->header.ancount > 0)
    {
        for (int i = 0; i < packet->header.ancount; i++)
        {
            free(packet->answers[i].name);
            free(packet->answers[i].rdata);
        }
        free(packet->answers);
        packet->answers = NULL;
    }

    if (packet->header.nscount > 0)
    {
        for (int i = 0; i < packet->header.nscount; i++)
        {
            free(packet->authorities[i].name);
            free(packet->authorities[i].rdata);
        }
        free(packet->authorities);
        packet->authorities = NULL;
    }

    if (packet->header.arcount > 0)
    {
        for (int i = 0; i < packet->header.arcount; i++)
        {
            free(packet->additional[i].name);
            free(packet->additional[i].rdata);
        }
        free(packet->additional);
        packet->additional = NULL;
    }

    packet->header.id = 0;
    packet->header.qdcount = 0;
    packet->header.ancount = 0;
    packet->header.nscount = 0;
    packet->header.arcount = 0;
    packet->header.qr = 0;
    packet->header.opcode = 0;
    packet->header.ra = 0;
    packet->header.rcode = 0;
    packet->header.z = 0;
    packet->header.rd = 0;
    packet->header.tc = 0;
    packet->header.aa = 0;
}
