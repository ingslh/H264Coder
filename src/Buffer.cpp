#include "Buffer.h"
#include <cstring>

namespace HM{

int Buffer::ReAlloc(uint8_t ** targetBuf, int targetLen){
    // Calculate remaining space
    int size = originBufLen - (buf - originBuf);
    if(targetLen > size) {
        // Reassign space
        int targetBufLen = targetLen * 3;
        *targetBuf = (uint8_t *)malloc(targetBufLen);

        mallocTimes++;

        return targetBufLen;
    }

    return 0;
}

int Buffer::ReAlloc(int len){
    uint8_t * targetBuf = nullptr;
    int ret = ReAlloc(&targetBuf, len);
    if(ret > 0){
        if(originBuf != nullptr){
            free(originBuf);
            originBuf = nullptr;
        }
        originBuf = targetBuf;
        originBufLen = ret;
    }

    buf = originBuf;
    bufLen = len;

    return len;
}

Buffer::Buffer(int size){
    uint8_t * targetBuf = nullptr;
    int ret = ReAlloc(&targetBuf, size);
    if(ret > 0){
        originBuf = targetBuf;
        originBufLen = ret;
    }

    buf = originBuf;
    bufLen = 0;
}

Buffer::Buffer(uint8_t * _buf, int _bufLen) : Buffer(_bufLen){
    memcpy(buf, _buf, _bufLen);
    bufLen = _bufLen;
}

Buffer::~Buffer(){
    Clear();

    if(originBuf != nullptr){
        free(originBuf);
        originBuf = nullptr;
    }
    originBufLen = 0;
}

int Buffer::Clear(){
    buf = originBuf;
    bufLen = 0;
    return 0;
}

Buffer::Buffer(const Buffer & buffer) : Buffer(){
    *this = buffer;
}

Buffer & Buffer::operator = (const Buffer & buffer){
    uint8_t * targetBuf = nullptr;
    int ret = ReAlloc(&targetBuf, buffer.bufLen);
    if(ret > 0){
        originBuf = targetBuf;
        originBufLen = ret;
    }

    buf = originBuf;

    bufLen = buffer.bufLen;
    memcpy(buf, buffer.buf, bufLen);

    return *this;
}

int Buffer::Append(uint8_t * _buf, int _bufLen){
    uint8_t * targetBuf = nullptr;
    int targetSize = bufLen + _bufLen;
    int ret = ReAlloc(&targetBuf, targetSize);
    if(ret > 0){
        // New memory is allocated and the old data is copied out first
        memcpy(targetBuf, buf, bufLen);
        // Copy the new incoming data again
        memcpy(targetBuf + bufLen, _buf, _bufLen);

        if(originBuf != nullptr){
            free(originBuf);
            originBuf = nullptr;
        }

        originBuf = targetBuf;
        originBufLen = ret;

        buf = originBuf;
        bufLen = targetSize;
    }
    else {
        //No new memory is allocated
        memcpy(buf + bufLen, _buf, _bufLen);
        bufLen = targetSize;
    }

    return 0;
}

int Buffer::Append(const Buffer & buffer){
    return Append(buffer.buf, buffer.bufLen);
}

int Buffer::CutOff(uint8_t * buffer, int len){
    if(len <= 0){
        return 0;
    }

    if(len > bufLen){
        len = bufLen;
    }

    memcpy(buffer, buf, len);
    buf = buf + len;
    bufLen = bufLen - len;

    return len;
}

int Buffer::CutOff(Buffer & buffer, int len){
    buffer.ReAlloc(len);

    int ret = CutOff(buffer.GetPtr(), len);
    return ret;
}

int Buffer::GetBuffer(uint8_t * _buf) const{
    if(_buf == nullptr){
        return bufLen;
    }

    memcpy(_buf, buf, bufLen);

    return 0;
}

int Buffer::GetLen() const
{
    return GetBuffer();
}

int Buffer::SetLen(int _len)
{
    bufLen = _len;
    return 0;
}

uint8_t * Buffer::GetPtr() const{
    return buf;
}


}