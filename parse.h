#ifndef PARSE_H_
#define PARSE_H_

#include "server.h"

/**
 * @brief Parses the data received from the packet and fills the packet with the
 * appropriate inputs. 
 * 
 * @param packet pointer to the Packet object
 * @param buffer pointer to the data buffer
 * @param length length of the data buffer
 * @param offset number of bytes to start reading
 * @return int   number of bytes read
 */
int ParsePacket(Packet* packet, uint8_t* buffer, int length, int offset);

/**
 * @brief Parses the header section of a  packet.
 * 
 * @param header pointer to the Header object
 * @param buffer pointer to the data buffer
 * @param length length of the data buffer
 * @param offset number of bytes to start reading
 * @return int   number of bytes read
 */
int ParseHeader(Header* header, uint8_t* buffer, int length, int offset);

/**
 * @brief Parses a question from the  packet.
 * 
 * @param question pointer to the Question object
 * @param buffer pointer to the data buffer
 * @param length length of the data buffer
 * @param offset number of bytes to start reading
 * @return int   number of bytes read
 */
int ParseQuestion(Question* question, uint8_t* buffer, int length, int offset);

/**
 * @brief Parses a record from the  packet.
 * 
 * @param record pointer to the Record object
 * @param buffer pointer to the data buffer
 * @param length length of the data buffer
 * @param offset number of bytes to start reading
 * @return int   number of bytes read
 */
int ParseRecord(Record* record, uint8_t* buffer, int length, int offset);

/**
 * @brief Parses a name from the  packet.
 * 
 * @param name pointer to the name string
 * @param buffer pointer to the data buffer
 * @param length length of the data buffer
 * @param offset number of bytes to start reading
 * @return int   number of bytes read
 */
int ParseName(char* name, uint8_t* buffer, int length, int offset);

/**
 * @brief Parses the length of the name of a question or record.
 * 
 * @param buffer pointer to the data buffer
 * @param length length of the data buffer
 * @param offset number of bytes to start reading
 * @return int   number of bytes read
 */
int ParseNameLength(uint8_t* buffer, int length, int offset);

#endif // PARSE_H_
