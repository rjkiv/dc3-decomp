#include "trie.h"

void Trie::inc_count(unsigned int n) {
    unsigned int count = get_count(n);
    set_count(n, count + 1);
}

void Trie::dec_count(unsigned int n) {
    unsigned int count = get_count(n);
    set_count(n, count - 1);
}

void Trie::inc_dup_count(unsigned int n) {
    unsigned int count = get_dup_count(n);
    set_dup_count(n, count + 1);
}

void Trie::dec_dup_count(unsigned int n) {
    unsigned int count = get_dup_count(n);
    set_dup_count(n, count - 1);
}

unsigned int Trie::get_free_node() {
    unsigned int n = mFreeHead;
    if (n != 0) {
        mFreeHead = get_next_sibling(n);
        return n;
    } else {
        MILO_ASSERT(_nodeCount < MAX_NODES, 0x82);
        return _nodeCount++;
    }
}

void Trie::delete_node(unsigned int n) {
    set_first_child(n, 0);
    set_next_sibling(n, 0);
    clear_parent(n);
    set_char(n, 0xFF);

    // if (mFreeHead) {
    //     set_next_sibling(n, mFreeHead);
    // }
    // mFreeHead = n;
    // return;

    // this part is fake
    unsigned int *freeHead = TRIE_GET_FREE_HEAD;
    if (*freeHead != 0) {
        check_index(n);
        TRIE_SET_SIBLING(n, *freeHead);
    }
    *freeHead = n;
}

int Trie::store(const char *str) {
    if (str && *str) {
        int node_idx = 1;
        int n_00 = 0;
        int len = strlen(str);
        for (int i = 0; i <= len; i++) {
            char curChar = str[i];
            int dupeCount = get_dup_count(node_idx);
            for (int j = 0; j < dupeCount; j++) {
                if (get_char(node_idx) != curChar) {
                    if (j != dupeCount - 1) {
                        node_idx = get_next_sibling(node_idx);
                    }
                } else {
                    n_00 = node_idx;
                    node_idx = get_first_child(node_idx);
                    goto cnt;
                }
            }
            unsigned int next_free = get_free_node();
            if (dupeCount == 0) {
                if (n_00 > 0) {
                    set_first_child(n_00, next_free);
                }
            } else {
                set_next_sibling(node_idx, next_free);
            }
            set_char(next_free, curChar);
            set_parent(next_free, n_00);
            if (n_00 > 0) {
                inc_count(get_first_child(n_00));
            } else {
                inc_count(1);
            }
            n_00 = next_free;
            node_idx = next_free;
            if (str[i] != '\0') {
                while (i++ < len) {
                    n_00 = get_free_node();
                    set_first_child(next_free, n_00);
                    set_parent(n_00, next_free);
                    set_char(n_00, str[i]);
                    inc_count(n_00);
                    node_idx = n_00;
                    next_free = n_00;
                }
                break;
            } else {
            cnt:
                continue;
            }
        }
        if (node_idx == 0) {
            node_idx = n_00;
        }
        inc_dup_count(node_idx);
        return node_idx;
    } else {
        return 0;
    }
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
