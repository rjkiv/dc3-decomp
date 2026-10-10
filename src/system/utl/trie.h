#pragma once
#include "os/Debug.h"
#include "utl/MemMgr.h"

#define MAX_NODES 0x20000

// oh yeah this class is awful
#pragma pack(push, 1)
class Trie {
public:
    Trie() {
        memset(this, 0, sizeof(Trie));
        _nodeCount = 1;
    }

    int store(const char *s);
    void remove(unsigned int node);

    char *get(int n, char *buf, int bufSize) {
        if (n > 0 && n < MAX_NODES && ch(n) == '\0') {
            char *p = buf + bufSize - 1;
            for (int i = 0; n != 0 && i < bufSize; i++, p--) {
                *p = ch(n);
                n = parent(n);
            }
            char *end = buf + bufSize;
            buf[bufSize - 1] = '\0';
            return p == end - 1 ? p : p + 1;
        } else {
            *buf = '\0';
            return buf;
        }
    }
    void check_index(unsigned int n) { MILO_ASSERT(0<= n && n < MAX_NODES, 0x36); }
    void inc_count(unsigned int n);
    void dec_count(unsigned int n);
    void inc_dup_count(unsigned int n);
    void dec_dup_count(unsigned int n);
    unsigned int get_free_node();
    void delete_node(unsigned int node);

    // declared in the class type record; inlined, no standalone symbol in the map
    char to_char(char c);
    char to_printed_char(char c);

    unsigned int &first_child(unsigned int idx) {
        check_index(idx);
        return _nodes[idx].firstChild;
    }
    void set_first_child(unsigned int idx, const unsigned int &value) {
        check_index(idx);
        _nodes[idx].firstChild = value;
    }
    unsigned int &sibling(unsigned int idx) {
        check_index(idx);
        return _nodes[idx].nextSibling;
    }
    void set_next_sibling(unsigned int idx, const unsigned int &sibling) {
        check_index(idx);
        _nodes[idx].nextSibling = sibling;
    }
    void clear_next_sibling(unsigned int idx) {
        if (_freeListHead) {
            check_index(idx);
            _nodes[idx].nextSibling = _freeListHead;
        }
        _freeListHead = idx;
    }
    unsigned int &parent(unsigned int idx) {
        check_index(idx);
        return _nodes[idx].parent;
    }
    void set_parent(unsigned int idx, unsigned int parent) {
        check_index(idx);
        _nodes[idx].parent = parent;
    }
    void clear_parent(unsigned int idx) {
        check_index(idx);
        _nodes[idx].parent = 0;
        _nodes[idx].mCounts = 0;
    }

    char &ch(unsigned int idx) {
        check_index(idx);
        return _nodes[idx].mChar;
    }
    void set_char(unsigned int idx, char c) {
        check_index(idx);
        _nodes[idx].mChar = c;
    }

    unsigned int dup_count(unsigned int idx) {
        check_index(idx);
        return _nodes[idx].mCounts >> 8;
    }
    void set_dup_count(unsigned int idx, unsigned int dup_count) {
        check_index(idx);
        _nodes[idx].mCounts = (dup_count << 8) | (_nodes[idx].mCounts & 0xFF);
    }

    unsigned int count(unsigned int idx) {
        check_index(idx);
        return _nodes[idx].mCounts & 0xFF;
    }
    void set_count(unsigned int idx, unsigned int count) {
        check_index(idx);
        _nodes[idx].mCounts = (_nodes[idx].mCounts & ~0xFF) | count;
    }

    MEM_OVERLOAD(Trie, 0x28);

private:
    // size 0x11
    struct Node {
        unsigned int firstChild; // 0x0
        unsigned int nextSibling; // 0x4
        unsigned int parent; // 0x8
        // top 24 bits = dupe count
        // bottom 8 bits = regular count
        unsigned int mCounts; // 0xc
        char mChar; // 0x10
    };

    Node _nodes[MAX_NODES]; // 0x0
    int _nodeCount; // 0x220000
    unsigned int _freeListHead; // 0x220004
    // Counts is a 4 byte int thats used to store Duplicate count and total count
    // DupCount = upper 24 bits of the int(16mil max) & Count = low 8 bits(255 max)
};
#pragma pack(pop)
