#include "trie.h"

void Trie::inc_count(unsigned int n) {
    unsigned int c = count(n);
    set_count(n, c + 1);
}

void Trie::dec_count(unsigned int n) {
    unsigned int c = count(n);
    set_count(n, c - 1);
}

void Trie::inc_dup_count(unsigned int n) {
    unsigned int c = dup_count(n);
    set_dup_count(n, c + 1);
}

void Trie::dec_dup_count(unsigned int n) {
    unsigned int c = dup_count(n);
    set_dup_count(n, c - 1);
}

unsigned int Trie::get_free_node() {
    unsigned int n = _freeListHead;
    if (n != 0) {
        _freeListHead = sibling(n);
        return n;
    } else {
        MILO_ASSERT(_nodeCount < MAX_NODES, 0x82);
        return _nodeCount++;
    }
}

void Trie::delete_node(unsigned int node) {
    set_first_child(node, 0);
    set_next_sibling(node, 0);
    clear_parent(node);
    set_char(node, 0xFF);
    clear_next_sibling(node);
}

int Trie::store(const char *s) {
    if (s && *s) {
        int node_idx = 1;
        int n_00 = 0;
        int len = strlen(s);
        for (int i = 0; i <= len; i++) {
            char curChar = s[i];
            char cnt = count(node_idx);
            for (int j = 0; j < cnt; j++) {
                if (ch(node_idx) != curChar) {
                    if (j != cnt - 1) {
                        node_idx = sibling(node_idx);
                    }
                } else {
                    n_00 = node_idx;
                    node_idx = first_child(node_idx);
                    goto cnt;
                }
            }
            unsigned int next_free = get_free_node();
            if (cnt == 0) {
                if (n_00 > 0) {
                    set_first_child(n_00, next_free);
                }
            } else {
                set_next_sibling(node_idx, next_free);
            }
            set_char(next_free, curChar);
            set_parent(next_free, n_00);
            if (n_00 > 0) {
                inc_count(first_child(n_00));
            } else {
                inc_count(1);
            }
            n_00 = next_free;
            node_idx = next_free;
            if (s[i] != '\0') {
                while (i++ < len) {
                    next_free = get_free_node();
                    set_first_child(n_00, next_free);
                    set_parent(next_free, n_00);
                    set_char(next_free, s[i]);
                    inc_count(next_free);
                    n_00 = next_free;
                    node_idx = next_free;
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

void Trie::remove(unsigned int node) {
    if (ch(node) == '\0' && dup_count(node) != 0) {
        if (dup_count(node) == 1) {
            do {
                if (node != 0 && parent(node) != 0
                    && count(first_child(parent(node))) == 1) {
                    unsigned int par = parent(node);
                    delete_node(node);
                    node = par;
                } else {
                    unsigned int u3;
                    if (parent(node) == 0) {
                        u3 = 1;
                    } else {
                        u3 = first_child(parent(node));
                    }
                    unsigned int u7 = u3;
                    unsigned int cnt = count(u3);
                    unsigned int u5 = 0;
                    for (int i = 0; i < cnt; i++) {
                        unsigned int n1 = u7;
                        if (n1 == node) {
                            if (u5 != 0) {
                                set_next_sibling(u5, sibling(node));
                                delete_node(node);
                                dec_count(u3);
                            } else if (node == 1) {
                                u7 = 1;
                                for (int j = 0; j < count(1) - 1; j++) {
                                    u7 = sibling(u7);
                                }
                                if (u7 != 1) {
                                    set_first_child(1, first_child(u7));
                                    set_char(1, ch(u7));
                                    delete_node(u7);
                                    dec_count(u3);
                                    u3 = first_child(1);
                                    for (int j = 0; j < count(first_child(1));
                                         j++) {
                                        set_parent(u3, 1);
                                        u3 = sibling(u3);
                                    }
                                } else {
                                    delete_node(1);
                                }
                            } else {
                                set_first_child(parent(node), sibling(node));
                                set_count(
                                    first_child(parent(node)), count(node) - 1
                                );
                                delete_node(node);
                            }
                            return;
                        }
                        u5 = n1;
                        u7 = sibling(n1);
                    }
                }
            } while (ch(node));
        } else {
            dec_dup_count(node);
        }
    }
}
