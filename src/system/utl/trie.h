#pragma once

#define MAX_NODES 0x20000
#define NODE_SIZE 0x11
#include "MemTrack.h"
#include "os/Debug.h"

// #pragma pack(push, 1)
// oh yeah this class is awful
// ive added helper functions where necessary since its non stop pointer arithmetic
class Trie {
public:
    int store(const char *);
    void remove(unsigned int);
    char *get(int, char *, int);
    void check_index(unsigned int n) { MILO_ASSERT(0<= n && n < MAX_NODES, 0x36); }
    void inc_count(unsigned int);
    void dec_count(unsigned int);
    void inc_dup_count(unsigned int);
    void dec_dup_count(unsigned int);
    unsigned int get_free_node();
    void delete_node(unsigned int);

    char *GetNode(int n) { return (char *)this + n * NODE_SIZE; }
    unsigned int *GetCounts(int n) { return (unsigned int *)(GetNode(n) + 0xC); }
    unsigned int GetDupCount(int n) { return (unsigned int)(*GetCounts(n) >> 8); }
    unsigned char GetCount(int n) { return *(unsigned char *)(GetNode(n) + 0xF); }
    int *GetNodeCount() { return (int *)(unsigned int *)((char *)this + 0x220000); }
    unsigned int GetFreeHead() { return *(unsigned int *)((char *)this + 0x220004); }
    void SetFreeHead(int n) { *(unsigned int *)(this + 0x220004) = n; }
    unsigned int GetSibling(int n) { return *(unsigned int *)(GetNode(n) + 0x4); }
    void ClearSibling(int n) { *(unsigned int *)((char *)this + n + 0x4) = 0; }
    void ClearChild(int n) { *(unsigned int *)((char *)this + n) = 0; }
    void SetNodeCount(int n) { *(unsigned int *)((char *)this + 0x220000) = n; }
    void IncNodeCount() {
        *(unsigned int *)((char *)this + 0x220000) = *GetNodeCount() + 1;
    }

protected:
    // Counts is a 4 byte int thats used to store Duplicate count and total count
    // DupCount = low 24 bits of the int(16mil max) & Count = high 8 bits(255 max)
    // int mNodeCount = 0x220000
    // int mHead = 0x220004
};
// #pragma pack(pop)
static Trie *pTrie = (Trie *)malloc(MAX_NODES * sizeof(Trie) + 8); // trie base