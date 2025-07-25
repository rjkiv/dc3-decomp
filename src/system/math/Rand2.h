#pragma once

/** Used for decrypting encrypted BinStreams */
class Rand2 {
private:
    int mSeed;
public:
    Rand2(int);
    int Int();
};