#ifndef H_
#define H_

// Library dependencies
#define GNU_SOURCE_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <fcntl.h>

#define DEBUG 1
#define NONBLOCKING
#define CACHE

// Macros for  packet
#define HEADER_QR_QUERY 0
#define HEADER_QR_RESPONSE 1

#define HEADER_OPCODE_QUERY 0
#define HEADER_OPCODE_IQUERY 1
#define HEADER_OPCODE_STATUS 2
#define HEADER_OPCODE_NOTIFY 4
#define HEADER_OPCODE_UPDATE 5

#define HEADER_RCODE_NOERROR 0
#define HEADER_RCODE_FORMERR 1
#define HEADER_RCODE_SERVFAIL 2
#define HEADER_RCODE_NXDOMAIN 3
#define HEADER_RCODE_NOTIMPL 4
#define HEADER_RCODE_REFUSED 5
#define HEADER_RCODE_YXDOMAIN 6
#define HEADER_RCODE_YXRRSET 7
#define HEADER_RCODE_NXRRSET 8
#define HEADER_RCODE_NOTAUTH 9
#define HEADER_RCODE_NOTZONE 10

#define RECORD_TYPE_A 1
#define RECORD_TYPE_AAAA 28
#define RECORD_TYPE_MX 15
#define RECORD_TYPE_NS 2

#define QUERY_CLASS_IN 1

#define LISTENER_PORT 8053

#define ERR(msg) fprintf(stderr, "%s %s.\n", \
                             msg, strerror(errno))
#define WARN(msg) fprintf(stderr, "%s\n", msg)

typedef uint16_t RecordType;
typedef uint16_t RecordClass;

// Type definitions for  packet
/**
 * @brief The header data structure for  Packets.
 * 
 */
typedef struct _Header
{
    uint16_t id;
    uint16_t qr : 1;
    uint16_t opcode : 4;
    uint16_t aa : 1;
    uint16_t tc : 1;
    uint16_t rd : 1;
    uint16_t ra : 1;
    uint16_t z : 3;
    uint16_t rcode : 4;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} Header;

typedef struct _Question
{
    char *qname; // array of string names
    RecordType qtype;
    RecordClass qclass;
} Question;

typedef struct _Record
{
    char *name;
    RecordType type;
    RecordClass class;
    uint32_t ttl;
    uint16_t rdlength;
    uint8_t *rdata;
} Record;

typedef struct _Packet
{
    Header header;
    Question *questions; // array of  Questions
    Record *answers;     // array of  Answers
    Record *authorities; // array of  Authority Records
    Record *additional;  // array of  Additional Records
} Packet;

#include "list.h"
#include "cache.h"

// Type definitions for  Listener
typedef struct _Listener
{
    int socketFD;
    struct sockaddr_in server;
    fd_set masterReadFS;
    fd_set masterWriteFS;
    int maxFD;
    LISTNode root;
    void* resolver;
} Listener;

#include "parse.h"
#include "write.h"
#include "log.h"
#include "client.h"
#include "resolver.h"
#include "tcp.h"

// Functions
/**
 * @brief Make an empty  packet object
 * 
 * @return Packet the dns packet data structure
 */
Packet MakePacket();

/**
 * @brief Opens the listener to start receiving dns queries.
 * 
 * @param serverIp string pointer of the server ip
 * @param port port number of the server
 * @return Listener the listener object
 */
Listener OpenListener(char *serverIp, uint16_t serverPort);

/**
 * @brief Close the listener
 * 
 * @param listener pointer to the listener object
 */
void CloseListener(Listener *listener);

/**
 * @brief Run the listener.
 * 
 * @param listener pointer to the listener object
 */
void RunListener(Listener *listener);

#endif // H_
