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

    int store(const char *str);
    void remove(unsigned int);

    char *get(int n, char *buf, int bufSize) {
        if (n > 0 && n < MAX_NODES && get_char(n) == '\0') {
            char *p = buf + bufSize - 1;
            for (int i = 0; n != 0 && i < bufSize; i++, p--) {
                *p = get_char(n);
                n = get_parent(n);
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
    void inc_count(unsigned int);
    void dec_count(unsigned int);
    void inc_dup_count(unsigned int);
    void dec_dup_count(unsigned int);
    unsigned int get_free_node();
    void delete_node(unsigned int);

    unsigned int get_first_child(unsigned int idx) {
        check_index(idx);
        return mNodes[idx].firstChild;
    }
    void set_first_child(unsigned int idx, unsigned int value) {
        check_index(idx);
        mNodes[idx].firstChild = value;
    }
    unsigned int get_next_sibling(unsigned int idx) {
        check_index(idx);
        return mNodes[idx].nextSibling;
    }
    void set_next_sibling(unsigned int idx, unsigned int sibling) {
        check_index(idx);
        mNodes[idx].nextSibling = sibling;
    }
    void clear_next_sibling(unsigned int idx) {
        if (mFreeHead) {
            check_index(idx);
            mNodes[idx].nextSibling = mFreeHead;
        }
        mFreeHead = idx;
    }
    unsigned int get_parent(unsigned int idx) {
        check_index(idx);
        return mNodes[idx].parent;
    }
    void set_parent(unsigned int idx, unsigned int parent) {
        check_index(idx);
        mNodes[idx].parent = parent;
    }
    void clear_parent(unsigned int idx) {
        check_index(idx);
        mNodes[idx].parent = 0;
        mNodes[idx].mCounts = 0;
    }

    void set_char(unsigned int idx, char c) {
        check_index(idx);
        mNodes[idx].mChar = c;
    }

    char get_char(unsigned int idx) {
        check_index(idx);
        return mNodes[idx].mChar;
    }

    unsigned int get_dup_count(unsigned int idx) {
        check_index(idx);
        return mNodes[idx].mCounts >> 8;
    }
    void set_dup_count(unsigned int idx, unsigned int dup_count) {
        check_index(idx);
        mNodes[idx].mCounts = (dup_count << 8) | (mNodes[idx].mCounts & 0xFF);
    }

    unsigned int get_count(unsigned int idx) {
        check_index(idx);
        return mNodes[idx].mCounts & 0xFF;
    }
    void set_count(unsigned int idx, unsigned int count) {
        check_index(idx);
        mNodes[idx].mCounts = (mNodes[idx].mCounts & ~0xFF) | count;
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

    Node mNodes[MAX_NODES]; // 0x0
    int _nodeCount; // 0x220000
    unsigned int mFreeHead; // 0x220004
    // Counts is a 4 byte int thats used to store Duplicate count and total count
    // DupCount = upper 24 bits of the int(16mil max) & Count = low 8 bits(255 max)
};
#pragma pack(pop)
