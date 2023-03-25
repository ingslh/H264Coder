#include "RBSP.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>

namespace HM{

RBSP::RBSP(const RBSP & rbsp)
{
    *this = rbsp;
}

RBSP::~RBSP()
{
}

RBSP& RBSP::operator = (const RBSP & rbsp)
{
    if(buf != nullptr){
        free(buf);
        buf = nullptr;
    }

    len = rbsp.len;
    buf = (uint8_t *)malloc(len);
    memcpy(buf, rbsp.buf, len);

    return *this;
}

int RBSP::RBSPtoSODB(){
    uint8_t* stream = buf;
    int last_byte_pos = len; 

    int ctr_bit, bitoffset;
    bitoffset = 0;
    //find trailing 1
    ctr_bit = (stream[last_byte_pos-1] & (0x01<<bitoffset));   // set up control bit

    while (ctr_bit==0)
    {                 // find trailing 1 bit
        ++bitoffset;
        if(bitoffset == 8)
        {
        if(last_byte_pos == 0)
            printf(" Panic: All zero data sequence in RBSP \n");
        //assert(last_byte_pos != 0);
        --last_byte_pos;
        bitoffset = 0;
        }
        ctr_bit= stream[last_byte_pos - 1] & (0x01<<(bitoffset));
    }
    return last_byte_pos;
}

}
