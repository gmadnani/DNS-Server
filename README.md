# DNS-Server

## Background
The Domain Name System (DNS) provides, among other things, the mapping between human-meaningful
hostnames and the numeric IP addresses that indicate where packets should be sent. DNS consists of a hierarchy
of servers, each knowing a portion of the complete mapping.
In this project, you will write a DNS server that accepts requests for IPv6 addresses and serves them either from
its own cache or by querying servers higher up the hierarchy. Each transaction consists of at most four messages:
one from your client to you, one from you to your upstream server, one from your upstream server to you and one from you to your client. The middle two can be sometimes skipped if you cache some of the answers.
In a DNS system, the entry mapping a name to an IPv6 address is called a AAAA (or “quad A”) record. Its “record type” is 28.
The server will also keep a log of its activities. This is important for reasons such as detecting denial-ofservice
attacks, as well as allowing service upgrades to reflect usage patterns.
For the log, you will need to print a text version of the IPv6 addresses. IPv6 addresses are 128 bits long. They are
represented in text as eight colon-separated strings of 16-bit numbers expressed in hexadecimal. As a shorthand,
a string of consecutive 16-bit numbers that are all zero may be replaced by a single “::”.

## Task 
Write a miniature DNS server that will serve AAAA queries.
Accept a DNS “AAAA” query over TCP on port 8053. Forward it to a server whose IPv4 address is the first
command-line argument and whose port is the second command-line argument. (For testing, use the value in
/etc/resolv.conf on your server and port 53). Send the response back to the client who sent the request, over the
same TCP connection. There will be a separate TCP connection for each query/response with the client. Log these
events, as described below.
Note that DNS usually uses UDP, but this project will use TCP because it is a more useful skill for you to learn. A
DNS message over TCP is slightly different from that over UDP: it has a two-byte header that specify the length
(in bytes) of the message, not including the two-byte header itself [4, 5]. This means that you know the size of
the message before you read it, and can malloc() enough space for it.
Assume that there is only one question in the DNS request you receive, although the standard allows there to be
more than one. If there is more than one answer in the reply, then only log the first one, but always reply to the
client with the entire list of answers. If there is no answer in the reply, log the request line only. If the first answer
in the response is not a AAAA field, then do not print a log entry (for any answer in the response).
The program should be ready to accept another query as soon as it has processed the previous query and
response. (If Non-blocking option is implemented, it must be ready before this too.)
Your server should not shut down by itself. SIGINT (like CTRL-C) will be used to terminate your server between
test cases.
You may notice that a port and interface which has been bound to a socket sometimes cannot be reused until
after a timeout. To make your testing and our marking easier, please override this behaviour by placing the
following lines before the bind() call:
int enable = 1;
if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
perror("setsockopt"); exit(1);
}

### Caching
Cache behaviour will be evaluated by checking the correct processing of TTL fields, ID fields, and eviction
behaviour, by both the log and the DNS responses.

### Non-blocking operation
Marks for non-blocking operation will be allocated if dns_svr is able to process up to multiple requests for the
user (up to, say, 100), and the matching replies from the server, regardless of the order of these (except, of
course, that replies must be after the requests they are replying to).