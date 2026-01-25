#pragma once
#include "utl/MemMgr.h"
#include "utl/TextStream.h"
#include "utl/Symbol.h"
#include <cstring>
#include <vector>

// i can't think of a better place to put this
inline bool IsAsciiNum(char c) { return c >= 0x30 && c <= 0x39; }

extern char gEmpty[8];

// ditto
inline bool streq(const char *s1, const char *s2) { return strcmp(s1, s2) == 0; }

inline bool strneq(const char *s1, const char *s2, int n) {
    return strncmp(s1, s2, n) == 0;
}

inline bool strieq(const char *s1, const char *s2) { return stricmp(s1, s2) == 0; }

class FixedString {
protected:
    char *mStr; // 0x0
public:
    FixedString() : mStr((char *)(gEmpty + 4)) {
        *(int *)(mStr - 4) = 0;
        mStr[0] = '\0';
    }
    FixedString(char *, int);
    ~FixedString() {
        if (capacity() != 0) {
            MemOrPoolFree(capacity() + 5, mStr - 4);
        }
    }

    unsigned int length() const { return strlen(mStr); }
    unsigned int size() const { return strlen(mStr); }
    unsigned int capacity() const { return *(unsigned int *)(mStr - 4); }
    const char *c_str() const { return mStr; }
    bool empty() const { return *mStr == '\0'; }

    bool operator<(const FixedString &) const;
    FixedString &operator+=(const char *);
    bool contains(const char *) const;

    unsigned int find(const char *) const;
    unsigned int find(char, unsigned int) const;
    unsigned int find(char) const;
    unsigned int find_last_of(char) const;
    unsigned int find_last_of(const char *) const;
    unsigned int find(const char *, unsigned int) const;
    unsigned int find_first_of(const char *, unsigned int) const;

    char &operator[](unsigned int);

    void ToLower();
    void ToUpper();
    void ReplaceAll(char, char);
    int compare(unsigned int, unsigned int, const char *) const;

    static const unsigned int npos;
};

class String : public TextStream, public FixedString {
    // TextStream vtable = 0x0
    // FixedString = 0x4
public:
    virtual ~String();
    virtual void Print(const char *str) { *this += str; }

    String();
    String(const char *);
    String(Symbol);
    String(const String &);
    String(unsigned int, char);

    bool operator==(const FixedString &) const;

    void reserve(unsigned int);

    String operator+(const char *) const;
    String operator+(char) const;
    String operator+(const FixedString &) const;
    String &operator+=(const char *);
    String &operator+=(Symbol);
    String &operator+=(const FixedString &);
    String &operator+=(char);
    String &operator=(const char *);
    String &operator=(Symbol);
    String &operator=(const FixedString &);
    String &operator=(const String &);

    // char rindex(int) const;
    // char &rindex(int);

    bool operator!=(const char *) const;
    bool operator!=(const FixedString &) const;
    bool operator==(const char *) const;
    // bool operator==(const String &) const;
    // bool operator<(const String &) const;
    bool operator==(Symbol) const;

    void resize(unsigned int);
    // unsigned int rfind(const char *) const;

    int split(const char *token, std::vector<String> &subStrings) const;

    String substr(unsigned int) const;
    String substr(unsigned int, unsigned int) const;

    // void swap(String &);
    String &replace(unsigned int, unsigned int, const char *);
    String &erase();
    String &erase(unsigned int);
    String &erase(unsigned int, unsigned int);
    String &insert(unsigned int, unsigned int, char);
    String &insert(unsigned int, const char *);
    // String &insert(unsigned int, const String &);
};

bool SearchReplace(const char *, const char *, const char *, char *);
bool StrNCopy(char *, const char *, int);
void RemoveSpaces(char *, int, const char *);

inline TextStream &operator<<(TextStream &ts, const String &str) {
    ts.Print(str.c_str());
    return ts;
}

template <int N>
class StackString : public TextStream, public FixedString {
private:
    char mStack[N];

public:
    StackString() : FixedString(mStack, N + 5) {}
    StackString(const char *str) : FixedString(mStack, N + 5) { *this += str; }
    // virtual ~StackString() {} // dtor is at 0x8269E480
    virtual void Print(const char *str) { *this += str; }
    operator const char *() const { return c_str(); }
};
