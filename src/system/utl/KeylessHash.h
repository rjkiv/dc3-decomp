#pragma once

/**
 * @brief A keyless hash table.
 *
 * @tparam T1 the key type used to hash the value.
 * @tparam T2 the type to store into the hash table.
 */
#include "math/Primes.h"
#include "math/Sort.h"
#include "os/Debug.h"
#include "utl/AllocInfo.h"
#include "utl/Loader.h"
template <class T1, class T2>
class KeylessHash {
private:
    /** The collection of entries in the hash table. */
    T2 *mEntries; // 0x0
    /** The hash table's size. */
    int mSize; // 0x4
    /** Whether or not this table owns its entries. */
    bool mOwnEntries; // 0x8
    /** The number of currently allocated entries in the table. */
    int mNumEntries; // 0xc
    /** The type T2 representing an empty hash table entry. */
    T2 mEmpty; // 0x10
    /** The type T2 representing a removed hash table entry. */
    T2 mRemoved;

    /** Get the first valid entry in the table from the supplied entry.
     * @param [in] entry The indexed entry in the table to begin searching from.
     * @returns The first valid entry from the supplied entry, if it exists.
     */
    T2 *FirstFrom(T2 *entry);

    /** Advance the index to search the supplied hash table with.
     * The hash table implementation is a circular buffer,
     * so if the end is reached, loop back to index zero.
     */
    void Advance(int &idx) {
        idx++;
        if (idx == mSize)
            idx = 0;
    }

    int Hash(const char *str, int size) { return HashString(str, size); }
    int Hash(void *key, int size) { return HashKey(key, size); }
    int HashValue(T2 &entry, int size);

    bool Cmp(const char *key, T2 &entry) const { return streq((const char *)entry, key); }
    // for AllocInfo** value
    bool Cmp(void *key, T2 &entry) { return entry->mMem == key; }

public:
    // NEW_OVERLOAD;
    // DELETE_OVERLOAD;

    KeylessHash(int, const T2 &, const T2 &, T2 *);
    ~KeylessHash();

    /** Get the entry in the hash table corresponding with the supplied key.
     * @param [in] key The key to search with.
     * @returns The corresponding entry, if it exists.
     */
    T2 *Find(const T1 &key);

    /** Insert this value into the hash table.
     * @param [in] val The val to insert.
     * @returns The entry in the hash table containing the value.
     */
    T2 *Insert(const T2 &val);

    /** Resize the hash table.
     * @param [in] size The new desired size of the table.
     * @param [in] entries Where the newly resized hash table entries will be written. If
     * this is NOT null, then this table does not own its entries.
     */
    void Resize(int size, T2 *entries);

    // getters
    int Size() const { return mSize; }
    int UsedSize() const { return mNumEntries; }

    void Remove(T2 *);
    void Clear();

    // keep these in here so that they're inlined - needed for ObjDirItr

    /** Get the very first valid entry in the table. */
    T2 *Begin() { return FirstFrom(mEntries); }
    /** Get the next valid entry in the table from the supplied entry. */
    T2 *Next(T2 *entry) { return FirstFrom(&entry[1]); }
};

template <class T1, class T2>
inline int KeylessHash<T1, T2>::HashValue(T2 &entry, int size) {
    return Hash((T1)entry, size);
}

template <>
inline int KeylessHash<void *, AllocInfo *>::HashValue(AllocInfo *&entry, int size) {
    return Hash(entry->mMem, size);
}

template <class T1, class T2>
KeylessHash<T1, T2>::KeylessHash(int size, const T2 &empty, const T2 &removed, T2 *entries)
    : mEmpty(empty), mRemoved(removed) {
    if (entries) {
        mSize = size;
        mEntries = entries;
        mOwnEntries = false;
    } else if (size != 0) {
        mSize = NextHashPrime(size);
        mEntries = new T2[mSize];
        mOwnEntries = true;
    } else {
        mSize = 0;
        mEntries = nullptr;
        mOwnEntries = true;
    }
    for (int i = 0; i < mSize; i++) {
        mEntries[i] = mEmpty;
    }
    mNumEntries = 0;
}

template <class T1, class T2>
KeylessHash<T1, T2>::~KeylessHash() {
    if (mOwnEntries) {
        delete[] mEntries;
    }
}

template <class T1, class T2>
T2 *KeylessHash<T1, T2>::FirstFrom(T2 *entry) {
    for (; entry < mEntries + mSize && (*entry == mEmpty || *entry == mRemoved); entry++)
        ;
    if (entry == mEntries + mSize)
        return nullptr;
    else
        return entry;
}

template <class T1, class T2>
T2 *KeylessHash<T1, T2>::Find(const T1 &key) {
    if (mEntries) {
        int i = Hash(key, mSize);
        MILO_ASSERT(i >= 0, 0x86);

        for (; mEntries[i] != mEmpty; Advance(i)) {
            if (mEntries[i] != mRemoved) {
                if (Cmp(key, mEntries[i]))
                    return &mEntries[i];
            }
        }
    }
    return 0;
}

template <class T1, class T2>
T2 *KeylessHash<T1, T2>::Insert(const T2 &val) {
    MILO_ASSERT(val != mEmpty && val != mRemoved, 0x98);
    if (!mEntries) {
        MILO_ASSERT(mOwnEntries, 0x9C);
        Resize(0x19, 0);
    }
    const char *valStr = (const char *)val;
    int i = HashString(valStr, mSize);
    MILO_ASSERT(i >= 0, 0xA2);
    while (mEntries[i] != mEmpty && mEntries[i] != mRemoved
           && !streq((const char *)mEntries[i], valStr)) {
        Advance(i);
    }
    if (mEntries[i] == mEmpty) {
        mNumEntries++;
        if (mNumEntries > mSize / 2) {
            if (mOwnEntries) {
                MILO_ASSERT(mSize, 0xB3);
                Resize(mSize * 2, 0);
                if (!TheLoadMgr.EditMode()) {
                    MILO_NOTIFY("Resizing hash table (%d)", mSize);
                }
                return Insert(val);
            } else {
                MILO_NOTIFY_ONCE("Hash table half full (%d)", mSize / 2);
            }
        }
        if (mNumEntries >= mSize) {
            MILO_FAIL("Hash table full (%d)", mSize);
        }
    }
    mEntries[i] = val;
    return &mEntries[i];
}

template <class T1, class T2>
void KeylessHash<T1, T2>::Resize(int size, T2 *entries) {
    MILO_ASSERT(size > mNumEntries * 2, 0xE6);
    bool owned;
    if (entries)
        owned = false;
    else {
        size = NextHashPrime(size);
        entries = new T2[size];
        owned = true;
    }
    for (int i = 0; i < size; i++) {
        entries[i] = mEmpty;
    }
    mNumEntries = 0;
    for (T2 *it = Begin(); it != 0; it = Next(it)) {
        int i = HashValue(*it, size);
        MILO_ASSERT(i >= 0, 0xFB);
        while (entries[i] != mEmpty) {
            i++;
            if (i == size)
                i = 0;
        }
        mNumEntries++;
        entries[i] = *it;
    }
    if (mOwnEntries)
        delete[] mEntries;
    mEntries = entries;
    mSize = size;
    mOwnEntries = owned;
}

template <class T1, class T2>
void KeylessHash<T1, T2>::Clear() {
    delete[] mEntries;
    mEntries = nullptr;
    mSize = 0;
    mOwnEntries = true;
    mNumEntries = 0;
}
