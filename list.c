#include "list.h"

LISTNode LIST_MakeNode(int key, void * value)
{
    LISTNode node = (LISTNode)malloc(sizeof(struct _LISTNode));

    if (!node) return NULL;

    node->key = key;
    node->value = value;
    node->next = NULL;

    return node;
}

LISTNode LIST_InsertNode(LISTNode start, LISTNode node)
{
    if (!start)
        return node;
    
    if (!node)
        return start; // no change

    start->next = LIST_InsertNode(start->next, node);

    return start;
}

LISTNode LIST_RemoveNode(LISTNode start, int key)
{
    if (!start)
        return NULL;

    if (start->key == key)
    {
        return start->next;
    }
    else
    {
        start->next = LIST_RemoveNode(start->next, key);
        return start;
    }
}

void LIST_DeleteNodeRecur(LISTNode start, void deleteHandler(void*))
{
    if (!start) return;

    // Delete next items
    LIST_DeleteNodeRecur(start->next, deleteHandler);

    // Delete start
    deleteHandler(start->value);
    LIST_DeleteNode(start);
}

void LIST_DeleteNode(LISTNode node)
{
    free(node);
}

LISTNode LIST_FindNode(LISTNode start, int key)
{
    if (!start)
        return NULL;

    if (start->key == key)
    {
        return start;
    }
    else
    {
        return LIST_FindNode(start->next, key);
    }
}


