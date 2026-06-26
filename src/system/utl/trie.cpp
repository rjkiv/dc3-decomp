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
    clear_next_sibling(n);
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
    if (get_char(n) == '\0' && get_dup_count(n) != 0) {
        if (get_dup_count(n) == 1) {
            do {
                if (n != 0 && get_parent(n) != 0
                    && get_count(get_first_child(get_parent(n))) == 1) {
                    unsigned int parent = get_parent(n);
                    delete_node(n);
                    n = parent;
                } else {
                    unsigned int u3;
                    if (get_parent(n) == 0) {
                        u3 = 1;
                    } else {
                        u3 = get_first_child(get_parent(n));
                    }
                    unsigned int u7 = u3;
                    unsigned int count = get_count(u3);
                    unsigned int u5 = 0;
                    for (int i = 0; i < count; i++) {
                        unsigned int n1 = u7;
                        if (n1 == n) {
                            if (u5 != 0) {
                                set_next_sibling(u5, get_next_sibling(n));
                                delete_node(n);
                                dec_count(u3);
                            } else if (n == 1) {
                                u7 = 1;
                                for (int j = 0; j < get_count(1) - 1; j++) {
                                    u7 = get_next_sibling(u7);
                                }
                                if (u7 != 1) {
                                    set_first_child(1, get_first_child(u7));
                                    set_char(1, get_char(u7));
                                    delete_node(u7);
                                    dec_count(u3);
                                    u3 = get_first_child(1);
                                    for (int j = 0; j < get_count(get_first_child(1));
                                         j++) {
                                        set_parent(u3, 1);
                                        u3 = get_next_sibling(u3);
                                    }
                                } else {
                                    delete_node(1);
                                }
                            } else {
                                set_first_child(get_parent(n), get_next_sibling(n));
                                set_count(
                                    get_first_child(get_parent(n)), get_count(n) - 1
                                );
                                delete_node(n);
                            }
                            return;
                        }
                        u5 = n1;
                        u7 = get_next_sibling(n1);
                    }
                }
            } while (get_char(n));
        } else {
            dec_dup_count(n);
        }
    }
}
