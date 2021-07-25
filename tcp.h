#ifndef TCP_H_
#define TCP_H_

#include "server.h"

/**
 * @brief Reads a TCP socket in non-blocking mode.
 * 
 * @param sockfd socket file descriptor
 * @param data pointer of the data buffer
 * @param length pointer of the length 
 * @param nread pointer of the number of bytes read
 * @return int error code
 */
int ReadTCP(int sockfd, uint8_t** data, uint16_t* length, uint16_t* nread);

/**
 * @brief Writes a TCP socket in non-blocking mode.
 * 
 * @param sockfd socket file descriptor
 * @param data pointer of the data buffer
 * @param length pointer of the length
 * @param nwritten pointer of the number of bytes written
 * @return int error code
 */
int WriteTCP(int sockfd, uint8_t** data, uint16_t* length, uint16_t* nwritten);

/**
 * @brief Update packet data based on updated packet object.
 * 
 * @param node pointer to the node value
 */
void UpdatePacketData(ResolverNode node);

#endif
