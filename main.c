#include "server.h"

Listener listener;
Log dnsLog;

void handler(int signum) {
    CloseListener(&listener);
    CloseLog(&dnsLog);

    exit(0);
}

int main(int argc, char* argv[]) {

    // Override signal handlers.
    signal(SIGINT, handler);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGABRT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGSEGV, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    // Parse parameters
    char * serverIp = "1.1.1.1";
    uint16_t serverPort = 53; // default port is 53 for  server

    if (argc != 3) {
        WARN("Usage: ./dns_svr <server-ip> <server-port>\n");
        WARN("Using default values.\n");
    }

    if (argc > 1) {
        serverIp = argv[1];
    } 

    if (argc > 2) {
        if (!sscanf(argv[2], "%hd", &serverPort)) {
            WARN("Invalid server port format. Using default: 53.\n");
        }
    }

    // Initialize objects
    dnsLog = OpenLog("dns_svr.log");
    listener = OpenListener(serverIp, serverPort);

    // Run the server here
    for (;;) 
    {
        RunListener(&listener);
    }
}


// References:
// https://github.com/tomasorti/dns-server/blob/master/dnsServer/src/server.cpp
// https://www.binarytides.com/socket-programming-c-linux-tutorial/
// https://datatracker.ietf.org/doc/html/rfc1035
// https://www.scottklement.com/rpg/socktut/nonblocking.html
// https://www.ibm.com/docs/en/i/7.2?topic=designs-example-nonblocking-io-select
