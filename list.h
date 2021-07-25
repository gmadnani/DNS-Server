#ifndef LIST_H_
#define LIST_H_

#include <stdlib.h>

/**
 * @brief Linked-List (LIST) node data structure 
 * with the keys arranged in increasing order
 * 
 */
typedef struct _LISTNode
{
    int key;
    void *value;

    struct _LISTNode *next;
} * LISTNode;

/**
 * @brief Creates a LIST node.
 * 
 * @param key key of the ndoe
 * @param value value of the node
 * @return LISTNode the created node
 */
LISTNode LIST_MakeNode(int key, void * value);

/**
 * @brief Inserts a node into the tree.
 * 
 * @param start start node
 * @param node new node
 * @return LISTNode the new start node
 */
LISTNode LIST_InsertNode(LISTNode start, LISTNode node);

/**
 * @brief Removes a node from the tree.
 * 
 * @param start start node
 * @param key key of node to remove
 * @return LISTNode the new start node
 */
LISTNode LIST_RemoveNode(LISTNode start, int key);

/**
 * @brief Delete a node
 * 
 * @param node the node to delete
 */
void LIST_DeleteNode(LISTNode node);

/**
 * @brief Deletes the node starting from the start ndoe.
 * 
 * @param start start node
 * @param deleteHandler function to handle deletion of values
 */
void LIST_DeleteNodeRecur(LISTNode start, void deleteHandler(void*));

/**
 * @brief Find a node in the tree.
 * 
 * @param start the start node
 * @param key the key of the node to find
 * @return LISTNode NULL if not found, node object if found
 */
LISTNode LIST_FindNode(LISTNode start, int key);

#endif // LIST_H_
