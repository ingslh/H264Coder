#pragma once

#include <cstdint>
#include "RBSP.h"

//Start code and Emulation Prevention need this to be defined in identical manner at encoder and decoder
#define ZEROBYTES_SHORTSTARTCODE 2 //indicates the number of zero bytes in the short start-code prefix

namespace HM{

class EBSP{
public:
    EBSP() = default;
    EBSP(uint8_t* _buf, int len);
    int EBSPtoRBSP(uint8_t* stream, int end_bytepos, int begin_bytepos);
    bool GenerateRBSP();

    uint8_t * buf = nullptr;
    int len = 0;

    RBSP rbsp;
};

}