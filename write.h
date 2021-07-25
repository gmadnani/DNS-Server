#ifndef WRITE_H_
#define WRITE_H_

#include "server.h"

#define WRITE_STR_BUFF 256

/**
 * @brief Writes the dns packet into the data buffer
 * 
 * @param packet pointer to the dns packet to write
 * @param data pointer to the data buffer
 * @param length length of the data buffer
 * @return int returns the number of bytes written
 */
int WritePacket(Packet *packet, uint8_t *data, int length);

/**
 * @brief Writes the dns header into the data buffer
 * 
 * @param header pointer to the dns header to write
 * @param data pointer to the data buffer
 * @param length length of the data buffer
 * @return int returns the nubmer of bytes written
 */
int WriteHeader(Header *header, uint8_t *data, int length);

/**
 * @brief Writes the dns question into the data buffer
 * 
 * @param question pointer to the dns question to write
 * @param data pointer to the data buffer
 * @param length length of the data buffer
 * @return int returns the nubmer of bytes written
 */
int WriteQuestion(Question *question, uint8_t *data, int length);

/**
 * @brief Writes the dns record into the data buffer
 * 
 * @param record pointer to the dns record to write
 * @param data pointer to the data buffer
 * @param length length of the data buffer
 * @return int returns the nubmer of bytes written
 */
int WriteRecord(Record *record, uint8_t *data, int length);

/**
 * @brief Clears the  packet object.
 * 
 * @param packet pointer to the  packet to clear.
 */
void ClearPacket(Packet *packet);

#endif // WRITE_H_
