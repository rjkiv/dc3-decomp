#pragma once

class StreamReader {
public:
    StreamReader() {}
    virtual ~StreamReader() {}
    virtual void Poll(float until) = 0;
    virtual void Seek(int iSample) = 0;
    virtual void EnableReads(bool bEnable) = 0;
    virtual bool Done() = 0;
    virtual bool Fail() = 0;
    virtual void Init() = 0;
};
