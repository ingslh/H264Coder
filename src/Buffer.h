#pragma once

#include <cstdint>
#include <mutex>

#define BUFFER_CACHE_SIZE 1024*1024

namespace HM
{

class Buffer{

public:
    Buffer(int size = BUFFER_CACHE_SIZE);
    Buffer(uint8_t * _buf, int _bufLen);
    ~Buffer();

    Buffer(const Buffer & buffer);
    Buffer & operator = (const Buffer & buffer);

    int Append(uint8_t * _buf, int _bufLen);
    int Append(const Buffer & buffer);

    int CutOff(Buffer & buffer, int len);
    int CutOff(uint8_t * buffer, int len);

    int GetBuffer(uint8_t * _buf = nullptr) const;

    int GetLen() const;
    int SetLen(int _len);

    int Clear();

    uint8_t * GetPtr() const;

private:
    uint8_t * originBuf = nullptr;
    int originBufLen = 0;

    uint8_t * buf = nullptr;
    int bufLen = 0;

    int ReAlloc(uint8_t ** buf, int len);
    int ReAlloc(int len);

    int mallocTimes = 0;
};
  
} // namespace HM
