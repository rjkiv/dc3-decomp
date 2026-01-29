#include "trie.h"

void Trie::inc_dup_count(unsigned int n) {
    check_index(n);
    char *node = TRIE_GET_NODE(n);
    unsigned int *countsPtr = TRIE_GET_COUNTS(node);
    unsigned int dupCount = TRIE_GET_DUP_COUNT(countsPtr);
    check_index(n);
    unsigned char count = TRIE_GET_COUNT(node);
    *countsPtr = TRIE_INC_DUP_COUNT(dupCount, count);
}

void Trie::dec_dup_count(unsigned int n) {
    check_index(n);
    char *node = TRIE_GET_NODE(n);
    unsigned int *countsPtr = TRIE_GET_COUNTS(node);
    unsigned int dupCount = TRIE_GET_DUP_COUNT(countsPtr);
    check_index(n);
    unsigned char count = TRIE_GET_COUNT(node);
    *countsPtr = TRIE_DEC_DUP_COUNT(dupCount, count);
}

void Trie::inc_count(unsigned int n) {
    check_index(n);
    char *node = TRIE_GET_NODE(n);
    unsigned char count = TRIE_GET_COUNT(node);
    check_index(n);
    unsigned int *countsPtr = TRIE_GET_COUNTS(node);
    *countsPtr = TRIE_INC_COUNT(countsPtr, count);
}

void Trie::dec_count(unsigned int n) {
    check_index(n);
    char *node = TRIE_GET_NODE(n);
    unsigned char count = TRIE_GET_COUNT(node);
    check_index(n);
    unsigned int *countsPtr = TRIE_GET_COUNTS(node);
    *countsPtr = TRIE_DEC_COUNT(countsPtr, count);
}

unsigned int Trie::get_free_node() {
    unsigned int n = *TRIE_GET_FREE_HEAD;

    if (n != 0) {
        check_index(n);
        *TRIE_GET_FREE_HEAD = TRIE_GET_SIBLING(n);
    } else {
        int _nodeCount = *TRIE_GET_NODE_COUNT;
        MILO_ASSERT(_nodeCount < MAX_NODES, 0x82);
        n = _nodeCount;
        TRIE_INC_NODE_COUNT;
    }
    return n;
}

void Trie::delete_node(unsigned int n) {
    check_index(n);
    TRIE_CLEAR_NODE(n);
    check_index(n);
    TRIE_CLEAR_SIBLING(n);
    check_index(n);
    TRIE_CLEAR_PARENT(n);
    *TRIE_GET_COUNTS(n) = *(unsigned int *)0;
    check_index(n);
    unsigned int *freeHead = TRIE_GET_FREE_HEAD;
    TRIE_SET_CHAR(n, 0xFF);
    if (*freeHead != 0) {
        check_index(n);
        TRIE_SET_SIBLING(n, *freeHead);
    }
    *freeHead = n;
}

void Trie::remove(unsigned int n) {
    check_index(n);
    char *node = TRIE_GET_NODE(n);
    if (TRIE_GET_CHAR(n) != 0)
        return;
    check_index(n);
    if (TRIE_GET_DUP_COUNT(node) == 0)
        return;
    if (TRIE_GET_DUP_COUNT(node) != 0x100) {
        check_index(n);
        return;
    }
    for (;;) {
        if (n != 0) {
            check_index(n);
            unsigned int parent = TRIE_GET_PARENT(n);
            if (parent != 0) {
                check_index(n);
                parent = TRIE_GET_PARENT(n);
                check_index(parent);
                unsigned int firstChild = TRIE_GET_CHILD(parent);
                check_index(firstChild);

                if (TRIE_GET_COUNT(TRIE_GET_NODE(firstChild)) == 1) {
                    check_index(n);
                    parent = TRIE_GET_PARENT(n);
                    delete_node(n);
                    n = parent;
                    check_index(n);
                    node = TRIE_GET_NODE(n);
                    if (TRIE_GET_CHAR(n) != 0)
                        break;
                }
            }
        }
        check_index(n);
        unsigned int firstChild;
        if (TRIE_GET_PARENT(n) == 0) {
            firstChild = 1;
        } else {
            check_index(n);
            unsigned int parent = TRIE_GET_PARENT(n);
            check_index(parent);
            firstChild = TRIE_GET_CHILD(parent);
        }
        check_index(firstChild);
        unsigned char count = TRIE_GET_COUNT(TRIE_GET_NODE(firstChild));
        unsigned int currNode = firstChild;
        unsigned int prevNode = 0;
        for (unsigned int i = 0; i < count; i++) {
            if (currNode != 0) {
                check_index(n);
                check_index(prevNode);
                TRIE_SET_SIBLING(prevNode, TRIE_GET_SIBLING(n));
                delete_node(n);
                dec_count(firstChild);
                return;
            }
            if (n == 1) {
                unsigned int last = 1;
                for (unsigned int j = 0; j < TRIE_GET_COUNT(TRIE_GET_NODE(1)) - 1; j++) {
                    check_index(last);
                    last = TRIE_GET_SIBLING(last);
                }
                if (last != 1) {
                    check_index(last);
                    *(unsigned int *)((char *)this + NODE_SIZE) = TRIE_GET_CHILD(last);
                    check_index(last);
                    *((char *)this + 0x21) = TRIE_GET_CHAR(last);
                    delete_node(last);
                    dec_count(firstChild);
                    unsigned int child = *(unsigned int *)((char *)this + NODE_SIZE);
                    unsigned int newChild = *(unsigned int *)((char *)this + NODE_SIZE);
                    check_index(newChild);
                    for (unsigned int j = 0; j < TRIE_GET_COUNT(TRIE_GET_NODE(newChild));
                         j++) {
                        check_index(child);
                        *(unsigned int *)(TRIE_GET_NODE(child) + 0x8) = 1;
                        check_index(child);
                        child = TRIE_GET_SIBLING(child);
                    }
                    return;
                }
                n = 1;
            } else {
                check_index(n);
                check_index(n);
                unsigned int parent = TRIE_GET_PARENT(n);
                check_index(parent);
                *(unsigned int *)TRIE_GET_NODE(parent) = *(unsigned int *)(node + 0x4);
                check_index(n);
                unsigned char oldCount = TRIE_GET_COUNT(node);
                check_index(n);
                parent = TRIE_GET_PARENT(n);
                check_index(parent);
                firstChild = TRIE_GET_CHILD(parent);
                check_index(firstChild);
                char *firstChildNode = TRIE_GET_NODE(firstChild);
                *(unsigned int *)(firstChildNode + 0xC) =
                    (*(unsigned int *)(firstChildNode + 0xC) & 0xFFFFFF00)
                    | (unsigned int)(oldCount - 1);
            }
            delete_node(n);
            dec_dup_count(n);
            return;
        }
        check_index(currNode);
        prevNode = currNode;
        currNode = TRIE_GET_SIBLING(currNode);
    }
}